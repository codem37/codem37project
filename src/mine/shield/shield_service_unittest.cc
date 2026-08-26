// Copyright (c) 2026 The codem37 Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "src/mine/shield/shield_service_impl.h"

#include <memory>
#include <string>
#include <utility>

#include "base/test/bind.h"
#include "base/test/task_environment.h"
#include "content/public/test/test_browser_context.h"
#include "mojo/public/cpp/bindings/remote.h"
#include "testing/gtest/include/gtest/gtest.h"
#include "url/gurl.h"
#include "url/origin.h"

namespace codem37 {
namespace {

class ShieldServiceTest : public testing::Test {
 public:
  ShieldServiceTest() = default;
  ~ShieldServiceTest() override = default;

 protected:
  base::test::TaskEnvironment task_environment_;
  content::TestBrowserContext browser_context_;
};

// 1. Verify that chrome://shield can bind, query subscriptions, and modify toggles.
TEST_F(ShieldServiceTest, AuthorizedOriginCanManageSubscriptions) {
  ShieldServiceImpl service(&browser_context_);
  mojo::Remote<shield::mojom::ShieldService> remote;

  service.BindReceiver(remote.BindNewPipeAndPassReceiver(),
                       url::Origin::Create(GURL("chrome://shield")));

  // Get default subscription
  std::vector<shield::mojom::FilterSubscriptionPtr> subs;
  remote->GetSubscriptions(base::BindLambdaForTesting(
      [&](std::vector<shield::mojom::FilterSubscriptionPtr> list) {
        subs = std::move(list);
      }));
  remote.FlushForTesting();

  ASSERT_EQ(subs.size(), 1u);
  EXPECT_EQ(subs[0]->id, "default_easy_privacy");
  EXPECT_TRUE(subs[0]->is_enabled);

  // Disable subscription
  remote->SetSubscriptionEnabled("default_easy_privacy", false,
      base::BindLambdaForTesting([](shield::mojom::ShieldStatus status) {
        EXPECT_EQ(status, shield::mojom::ShieldStatus::kSuccess);
      }));
  remote.FlushForTesting();
}

// 2. Verify unauthorized origin rejection.
TEST_F(ShieldServiceTest, UnauthorizedOriginCannotBind) {
  ShieldServiceImpl service(&browser_context_);
  mojo::Remote<shield::mojom::ShieldService> remote;

  // Attempt binding from chrome://vault (cross-WebUI isolation) or web
  service.BindReceiver(remote.BindNewPipeAndPassReceiver(),
                       url::Origin::Create(GURL("chrome://vault")));

  bool disconnected = false;
  remote.set_disconnect_handler(base::BindLambdaForTesting([&]() {
    disconnected = true;
  }));

  remote->GetSubscriptions(base::BindLambdaForTesting([](std::vector<shield::mojom::FilterSubscriptionPtr>) {}));
  remote.FlushForTesting();

  EXPECT_TRUE(disconnected);
}

// 3. Verify internal blocking decisions.
TEST_F(ShieldServiceTest, RequestBlockingLogic) {
  ShieldServiceImpl service(&browser_context_);

  // Tracker domain blocked
  EXPECT_TRUE(service.ShouldBlockRequest(
      GURL("https://tracker.analytics.com/log"), GURL("https://news.com")));

  // Non-tracker domain allowed
  EXPECT_FALSE(service.ShouldBlockRequest(
      GURL("https://cdn.news.com/image.png"), GURL("https://news.com")));

  // Per-site disable toggle
  service.SetSiteShieldEnabled("news.com", false,
      base::BindLambdaForTesting([](shield::mojom::ShieldStatus) {}));

  // Now tracker should not be blocked on news.com
  EXPECT_FALSE(service.ShouldBlockRequest(
      GURL("https://tracker.analytics.com/log"), GURL("https://news.com")));
}

}  // namespace
}  // namespace codem37
