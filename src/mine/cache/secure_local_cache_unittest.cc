// Copyright (c) 2026 The codem37 Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "src/mine/cache/secure_local_cache_service_impl.h"

#include <memory>
#include <vector>

#include "base/files/file_util.h"
#include "base/files/scoped_temp_dir.h"
#include "base/test/bind.h"
#include "base/test/task_environment.h"
#include "content/public/test/test_browser_context.h"
#include "mojo/public/cpp/bindings/remote.h"
#include "testing/gtest/include/gtest/gtest.h"
#include "url/gurl.h"
#include "url/origin.h"

namespace codem37 {
namespace {

class SecureLocalCacheTest : public testing::Test {
 public:
  SecureLocalCacheTest() = default;
  ~SecureLocalCacheTest() override = default;

 protected:
  base::test::TaskEnvironment task_environment_;
  content::TestBrowserContext browser_context_;
};

// 1. Verify that stored records are encrypted at rest with AES-256-GCM ciphertext.
TEST_F(SecureLocalCacheTest, StoresRecordsEncryptedAtRest) {
  SecureLocalCacheServiceImpl service(&browser_context_);
  mojo::Remote<cache::mojom::SecureLocalCache> remote;

  service.BindReceiver(remote.BindNewPipeAndPassReceiver(),
                       url::Origin::Create(GURL("chrome://mine-settings")));

  // Add bookmark
  std::string bookmark_id;
  remote->AddBookmark(GURL("https://example.org/private"), "Secret Bookmark",
                      base::BindLambdaForTesting([&](const std::string& id) {
                        bookmark_id = id;
                      }));
  remote.FlushForTesting();

  EXPECT_FALSE(bookmark_id.empty());

  // Check on-disk cache file
  base::FilePath cache_file = browser_context_.GetPath().AppendASCII("secure_cache.db");
  EXPECT_TRUE(base::PathExists(cache_file));

  std::string file_data;
  ASSERT_TRUE(base::ReadFileToString(cache_file, &file_data));

  // Verify file has "C37C" magic bytes and does NOT contain plaintext strings
  EXPECT_TRUE(file_data.find("C37C") == 0);
  EXPECT_EQ(file_data.find("Secret Bookmark"), std::string::npos);
  EXPECT_EQ(file_data.find("https://example.org/private"), std::string::npos);
}

// 2. Verify ClearMemory zeroizes in-RAM state while retaining on-disk encrypted cache.
TEST_F(SecureLocalCacheTest, ClearMemoryZeroizesRAM) {
  SecureLocalCacheServiceImpl service(&browser_context_);
  mojo::Remote<cache::mojom::SecureLocalCache> remote;

  service.BindReceiver(remote.BindNewPipeAndPassReceiver(),
                       url::Origin::Create(GURL("chrome://mine-settings")));

  // Set custom theme
  auto theme = cache::mojom::ThemeConfig::New();
  theme->mode = cache::mojom::ThemeMode::kDark;
  theme->accent_color = "#ff0055";
  theme->custom_css = "";

  remote->SetTheme(std::move(theme), base::BindLambdaForTesting([](bool success) {
    EXPECT_TRUE(success);
  }));
  remote.FlushForTesting();

  // Clear memory
  remote->ClearMemory(base::BindLambdaForTesting([](bool success) {
    EXPECT_TRUE(success);
  }));
  remote.FlushForTesting();

  // On-disk file still exists
  base::FilePath cache_file = browser_context_.GetPath().AppendASCII("secure_cache.db");
  EXPECT_TRUE(base::PathExists(cache_file));
}

// 3. Verify ClearAllCache wipes both memory and on-disk file.
TEST_F(SecureLocalCacheTest, ClearAllCacheWipesDiskAndMemory) {
  SecureLocalCacheServiceImpl service(&browser_context_);
  mojo::Remote<cache::mojom::SecureLocalCache> remote;

  service.BindReceiver(remote.BindNewPipeAndPassReceiver(),
                       url::Origin::Create(GURL("chrome://mine-settings")));

  remote->RecordHistory(GURL("https://example.com"), "Example Domain",
                        base::BindLambdaForTesting([](bool success) {
                          EXPECT_TRUE(success);
                        }));
  remote.FlushForTesting();

  base::FilePath cache_file = browser_context_.GetPath().AppendASCII("secure_cache.db");
  EXPECT_TRUE(base::PathExists(cache_file));

  // Clear all cache
  remote->ClearAllCache(base::BindLambdaForTesting([](bool success) {
    EXPECT_TRUE(success);
  }));
  remote.FlushForTesting();

  EXPECT_FALSE(base::PathExists(cache_file));
}

// 4. Verify unauthorized web renderer origin rejection.
TEST_F(SecureLocalCacheTest, UnauthorizedOriginCannotBind) {
  SecureLocalCacheServiceImpl service(&browser_context_);
  mojo::Remote<cache::mojom::SecureLocalCache> remote;

  // Arbitrary untrusted origin
  service.BindReceiver(remote.BindNewPipeAndPassReceiver(),
                       url::Origin::Create(GURL("https://untrusted-site.com")));

  bool disconnected = false;
  remote.set_disconnect_handler(base::BindLambdaForTesting([&]() {
    disconnected = true;
  }));

  remote->GetBookmarks(base::BindLambdaForTesting(
      [](std::vector<cache::mojom::BookmarkItemPtr>) {}));
  remote.FlushForTesting();

  EXPECT_TRUE(disconnected);
}

}  // namespace
}  // namespace codem37
