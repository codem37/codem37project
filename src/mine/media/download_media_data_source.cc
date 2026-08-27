// Copyright 2026 The codem37 Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "mine/media/download_media_data_source.h"

#include <algorithm>
#include "mine/fetcher/engine/segment_scheduler.h"
#include "mine/fetcher/storage/download_file.h"

namespace mine {
namespace media {

DownloadMediaDataSource::DownloadMediaDataSource(
    storage::DownloadFile* file,
    engine::SegmentScheduler* scheduler)
    : file_(file), scheduler_(scheduler) {}

DownloadMediaDataSource::~DownloadMediaDataSource() = default;

bool DownloadMediaDataSource::IsRangeAvailable(uint64_t offset, size_t length) const {
  if (!scheduler_) return false;
  uint64_t req_end = offset + length - 1;

  // Check if requested range is fully covered by completed chunks
  auto snapshot = scheduler_->GetSnapshot();
  for (const auto& r : snapshot) {
    if (r.start <= offset && (r.start + r.completed) > req_end) {
      return true;
    }
  }
  return false;
}

int64_t DownloadMediaDataSource::Read(uint64_t offset, uint8_t* buffer, size_t length) {
  if (!file_ || !buffer || length == 0) {
    return -1;
  }

  // If entire range is not fully downloaded, read whatever contiguous bytes are available
  if (!IsRangeAvailable(offset, length)) {
    // Check how much is available from current offset
    size_t available_len = 0;
    if (scheduler_) {
      auto snapshot = scheduler_->GetSnapshot();
      for (const auto& r : snapshot) {
        if (r.start <= offset && (r.start + r.completed) > offset) {
          available_len = static_cast<size_t>((r.start + r.completed) - offset);
          break;
        }
      }
    }
    if (available_len == 0) {
      return 0; // Wait for background workers to fetch range
    }
    length = std::min(length, available_len);
  }

  return file_->ReadAt(offset, buffer, length);
}

uint64_t DownloadMediaDataSource::total_size() const {
  return file_ ? file_->total_size() : 0;
}

}  // namespace media
}  // namespace mine
