// Copyright 2026 The codem37 Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "mine/fetcher/engine/http_resource_probe.h"

namespace mine {
namespace engine {

bool HttpResourceProbe::QualifiesForSegmentation(const ResourceMetadata& meta,
                                                 uint64_t threshold_bytes) {
  // Phase 0 / Phase 5 Invariant:
  // Must be HTTP 200/206, known content length >= threshold, and support ranges.
  if (meta.http_status_code != 200 && meta.http_status_code != 206) {
    return false;
  }
  if (!meta.supports_ranges) {
    return false;
  }
  if (meta.content_length < static_cast<int64_t>(threshold_bytes)) {
    return false;
  }
  return true;
}

}  // namespace engine
}  // namespace mine
