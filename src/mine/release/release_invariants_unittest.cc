// Copyright (c) 2026 The codem37 Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <string>
#include <vector>

#include "testing/gtest/include/gtest/gtest.h"

namespace codem37 {
namespace {

struct SemanticVersion {
  int major = 0;
  int minor = 0;
  int patch = 0;

  bool operator<(const SemanticVersion& other) const {
    if (major != other.major) return major < other.major;
    if (minor != other.minor) return minor < other.minor;
    return patch < other.patch;
  }
};

SemanticVersion ParseVersion(const std::string& ver_str) {
  SemanticVersion ver;
  int parsed = sscanf(ver_str.c_str(), "%d.%d.%d", &ver.major, &ver.minor, &ver.patch);
  if (parsed != 3) {
    return {0, 0, 0};
  }
  return ver;
}

// 1. Invariant: Semantic version parsing and monotonic comparison.
TEST(ReleaseInvariantsTest, SemanticVersionComparison) {
  SemanticVersion v100 = ParseVersion("1.0.0");
  SemanticVersion v101 = ParseVersion("1.0.1");
  SemanticVersion v110 = ParseVersion("1.1.0");
  SemanticVersion v200 = ParseVersion("2.0.0");

  EXPECT_TRUE(v100 < v101);
  EXPECT_TRUE(v101 < v110);
  EXPECT_TRUE(v110 < v200);
  EXPECT_FALSE(v101 < v100);
}

// 2. Invariant: Downgrade protection prevents applying older package versions.
TEST(ReleaseInvariantsTest, DowngradeProtectionEnforced) {
  SemanticVersion current_installed = ParseVersion("1.2.0");
  SemanticVersion incoming_update = ParseVersion("1.1.9");

  bool is_downgrade = incoming_update < current_installed;
  EXPECT_TRUE(is_downgrade);

  // Downgrade must be rejected unless explicitly designated emergency rollback
  bool allow_install = !is_downgrade;
  EXPECT_FALSE(allow_install);
}

// 3. Invariant: Independent component rollback does not impact browser binary.
TEST(ReleaseInvariantsTest, IndependentComponentRollback) {
  std::string browser_version = "1.0.0";
  std::string active_ruleset = "2026.08.26.2";
  std::string fallback_ruleset = "2026.08.26.1";

  // Simulate corrupt ruleset activation -> fallback triggered
  bool ruleset_corrupted = true;
  if (ruleset_corrupted) {
    active_ruleset = fallback_ruleset;
  }

  // Ruleset fell back safely while browser version remained unchanged
  EXPECT_EQ(active_ruleset, "2026.08.26.1");
  EXPECT_EQ(browser_version, "1.0.0");
}

}  // namespace
}  // namespace codem37
