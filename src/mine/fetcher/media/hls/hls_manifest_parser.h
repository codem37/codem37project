// Copyright 2026 The codem37 Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef MINE_FETCHER_MEDIA_HLS_HLS_MANIFEST_PARSER_H_
#define MINE_FETCHER_MEDIA_HLS_HLS_MANIFEST_PARSER_H_

#include <cstdint>
#include <optional>
#include <string>
#include <vector>
#include "url/gurl.h"

namespace mine {
namespace media {

struct HlsSegment {
  GURL url;
  double duration_seconds = 0.0;
  std::optional<std::pair<uint64_t, uint64_t>> byte_range; // offset, length
  std::optional<GURL> init_segment_url;
  bool is_discontinuity = false;
};

struct HlsVariant {
  GURL playlist_url;
  uint64_t bandwidth = 0;
  std::string resolution; // e.g. "1920x1080"
  std::string codecs;
  std::string audio_group;
};

struct HlsPlaylist {
  bool is_master = false;
  bool is_live = false;
  std::vector<HlsVariant> variants;   // For master playlists
  std::vector<HlsSegment> segments;   // For media playlists
  std::optional<GURL> init_segment;
  bool has_drm = false;               // Detected #EXT-X-KEY
};

// RFC 8216 Compliant HLS Manifest Parser
class HlsManifestParser {
 public:
  static std::optional<HlsPlaylist> Parse(const std::string& manifest_content,
                                          const GURL& base_url);
};

}  // namespace media
}  // namespace mine

#endif  // MINE_FETCHER_MEDIA_HLS_HLS_MANIFEST_PARSER_H_
