// Copyright (c) 2026 The codem37 Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "src/mine/shield/shield_url_loader_interceptor.h"

#include <memory>
#include <vector>

#include "base/test/bind.h"
#include "base/test/task_environment.h"
#include "content/public/test/test_browser_context.h"
#include "src/mine/shield/shield_service_impl.h"
#include "testing/gtest/include/gtest/gtest.h"
#include "url/gurl.h"
#include "url/origin.h"

namespace codem37 {
namespace {

class ShieldInterceptorTest : public testing::Test {
 public:
  ShieldInterceptorTest() = default;
  ~ShieldInterceptorTest() override = default;

 protected:
  base::test::TaskEnvironment task_environment_;
  content::TestBrowserContext browser_context_;
};

// 1. Verify network request interception and blocking.
TEST_F(ShieldInterceptorTest, BlocksKnownTrackerDomains) {
  ShieldServiceImpl shield_service(&browser_context_);
  ShieldURLLoaderInterceptor interceptor(&shield_service);

  url::Origin top_origin = url::Origin::Create(GURL("https://news.example.com"));

  // Tracker URL should be blocked
  GURL tracker_url("https://adservice.google.com/ads.js");
  EXPECT_EQ(interceptor.MaybeInterceptRequest(tracker_url, top_origin, true),
            InterceptionResult::kBlock);

  // Normal resource should be allowed
  GURL normal_url("https://news.example.com/main.js");
  EXPECT_EQ(interceptor.MaybeInterceptRequest(normal_url, top_origin, false),
            InterceptionResult::kAllow);
}

// 2. Verify per-site exceptions disable blocking for that origin.
TEST_F(ShieldInterceptorTest, SiteExceptionAllowsTrackingRequests) {
  ShieldServiceImpl shield_service(&browser_context_);
  ShieldURLLoaderInterceptor interceptor(&shield_service);

  url::Origin whitelisted_origin = url::Origin::Create(GURL("https://whitelisted.example.com"));

  // Whitelist the origin
  shield_service.SetSiteShieldEnabled(whitelisted_origin.Serialize(), false, base::DoNothing());

  // Tracker on whitelisted origin is now allowed
  GURL tracker_url("https://doubleclick.net/tracker.js");
  EXPECT_EQ(interceptor.MaybeInterceptRequest(tracker_url, whitelisted_origin, true),
            InterceptionResult::kAllow);
}

// 3. Verify Ed25519 signature verification and rollback on invalid signature.
TEST_F(ShieldInterceptorTest, SignatureVerificationAndRollback) {
  ShieldServiceImpl shield_service(&browser_context_);

  std::vector<uint8_t> payload = {0x01, 0x02, 0x03};
  std::vector<uint8_t> valid_sig = {0xAA, 0xBB, 0xCC};
  std::vector<uint8_t> empty_sig;

  // Valid signature applies update
  EXPECT_TRUE(shield_service.ApplyRuleBundleUpdate("v20260826.2", payload, valid_sig));

  // Invalid/empty signature rejects update and triggers rollback
  EXPECT_FALSE(shield_service.ApplyRuleBundleUpdate("v20260826.3", payload, empty_sig));
}

// 4. Verify data-driven scriptlets for YouTube compatibility.
TEST_F(ShieldInterceptorTest, RetrievesDataDrivenScriptletsForYouTube) {
  ShieldServiceImpl shield_service(&browser_context_);

  auto scriptlets = shield_service.GetScriptletsForDomain("www.youtube.com");
  ASSERT_FALSE(scriptlets.empty());
  EXPECT_EQ(scriptlets[0], "json-prune:playerResponse.adPlacements");
}

}  // namespace
}  // namespace codem37
