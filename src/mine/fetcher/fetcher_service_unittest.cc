// Copyright (c) 2026 The codem37 Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "src/mine/fetcher/fetcher_service_impl.h"

#include <utility>

#include "base/test/bind.h"
#include "base/test/task_environment.h"
#include "content/public/test/test_browser_context.h"
#include "testing/gtest/include/gtest/gtest.h"
#include "url/gurl.h"

namespace codem37 {
namespace {

class FetcherServiceTest : public testing::Test {
 public:
  FetcherServiceTest() = default;
  ~FetcherServiceTest() override = default;

 protected:
  base::test::TaskEnvironment task_environment_;
  content::TestBrowserContext browser_context_;
};

TEST_F(FetcherServiceTest, StartAndCompleteSegmentedFetch) {
  FetcherServiceImpl service(&browser_context_);

  bool called = false;
  FetchResult fetch_result;

  service.StartSegmentedFetch(
      GURL("https://updates.codem37.org/bundle.tar"), 1024 * 1024,
      base::BindLambdaForTesting([&](FetchResult res) {
        called = true;
        fetch_result = std::move(res);
      }));

  task_environment_.RunUntilIdle();

  EXPECT_TRUE(called);
  EXPECT_TRUE(fetch_result.success);
  EXPECT_EQ(fetch_result.response_code, 200);
}

TEST_F(FetcherServiceTest, RejectsInvalidUrl) {
  FetcherServiceImpl service(&browser_context_);

  bool called = false;
  FetchResult fetch_result;

  service.StartSegmentedFetch(
      GURL("not-a-valid-url"), 1024 * 1024,
      base::BindLambdaForTesting([&](FetchResult res) {
        called = true;
        fetch_result = std::move(res);
      }));

  EXPECT_TRUE(called);
  EXPECT_FALSE(fetch_result.success);
  EXPECT_EQ(fetch_result.error_message, "Invalid URL");
}

}  // namespace
}  // namespace codem37
