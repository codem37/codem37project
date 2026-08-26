// Copyright (c) 2026 The codem37 Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CODEM37_SRC_MINE_FETCHER_SEGMENTED_FETCH_PRODUCER_H_
#define CODEM37_SRC_MINE_FETCHER_SEGMENTED_FETCH_PRODUCER_H_

#include <memory>
#include <string>
#include <vector>

#include "base/files/file.h"
#include "base/files/file_path.h"
#include "base/functional/callback.h"
#include "base/memory/weak_ptr.h"
#include "base/sequence_checker.h"
#include "url/gurl.h"

namespace codem37 {

struct SegmentRange {
  size_t index = 0;
  uint64_t start_byte = 0;
  uint64_t end_byte = 0;
  uint64_t received_bytes = 0;
  int retry_count = 0;
  bool is_completed = false;
};

using SegmentedProgressCallback =
    base::RepeatingCallback<void(uint64_t received_bytes, uint64_t total_bytes)>;
using SegmentedCompleteCallback = base::OnceCallback<void(bool success, const std::string& error)>;

// Internal producer that downloads large range-capable files via parallel segments,
// writing directly to non-overlapping offsets in a sparse destination file.
class SegmentedFetchProducer {
 public:
  SegmentedFetchProducer(const GURL& url,
                         const base::FilePath& destination_path,
                         uint64_t total_bytes,
                         const std::string& etag,
                         SegmentedProgressCallback progress_cb,
                         SegmentedCompleteCallback complete_cb);
  ~SegmentedFetchProducer();

  SegmentedFetchProducer(const SegmentedFetchProducer&) = delete;
  SegmentedFetchProducer& operator=(const SegmentedFetchProducer&) = delete;

  void Start();
  void Pause();
  void Resume();
  void Cancel();

  static bool SaveSidecarState(const base::FilePath& destination_path,
                              const GURL& url,
                              uint64_t total_bytes,
                              const std::string& etag,
                              const std::vector<SegmentRange>& segments);
  static bool LoadSidecarState(const base::FilePath& destination_path,
                              std::string& out_etag,
                              std::vector<SegmentRange>& out_segments);

 private:
  void InitializeSparseFile();
  void OnSegmentChunkReceived(size_t segment_index,
                              uint64_t chunk_offset,
                              const std::vector<uint8_t>& chunk_data);
  void OnSegmentFinished(size_t segment_index, bool success);
  void CheckOverallCompletion();

  SEQUENCE_CHECKER(sequence_checker_);

  GURL url_;
  base::FilePath destination_path_;
  base::FilePath sidecar_path_;
  uint64_t total_bytes_ = 0;
  std::string etag_;

  SegmentedProgressCallback progress_cb_;
  SegmentedCompleteCallback complete_cb_;

  base::File destination_file_;
  std::vector<SegmentRange> segments_;
  bool is_paused_ = false;
  bool is_cancelled_ = false;

  base::WeakPtrFactory<SegmentedFetchProducer> weak_factory_{this};
};

}  // namespace codem37

#endif  // CODEM37_SRC_MINE_FETCHER_SEGMENTED_FETCH_PRODUCER_H_
