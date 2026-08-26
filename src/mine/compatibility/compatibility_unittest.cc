// Copyright (c) 2026 The codem37 Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "src/mine/compatibility/compatibility_override_manager.h"
#include "src/mine/compatibility/user_agent_provider.h"
#include "src/mine/diagnostics/compatibility_diagnostics.h"

#include "testing/gtest/include/gtest/gtest.h"
#include "url/gurl.h"

namespace codem37 {
namespace {

// 1. Verify truthful User-Agent and Client Hints generation matching pinned milestone.
TEST(CompatibilityTest, TruthfulUserAgentAndBrandGeneration) {
  std::string ua = UserAgentProvider::GetUserAgent();
  EXPECT_TRUE(ua.find("Chrome/134.0.6998.88") != std::string::npos);
  EXPECT_TRUE(ua.find("codem37/134") != std::string::npos);

  auto brands = UserAgentProvider::GetBrandVersionList();
  ASSERT_GE(brands.size(), 2u);
  EXPECT_EQ(brands[0].brand, "Chromium");
  EXPECT_EQ(brands[0].version, "134");
  EXPECT_EQ(brands[1].brand, "codem37");
  EXPECT_EQ(brands[1].version, "134");
}

// 2. Verify override registration, expiration, and security-bypass rejection.
TEST(CompatibilityTest, OverrideManagerEnforcesInvariantsAndExpiry) {
  CompatibilityOverrideManager manager;

  // Attempted security bypass is strictly rejected
  SiteCompatibilityOverride bad_override;
  bad_override.id = "bad-override";
  bad_override.target_domain = "evil.example.com";
  bad_override.attempts_security_bypass = true;
  bad_override.expiry_timestamp_unix = 2000000000;
  EXPECT_FALSE(manager.RegisterOverride(bad_override));

  // Valid override
  SiteCompatibilityOverride valid_override;
  valid_override.id = "compat-101";
  valid_override.target_domain = "legacy-portal.example.com";
  valid_override.reason = "Server-side strict UA sniff";
  valid_override.chromium_milestone = "134";
  valid_override.expiry_timestamp_unix = 1800000000;
  valid_override.custom_user_agent = "Mozilla/5.0 (CustomUA)";
  EXPECT_TRUE(manager.RegisterOverride(valid_override));

  // Active before expiry
  EXPECT_TRUE(manager.HasActiveOverride("legacy-portal.example.com", 1700000000));
  EXPECT_EQ(manager.GetUserAgentForDomain("legacy-portal.example.com", 1700000000),
            "Mozilla/5.0 (CustomUA)");

  // Expired after expiry timestamp
  EXPECT_FALSE(manager.HasActiveOverride("legacy-portal.example.com", 1900000000));
  EXPECT_NE(manager.GetUserAgentForDomain("legacy-portal.example.com", 1900000000),
            "Mozilla/5.0 (CustomUA)");
}

// 3. Verify diagnostics report sanitizer strips query parameters, tokens, and paths.
TEST(CompatibilityTest, DiagnosticsSanitizerStripsSensitiveData) {
  RawFailureReport raw;
  raw.failed_url = GURL("https://auth.company.com/sso/callback?code=super_secret_token_123&state=xyz");
  raw.http_status = 403;
  raw.net_error_code = -105;
  raw.auth_error_category = "OIDC_MISMATCH";
  raw.is_cdm_available = false;

  SanitizedFailureReport sanitized = CompatibilityDiagnostics::SanitizeReport(raw);

  // Host strictly preserved; secret tokens and query strings stripped
  EXPECT_EQ(sanitized.domain_etld_plus_one, "auth.company.com");
  EXPECT_EQ(sanitized.http_status, 403);
  EXPECT_FALSE(sanitized.is_cdm_available);
  EXPECT_EQ(sanitized.browser_version, "codem37/134.0.6998.88");

  // Verify DRM unavailable message
  std::string drm_msg = CompatibilityDiagnostics::FormatDrmUnavailableMessage();
  EXPECT_EQ(drm_msg, "Protected content requires DRM support that is not currently available.");
}

}  // namespace
}  // namespace codem37
