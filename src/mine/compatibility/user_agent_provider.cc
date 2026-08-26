// Copyright (c) 2026 The codem37 Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "src/mine/compatibility/user_agent_provider.h"

namespace codem37 {

namespace {

constexpr char kChromiumMilestone[] = "134.0.6998.88";
constexpr char kMajorMilestone[] = "134";

}  // namespace

std::string UserAgentProvider::GetPlatformName() {
#if defined(_WIN32) || defined(_WIN64)
  return "Windows NT 10.0; Win64; x64";
#elif defined(__linux__)
  return "X11; Linux x86_64";
#else
  return "Unknown; x86_64";
#endif
}

std::string UserAgentProvider::GetUserAgent() {
  return "Mozilla/5.0 (" + GetPlatformName() + ") AppleWebKit/537.36 (KHTML, like Gecko) Chrome/" +
         kChromiumMilestone + " Safari/537.36 codem37/" + kMajorMilestone;
}

std::vector<UserAgentBrandVersion> UserAgentProvider::GetBrandVersionList() {
  return {
      {"Chromium", kMajorMilestone},
      {"codem37", kMajorMilestone},
      {"Not(A:Brand", "24"},
  };
}

}  // namespace codem37
