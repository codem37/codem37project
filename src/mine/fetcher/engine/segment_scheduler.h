// Copyright 2026 The codem37 Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef MINE_FETCHER_ENGINE_SEGMENT_SCHEDULER_H_
#define MINE_FETCHER_ENGINE_SEGMENT_SCHEDULER_H_

#include <cstdint>
#include <memory>
#include <optional>
#include <vector>
#include "base/synchronization/lock.h"

namespace mine {
namespace engine {

enum class RangeStatus {
  kPending,
  kActive,
  kCompleted,
  kFailed,
};

struct DownloadRange {
  uint32_t id;
  uint64_t start;
  uint64_t end;
  uint64_t completed;
  RangeStatus status;
  int worker_id = -1;
};

// Thread-safe dynamic range scheduler with midpoint splitting.
class SegmentScheduler {
 public:
  static constexpr size_t kMaxConcurrentWorkers = 8;
  static constexpr uint64_t kMinSegmentSize = 8 * 1024 * 1024; // 8 MB

  SegmentScheduler(uint64_t total_size, size_t initial_workers = 4);
  ~SegmentScheduler();

  // Initializes from existing persisted ranges on resume
  void LoadRanges(const std::vector<DownloadRange>& ranges);

  // Claims a pending range or splits the largest remaining active range.
  std::optional<DownloadRange> GetNextWorkRange(int worker_id);

  // Updates progress for a range
  void UpdateProgress(uint32_t range_id, uint64_t bytes_written);

  // Marks a range as completed
  void MarkCompleted(uint32_t range_id);

  // Marks a range as failed (re-queues for retry)
  void MarkFailed(uint32_t range_id);

  // Returns all current ranges (for .c37state snapshot persistence)
  std::vector<DownloadRange> GetSnapshot() const;

  // Calculates total progress across all ranges
  uint64_t GetTotalCompletedBytes() const;
  bool IsAllCompleted() const;

  uint64_t total_size() const { return total_size_; }

 private:
  uint64_t total_size_;
  mutable base::Lock lock_;
  std::vector<DownloadRange> ranges_;
  uint32_t next_range_id_ = 1;
};

}  // namespace engine
}  // namespace mine

#endif  // MINE_FETCHER_ENGINE_SEGMENT_SCHEDULER_H_
