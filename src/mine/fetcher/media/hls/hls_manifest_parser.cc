// Copyright 2026 The codem37 Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "mine/fetcher/media/hls/hls_manifest_parser.h"

#include "base/strings/string_number_conversions.h"
#include "base/strings/string_split.h"
#include "base/strings/string_util.h"

namespace mine {
namespace media {

std::optional<HlsPlaylist> HlsManifestParser::Parse(
    const std::string& manifest_content,
    const GURL& base_url) {
  if (!base::StartsWith(manifest_content, "#EXTM3U", base::CompareCase::SENSITIVE)) {
    return std::nullopt;
  }

  HlsPlaylist playlist;
  auto lines = base::SplitStringPiece(manifest_content, "\n",
                                      base::TRIM_WHITESPACE,
                                      base::SPLIT_WANT_NONEMPTY);

  HlsVariant current_variant;
  bool pending_variant = false;
  double current_inf_duration = 0.0;
  bool is_discontinuity = false;

  for (const auto& raw_line : lines) {
    base::StringPiece line = base::TrimWhitespaceASCII(raw_line, base::TRIM_ALL);
    if (line.empty()) continue;

    if (base::StartsWith(line, "#EXT-X-STREAM-INF:")) {
      playlist.is_master = true;
      current_variant = HlsVariant();
      pending_variant = true;

      // Extract BANDWIDTH, RESOLUTION, CODECS
      base::StringPiece attrs = line.substr(18);
      auto tokens = base::SplitStringPiece(attrs, ",", base::TRIM_WHITESPACE, base::SPLIT_WANT_NONEMPTY);
      for (const auto& token : tokens) {
        if (base::StartsWith(token, "BANDWIDTH=")) {
          uint64_t bw = 0;
          if (base::StringToUint64(token.substr(10), &bw)) {
            current_variant.bandwidth = bw;
          }
        } else if (base::StartsWith(token, "RESOLUTION=")) {
          current_variant.resolution = std::string(token.substr(11));
        } else if (base::StartsWith(token, "CODECS=")) {
          current_variant.codecs = std::string(token.substr(7));
        }
      }
    } else if (base::StartsWith(line, "#EXT-X-MAP:")) {
      // Initialization segment (#EXT-X-MAP:URI="init.mp4")
      size_t uri_pos = line.find("URI=\"");
      if (uri_pos != std::string::npos) {
        size_t end_pos = line.find("\"", uri_pos + 5);
        if (end_pos != std::string::npos) {
          std::string uri = std::string(line.substr(uri_pos + 5, end_pos - (uri_pos + 5)));
          playlist.init_segment = base_url.Resolve(uri);
        }
      }
    } else if (base::StartsWith(line, "#EXT-X-KEY:")) {
      // Detected DRM/Encryption key
      playlist.has_drm = true;
    } else if (base::StartsWith(line, "#EXT-X-DISCONTINUITY")) {
      is_discontinuity = true;
    } else if (base::StartsWith(line, "#EXTINF:")) {
      // Extract segment duration
      base::StringPiece duration_str = line.substr(8);
      size_t comma = duration_str.find(",");
      if (comma != std::string::npos) {
        duration_str = duration_str.substr(0, comma);
      }
      double dur = 0.0;
      if (base::StringToDouble(duration_str, &dur)) {
        current_inf_duration = dur;
      }
    } else if (base::StartsWith(line, "#EXT-X-ENDLIST")) {
      playlist.is_live = false;
    } else if (!base::StartsWith(line, "#")) {
      // URI Line
      GURL target_url = base_url.Resolve(std::string(line));
      if (pending_variant) {
        current_variant.playlist_url = target_url;
        playlist.variants.push_back(current_variant);
        pending_variant = false;
      } else {
        HlsSegment seg;
        seg.url = target_url;
        seg.duration_seconds = current_inf_duration;
        seg.init_segment_url = playlist.init_segment;
        seg.is_discontinuity = is_discontinuity;
        playlist.segments.push_back(seg);

        // Reset per-segment state
        current_inf_duration = 0.0;
        is_discontinuity = false;
      }
    }
  }

  return playlist;
}

}  // namespace media
}  // namespace mine
