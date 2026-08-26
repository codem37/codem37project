// Copyright (c) 2026 The codem37 Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "src/mine/fetcher/segmented_fetch_producer.h"

#include <memory>
#include <vector>

#include "base/files/file_util.h"
#include "base/files/scoped_temp_dir.h"
#include "base/test/bind.h"
#include "base/test/task_environment.h"
#include "testing/gtest/include/gtest/gtest.h"
#include "url/gurl.h"

namespace codem37 {
namespace {

class SegmentedFetchTest : public testing::Test {
 public:
  SegmentedFetchTest() = default;
  ~SegmentedFetchTest() override = default;

 protected:
  base::test::TaskEnvironment task_environment_;
};

// 1. Verify sidecar persistence and reload across restarts.
TEST_F(SegmentedFetchTest, SidecarPersistenceAndResume) {
  base::ScopedTempDir temp_dir;
  ASSERT_TRUE(temp_dir.CreateUniqueTempDir());

  base::FilePath dest_file = temp_dir.GetPath().AppendASCII("large_download.iso");
  GURL url("https://dist.example.com/large.iso");
  uint64_t total_bytes = 100 * 1024 * 1024; // 100 MB
  std::string etag = "\"v1.0-etag-12345\"";

  std::vector<SegmentRange> segments;
  for (size_t i = 0; i < 4; ++i) {
    SegmentRange seg;
    seg.index = i;
    seg.start_byte = i * (25 * 1024 * 1024);
    seg.end_byte = (i + 1) * (25 * 1024 * 1024) - 1;
    seg.received_bytes = (i == 0) ? (25 * 1024 * 1024) : 0;
    seg.is_completed = (i == 0);
    segments.push_back(seg);
  }

  // Save sidecar state
  ASSERT_TRUE(SegmentedFetchProducer::SaveSidecarState(
      dest_file, url, total_bytes, etag, segments));

  // Verify sidecar file exists on disk
  base::FilePath sidecar_file = dest_file.AddExtension(FILE_PATH_LITERAL(".c37state"));
  EXPECT_TRUE(base::PathExists(sidecar_file));

  // Reload sidecar state
  std::string loaded_etag;
  std::vector<SegmentRange> loaded_segments;
  ASSERT_TRUE(SegmentedFetchProducer::LoadSidecarState(
      dest_file, loaded_etag, loaded_segments));

  EXPECT_EQ(loaded_etag, etag);
  ASSERT_EQ(loaded_segments.size(), 4u);
  EXPECT_TRUE(loaded_segments[0].is_completed);
  EXPECT_EQ(loaded_segments[0].received_bytes, 25 * 1024 * 1024u);
  EXPECT_FALSE(loaded_segments[1].is_completed);
}

// 2. Verify direct-to-offset sparse file initialization and completion.
TEST_F(SegmentedFetchTest, DirectToOffsetReassembly) {
  base::ScopedTempDir temp_dir;
  ASSERT_TRUE(temp_dir.CreateUniqueTempDir());

  base::FilePath dest_file = temp_dir.GetPath().AppendASCII("sample.bin");
  GURL url("https://example.com/sample.bin");
  uint64_t total_bytes = 16 * 1024 * 1024; // 16 MB

  bool completed = false;
  uint64_t last_progress_recv = 0;

  SegmentedFetchProducer producer(
      url, dest_file, total_bytes, "\"sample-etag\"",
      base::BindLambdaForTesting([&](uint64_t recv, uint64_t tot) {
        last_progress_recv = recv;
      }),
      base::BindLambdaForTesting([&](bool success, const std::string& err) {
        completed = success;
      }));

  producer.Start();
  task_environment_.RunUntilIdle();

  EXPECT_TRUE(completed);
  EXPECT_EQ(last_progress_recv, total_bytes);
  EXPECT_TRUE(base::PathExists(dest_file));

  int64_t file_size = 0;
  base::GetFileSize(dest_file, &file_size);
  EXPECT_EQ(static_cast<uint64_t>(file_size), total_bytes);
}

}  // namespace
}  // namespace codem37
