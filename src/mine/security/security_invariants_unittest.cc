// Copyright (c) 2026 The codem37 Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/test/bind.h"
#include "base/test/task_environment.h"
#include "content/public/test/test_browser_context.h"
#include "mojo/public/cpp/bindings/remote.h"
#include "src/mine/cache/secure_local_cache_service_impl.h"
#include "src/mine/vault/vault_service_impl.h"
#include "testing/gtest/include/gtest/gtest.h"
#include "url/gurl.h"
#include "url/origin.h"

namespace codem37 {
namespace {

class SecurityInvariantsTest : public testing::Test {
 public:
  SecurityInvariantsTest() = default;
  ~SecurityInvariantsTest() override = default;

 protected:
  base::test::TaskEnvironment task_environment_;
  content::TestBrowserContext browser_context_a_;
  content::TestBrowserContext browser_context_b_;
};

// 1. Invariant: Compromised renderer cannot bind VaultService.
TEST_F(SecurityInvariantsTest, CompromisedRendererCannotBindVault) {
  VaultServiceImpl vault(&browser_context_a_);
  mojo::Remote<vault::mojom::VaultService> remote;

  // Hostile origin attempting to obtain Vault interface
  vault.BindReceiver(remote.BindNewPipeAndPassReceiver(),
                     url::Origin::Create(GURL("https://hostile-site.com")));

  bool disconnected = false;
  remote.set_disconnect_handler(base::BindLambdaForTesting([&]() {
    disconnected = true;
  }));

  remote->GetLockState(base::BindLambdaForTesting([](bool) {}));
  remote.FlushForTesting();

  EXPECT_TRUE(disconnected);
}

// 2. Invariant: Compromised renderer cannot bind SecureLocalCacheService.
TEST_F(SecurityInvariantsTest, CompromisedRendererCannotBindCache) {
  SecureLocalCacheServiceImpl cache(&browser_context_a_);
  mojo::Remote<cache::mojom::SecureLocalCache> remote;

  // Hostile origin attempting to obtain Cache interface
  cache.BindReceiver(remote.BindNewPipeAndPassReceiver(),
                     url::Origin::Create(GURL("https://malicious-script.com")));

  bool disconnected = false;
  remote.set_disconnect_handler(base::BindLambdaForTesting([&]() {
    disconnected = true;
  }));

  remote->GetBookmarks(base::BindLambdaForTesting(
      [](std::vector<cache::mojom::BookmarkItemPtr>) {}));
  remote.FlushForTesting();

  EXPECT_TRUE(disconnected);
}

// 3. Invariant: Cross-profile isolation is structurally preserved.
TEST_F(SecurityInvariantsTest, CrossProfileIsolationEnforced) {
  SecureLocalCacheServiceImpl cache_a(&browser_context_a_);
  SecureLocalCacheServiceImpl cache_b(&browser_context_b_);

  mojo::Remote<cache::mojom::SecureLocalCache> remote_a;
  mojo::Remote<cache::mojom::SecureLocalCache> remote_b;

  cache_a.BindReceiver(remote_a.BindNewPipeAndPassReceiver(),
                       url::Origin::Create(GURL("chrome://mine-settings")));
  cache_b.BindReceiver(remote_b.BindNewPipeAndPassReceiver(),
                       url::Origin::Create(GURL("chrome://mine-settings")));

  // Add bookmark to Profile A
  remote_a->AddBookmark(GURL("https://profile-a.com"), "Profile A Bookmark",
                        base::BindLambdaForTesting([](const std::string&) {}));
  remote_a.FlushForTesting();

  // Query bookmarks from Profile B -> must be empty
  std::vector<cache::mojom::BookmarkItemPtr> list_b;
  remote_b->GetBookmarks(base::BindLambdaForTesting(
      [&](std::vector<cache::mojom::BookmarkItemPtr> items) {
        list_b = std::move(items);
      }));
  remote_b.FlushForTesting();

  EXPECT_TRUE(list_b.empty());
}

}  // namespace
}  // namespace codem37
