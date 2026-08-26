// Copyright (c) 2026 The codem37 Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "src/mine/compatibility/compatibility_override_manager.h"

#include "src/mine/compatibility/user_agent_provider.h"

namespace codem37 {

CompatibilityOverrideManager::CompatibilityOverrideManager() = default;
CompatibilityOverrideManager::~CompatibilityOverrideManager() = default;

bool CompatibilityOverrideManager::RegisterOverride(
    const SiteCompatibilityOverride& override_entry) {
  // Absolute Invariant: Rejects any override attempting to bypass security
  if (override_entry.attempts_security_bypass) {
    return false;
  }

  // Must have valid ID, domain, milestone, and expiry timestamp
  if (override_entry.id.empty() || override_entry.target_domain.empty() ||
      override_entry.expiry_timestamp_unix <= 0) {
    return false;
  }

  overrides_[override_entry.target_domain] = override_entry;
  return true;
}

bool CompatibilityOverrideManager::HasActiveOverride(
    const std::string& domain,
    int64_t current_time_unix) const {
  auto it = overrides_.find(domain);
  if (it == overrides_.end()) {
    return false;
  }
  // Check expiration date
  return it->second.expiry_timestamp_unix > current_time_unix;
}

std::string CompatibilityOverrideManager::GetUserAgentForDomain(
    const std::string& domain,
    int64_t current_time_unix) const {
  auto it = overrides_.find(domain);
  if (it != overrides_.end() && it->second.expiry_timestamp_unix > current_time_unix) {
    if (!it->second.custom_user_agent.empty()) {
      return it->second.custom_user_agent;
    }
  }
  return UserAgentProvider::GetUserAgent();
}

}  // namespace codem37
