// Copyright 2026 The codem37 Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef MINE_FETCHER_ENGINE_RESOURCE_VALIDATOR_H_
#define MINE_FETCHER_ENGINE_RESOURCE_VALIDATOR_H_

#include <cstdint>
#include <optional>
#include <string>

namespace mine {
namespace engine {

class ResourceValidator {
 public:
  // Verifies that a worker's HTTP 206 response matches expected range and ETag.
  static bool ValidateRangeResponse(int http_status,
                                    uint64_t expected_start,
                                    uint64_t expected_end,
                                    uint64_t content_range_start,
                                    uint64_t content_range_end,
                                    uint64_t content_range_total,
                                    const std::optional<std::string>& expected_etag,
                                    const std::optional<std::string>& response_etag);
};

}  // namespace engine
}  // namespace mine

#endif  // MINE_FETCHER_ENGINE_RESOURCE_VALIDATOR_H_
