// Copyright (c) 2026 The codem37 Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CODEM37_SRC_MINE_COMPATIBILITY_USER_AGENT_PROVIDER_H_
#define CODEM37_SRC_MINE_COMPATIBILITY_USER_AGENT_PROVIDER_H_

#include <string>
#include <vector>

namespace codem37 {

struct UserAgentBrandVersion {
  std::string brand;
  std::string version;
};

// Generates truthful, standards-compliant User-Agent and Client Hints.
class UserAgentProvider {
 public:
  static std::string GetUserAgent();
  static std::vector<UserAgentBrandVersion> GetBrandVersionList();
  static std::string GetPlatformName();
};

}  // namespace codem37

#endif  // CODEM37_SRC_MINE_COMPATIBILITY_USER_AGENT_PROVIDER_H_
