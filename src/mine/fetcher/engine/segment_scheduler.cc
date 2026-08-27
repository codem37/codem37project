// Copyright 2026 The codem37 Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "mine/fetcher/engine/segment_scheduler.h"

#include <algorithm>
#include "base/logging.h"

namespace mine {
namespace engine {

SegmentScheduler::SegmentScheduler(uint64_t total_size, size_t initial_workers)
    : total_size_(total_size) {
  if (total_size == 0) return;

  size_t num_chunks = std::min(initial_workers, kMaxConcurrentWorkers);
  if (num_chunks == 0) num_chunks = 1;

  // Don't split smaller than kMinSegmentSize
  while (num_chunks > 1 && (total_size / num_chunks) < kMinSegmentSize) {
    num_chunks--;
  }

  uint64_t chunk_size = total_size / num_chunks;
  uint64_t current_offset = 0;

  for (size_t i = 0; i < num_chunks; ++i) {
    uint64_t end = (i == num_chunks - 1) ? (total_size - 1) : (current_offset + chunk_size - 1);
    DownloadRange range;
    range.id = next_range_id_++;
    range.start = current_offset;
    range.end = end;
    range.completed = 0;
    range.status = RangeStatus::kPending;
    ranges_.push_back(range);
    current_offset = end + 1;
  }
}

SegmentScheduler::~SegmentScheduler() = default;

void SegmentScheduler::LoadRanges(const std::vector<DownloadRange>& ranges) {
  base::AutoLock auto_lock(lock_);
  ranges_ = ranges;
  for (const auto& r : ranges_) {
    if (r.id >= next_range_id_) {
      next_range_id_ = r.id + 1;
    }
  }
}

std::optional<DownloadRange> SegmentScheduler::GetNextWorkRange(int worker_id) {
  base::AutoLock auto_lock(lock_);

  // 1. Look for pending range first
  for (auto& r : ranges_) {
    if (r.status == RangeStatus::kPending) {
      r.status = RangeStatus::kActive;
      r.worker_id = worker_id;
      return r;
    }
  }

  // 2. If no pending range, find the largest active range that can be split at midpoint
  DownloadRange* best_candidate = nullptr;
  uint64_t max_remaining = 0;

  for (auto& r : ranges_) {
    if (r.status == RangeStatus::kActive) {
      uint64_t current_pos = r.start + r.completed;
      if (r.end > current_pos) {
        uint64_t remaining = r.end - current_pos + 1;
        if (remaining >= (2 * kMinSegmentSize) && remaining > max_remaining) {
          max_remaining = remaining;
          best_candidate = &r;
        }
      }
    }
  }

  if (best_candidate) {
    uint64_t current_pos = best_candidate->start + best_candidate->completed;
    uint64_t midpoint = current_pos + (best_candidate->end - current_pos) / 2;

    // Create new split range for the second half
    DownloadRange new_range;
    new_range.id = next_range_id_++;
    new_range.start = midpoint + 1;
    new_range.end = best_candidate->end;
    new_range.completed = 0;
    new_range.status = RangeStatus::kActive;
    new_range.worker_id = worker_id;

    // Truncate original candidate range to midpoint
    best_candidate->end = midpoint;

    ranges_.push_back(new_range);
    return new_range;
  }

  return std::nullopt;
}

void SegmentScheduler::UpdateProgress(uint32_t range_id, uint64_t bytes_written) {
  base::AutoLock auto_lock(lock_);
  for (auto& r : ranges_) {
    if (r.id == range_id) {
      r.completed += bytes_written;
      if (r.start + r.completed > r.end + 1) {
        r.completed = (r.end - r.start + 1);
      }
      break;
    }
  }
}

void SegmentScheduler::MarkCompleted(uint32_t range_id) {
  base::AutoLock auto_lock(lock_);
  for (auto& r : ranges_) {
    if (r.id == range_id) {
      r.completed = (r.end - r.start + 1);
      r.status = RangeStatus::kCompleted;
      r.worker_id = -1;
      break;
    }
  }
}

void SegmentScheduler::MarkFailed(uint32_t range_id) {
  base::AutoLock auto_lock(lock_);
  for (auto& r : ranges_) {
    if (r.id == range_id) {
      r.status = RangeStatus::kPending;
      r.worker_id = -1;
      break;
    }
  }
}

std::vector<DownloadRange> SegmentScheduler::GetSnapshot() const {
  base::AutoLock auto_lock(lock_);
  return ranges_;
}

uint64_t SegmentScheduler::GetTotalCompletedBytes() const {
  base::AutoLock auto_lock(lock_);
  uint64_t total = 0;
  for (const auto& r : ranges_) {
    total += r.completed;
  }
  return total;
}

bool SegmentScheduler::IsAllCompleted() const {
  base::AutoLock auto_lock(lock_);
  if (ranges_.empty()) return false;
  for (const auto& r : ranges_) {
    if (r.status != RangeStatus::kCompleted) return false;
  }
  return true;
}

}  // namespace engine
}  // namespace mine
