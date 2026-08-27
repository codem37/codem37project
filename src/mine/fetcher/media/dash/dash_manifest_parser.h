// Copyright 2026 The codem37 Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef MINE_FETCHER_MEDIA_DASH_DASH_MANIFEST_PARSER_H_
#define MINE_FETCHER_MEDIA_DASH_DASH_MANIFEST_PARSER_H_

#include <cstdint>
#include <optional>
#include <string>
#include <vector>
#include "url/gurl.h"

namespace mine {
namespace media {

struct DashRepresentation {
  std::string id;
  uint64_t bandwidth = 0;
  uint32_t width = 0;
  uint32_t height = 0;
  std::string codecs;
  std::string mime_type;
  std::string initialization_template;
  std::string media_template;
  GURL base_url;
};

struct DashAdaptationSet {
  std::string content_type; // "video" or "audio"
  std::string mime_type;
  std::vector<DashRepresentation> representations;
};

struct DashManifest {
  bool is_dynamic = false;
  double media_presentation_duration_seconds = 0.0;
  std::vector<DashAdaptationSet> adaptation_sets;
  bool has_drm = false; // ContentProtection element present
};

// ISO/IEC 23009-1 DASH MPD XML Parser
class DashManifestParser {
 public:
  static std::optional<DashManifest> Parse(const std::string& mpd_content,
                                           const GURL& base_url);
};

}  // namespace media
}  // namespace mine

#endif  // MINE_FETCHER_MEDIA_DASH_DASH_MANIFEST_PARSER_H_
