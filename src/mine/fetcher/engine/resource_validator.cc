// Copyright 2026 The codem37 Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "mine/fetcher/engine/resource_validator.h"

namespace mine {
namespace engine {

bool ResourceValidator::ValidateRangeResponse(
    int http_status,
    uint64_t expected_start,
    uint64_t expected_end,
    uint64_t content_range_start,
    uint64_t content_range_end,
    uint64_t content_range_total,
    const std::optional<std::string>& expected_etag,
    const std::optional<std::string>& response_etag) {
  // 1. Must be HTTP 206 Partial Content
  if (http_status != 206) {
    return false;
  }

  // 2. Content-Range boundaries must match requested range
  if (content_range_start != expected_start || content_range_end != expected_end) {
    return false;
  }

  // 3. ETag consistency check across parallel workers
  if (expected_etag.has_value() && response_etag.has_value()) {
    if (*expected_etag != *response_etag) {
      return false;
    }
  }

  return true;
}

}  // namespace engine
}  // namespace mine
