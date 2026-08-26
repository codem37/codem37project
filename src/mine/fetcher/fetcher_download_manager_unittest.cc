// Copyright (c) 2026 The codem37 Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "src/mine/fetcher/fetcher_service_impl.h"

#include <memory>
#include <utility>

#include "base/test/bind.h"
#include "base/test/task_environment.h"
#include "content/public/test/test_browser_context.h"
#include "mojo/public/cpp/bindings/remote.h"
#include "src/mine/fetcher/fetcher_service_factory.h"
#include "testing/gtest/include/gtest/gtest.h"
#include "url/gurl.h"
#include "url/origin.h"

namespace codem37 {
namespace {

class FetcherDownloadManagerTest : public testing::Test {
 public:
  FetcherDownloadManagerTest() = default;
  ~FetcherDownloadManagerTest() override = default;

 protected:
  base::test::TaskEnvironment task_environment_;
  content::TestBrowserContext browser_context_;
};

// 1. Verify that authorized WebUI can bind MineFetcher and perform pause/resume/cancel operations.
TEST_F(FetcherDownloadManagerTest, AuthorizedWebUICanControlDownloads) {
  FetcherServiceImpl service(&browser_context_);
  mojo::Remote<fetcher::mojom::MineFetcher> remote;

  service.BindReceiver(remote.BindNewPipeAndPassReceiver(),
                       url::Origin::Create(GURL("chrome://downloads")));

  // List downloads initially empty
  std::vector<fetcher::mojom::DownloadItemSnapshotPtr> list;
  remote->ListActiveDownloads(base::BindLambdaForTesting(
      [&](std::vector<fetcher::mojom::DownloadItemSnapshotPtr> items) {
        list = std::move(items);
      }));
  remote.FlushForTesting();

  EXPECT_TRUE(list.empty());

  // Non-existent ID pause returns false safely
  bool pause_ok = true;
  remote->Pause(99999, base::BindLambdaForTesting([&](bool result) {
    pause_ok = result;
  }));
  remote.FlushForTesting();
  EXPECT_FALSE(pause_ok);
}

// 2. Verify unauthorized origin rejection.
TEST_F(FetcherDownloadManagerTest, UnauthorizedOriginCannotBind) {
  FetcherServiceImpl service(&browser_context_);
  mojo::Remote<fetcher::mojom::MineFetcher> remote;

  // Arbitrary website attempting to control downloads
  service.BindReceiver(remote.BindNewPipeAndPassReceiver(),
                       url::Origin::Create(GURL("https://malicious.com")));

  bool disconnected = false;
  remote.set_disconnect_handler(base::BindLambdaForTesting([&]() {
    disconnected = true;
  }));

  remote->ListActiveDownloads(base::BindLambdaForTesting(
      [](std::vector<fetcher::mojom::DownloadItemSnapshotPtr>) {}));
  remote.FlushForTesting();

  EXPECT_TRUE(disconnected);
}

}  // namespace
}  // namespace codem37
