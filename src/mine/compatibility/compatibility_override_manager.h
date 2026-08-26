// Copyright (c) 2026 The codem37 Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CODEM37_SRC_MINE_COMPATIBILITY_COMPATIBILITY_OVERRIDE_MANAGER_H_
#define CODEM37_SRC_MINE_COMPATIBILITY_COMPATIBILITY_OVERRIDE_MANAGER_H_

#include <map>
#include <string>
#include <vector>

namespace codem37 {

struct SiteCompatibilityOverride {
  std::string id;
  std::string target_domain;
  std::string reason;
  std::string chromium_milestone;
  int64_t expiry_timestamp_unix = 0;
  bool attempts_security_bypass = false; // Must ALWAYS be false
  std::string custom_user_agent;
};

// Manages signed, versioned site-specific compatibility overrides.
class CompatibilityOverrideManager {
 public:
  CompatibilityOverrideManager();
  ~CompatibilityOverrideManager();

  bool RegisterOverride(const SiteCompatibilityOverride& override_entry);
  bool HasActiveOverride(const std::string& domain, int64_t current_time_unix) const;
  std::string GetUserAgentForDomain(const std::string& domain,
                                   int64_t current_time_unix) const;

 private:
  std::map<std::string, SiteCompatibilityOverride> overrides_;
};

}  // namespace codem37

#endif  // CODEM37_SRC_MINE_COMPATIBILITY_COMPATIBILITY_OVERRIDE_MANAGER_H_
