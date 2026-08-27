// Copyright 2026 The codem37 Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef MINE_FETCHER_ENGINE_HTTP_RESOURCE_PROBE_H_
#define MINE_FETCHER_ENGINE_HTTP_RESOURCE_PROBE_H_

#include <cstdint>
#include <optional>
#include <string>
#include "base/functional/callback.h"
#include "url/gurl.h"

namespace mine {
namespace engine {

struct ResourceMetadata {
  GURL url;
  int64_t content_length = -1;
  bool supports_ranges = false;
  std::optional<std::string> etag;
  std::optional<std::string> last_modified;
  std::string mime_type;
  std::string filename_suggestion;
  int http_status_code = 0;
};

using ProbeCallback = base::OnceCallback<void(std::optional<ResourceMetadata>)>;

// Probes server capabilities before allocating workers.
class HttpResourceProbe {
 public:
  // Decides whether a resource qualifies for multi-connection segmented fetch:
  // Invariants: Content-Length known, >= 100 MB, Accept-Ranges: bytes supported.
  static bool QualifiesForSegmentation(const ResourceMetadata& meta,
                                       uint64_t threshold_bytes = 100 * 1024 * 1024);
};

}  // namespace engine
}  // namespace mine

#endif  // MINE_FETCHER_ENGINE_HTTP_RESOURCE_PROBE_H_
