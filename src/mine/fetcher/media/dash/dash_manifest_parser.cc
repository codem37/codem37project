// Copyright 2026 The codem37 Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "mine/fetcher/media/dash/dash_manifest_parser.h"

#include "base/strings/string_number_conversions.h"
#include "base/strings/string_util.h"

namespace mine {
namespace media {

namespace {

// Lightweight XML attribute extractor helper
std::string ExtractAttribute(base::StringPiece tag, base::StringPiece attr_name) {
  std::string pattern = std::string(attr_name) + "=\"";
  size_t start = tag.find(pattern);
  if (start == base::StringPiece::npos) return "";
  start += pattern.length();
  size_t end = tag.find("\"", start);
  if (end == base::StringPiece::npos) return "";
  return std::string(tag.substr(start, end - start));
}

}  // namespace

std::optional<DashManifest> DashManifestParser::Parse(
    const std::string& mpd_content,
    const GURL& base_url) {
  if (mpd_content.find("<MPD") == std::string::npos) {
    return std::nullopt;
  }

  DashManifest manifest;
  if (mpd_content.find("type=\"dynamic\"") != std::string::npos) {
    manifest.is_dynamic = true;
  }

  if (mpd_content.find("<ContentProtection") != std::string::npos) {
    manifest.has_drm = true;
  }

  // Find all AdaptationSets
  size_t pos = 0;
  while ((pos = mpd_content.find("<AdaptationSet", pos)) != std::string::npos) {
    size_t end_adapt = mpd_content.find("</AdaptationSet>", pos);
    if (end_adapt == std::string::npos) end_adapt = mpd_content.length();

    base::StringPiece adapt_block(&mpd_content[pos], end_adapt - pos);
    DashAdaptationSet adapt_set;
    adapt_set.content_type = ExtractAttribute(adapt_block, "contentType");
    adapt_set.mime_type = ExtractAttribute(adapt_block, "mimeType");

    // Search for representations inside this AdaptationSet
    size_t rep_pos = 0;
    while ((rep_pos = adapt_block.find("<Representation", rep_pos)) != base::StringPiece::npos) {
      size_t rep_end = adapt_block.find(">", rep_pos);
      if (rep_end == base::StringPiece::npos) break;

      base::StringPiece rep_tag = adapt_block.substr(rep_pos, rep_end - rep_pos + 1);
      DashRepresentation rep;
      rep.id = ExtractAttribute(rep_tag, "id");
      rep.codecs = ExtractAttribute(rep_tag, "codecs");
      rep.mime_type = ExtractAttribute(rep_tag, "mimeType");
      if (rep.mime_type.empty()) {
        rep.mime_type = adapt_set.mime_type;
      }

      std::string bw_str = ExtractAttribute(rep_tag, "bandwidth");
      base::StringToUint64(bw_str, &rep.bandwidth);

      std::string width_str = ExtractAttribute(rep_tag, "width");
      base::StringToUint(width_str, &rep.width);

      std::string height_str = ExtractAttribute(rep_tag, "height");
      base::StringToUint(height_str, &rep.height);

      rep.base_url = base_url;
      adapt_set.representations.push_back(rep);
      rep_pos = rep_end + 1;
    }

    manifest.adaptation_sets.push_back(adapt_set);
    pos = end_adapt + 16;
  }

  return manifest;
}

}  // namespace media
}  // namespace mine
