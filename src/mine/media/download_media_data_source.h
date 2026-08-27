// Copyright 2026 The codem37 Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef MINE_MEDIA_DOWNLOAD_MEDIA_DATA_SOURCE_H_
#define MINE_MEDIA_DOWNLOAD_MEDIA_DATA_SOURCE_H_

#include <cstdint>
#include <memory>
#include "base/memory/scoped_refptr.h"
#include "base/synchronization/lock.h"

namespace mine {
namespace storage {
class DownloadFile;
}

namespace engine {
class SegmentScheduler;
}

namespace media {

// Range-aware data source feeding Chromium's media demuxer & FFmpeg pipeline
// directly from the active segmented download file.
class DownloadMediaDataSource {
 public:
  DownloadMediaDataSource(storage::DownloadFile* file,
                          engine::SegmentScheduler* scheduler);
  ~DownloadMediaDataSource();

  // Non-copyable
  DownloadMediaDataSource(const DownloadMediaDataSource&) = delete;
  DownloadMediaDataSource& operator=(const DownloadMediaDataSource&) = delete;

  // Reads `length` bytes at `offset` into `buffer`.
  // Returns number of bytes read, or 0 if range is currently pending download.
  int64_t Read(uint64_t offset, uint8_t* buffer, size_t length);

  // Queries if a byte range is already written to disk.
  bool IsRangeAvailable(uint64_t offset, size_t length) const;

  uint64_t total_size() const;

 private:
  storage::DownloadFile* file_;          // Weak ref owned by producer
  engine::SegmentScheduler* scheduler_;  // Weak ref owned by producer
};

}  // namespace media
}  // namespace mine

#endif  // MINE_MEDIA_DOWNLOAD_MEDIA_DATA_SOURCE_H_
