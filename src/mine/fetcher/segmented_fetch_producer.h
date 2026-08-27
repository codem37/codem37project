// Copyright (c) 2026 The codem37 Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CODEM37_SRC_MINE_FETCHER_SEGMENTED_FETCH_PRODUCER_H_
#define CODEM37_SRC_MINE_FETCHER_SEGMENTED_FETCH_PRODUCER_H_

#include <memory>
#include <string>
#include <vector>

#include "base/files/file_path.h"
#include "base/functional/callback.h"
#include "base/memory/scoped_refptr.h"
#include "base/memory/weak_ptr.h"
#include "base/sequence_checker.h"
#include "mine/fetcher/engine/segment_scheduler.h"
#include "mine/fetcher/engine/segment_worker.h"
#include "mine/fetcher/storage/download_file.h"
#include "mine/fetcher/storage/download_state_store.h"
#include "url/gurl.h"

namespace network {
class SharedURLLoaderFactory;
}

namespace codem37 {

using SegmentedProgressCallback =
    base::RepeatingCallback<void(uint64_t received_bytes, uint64_t total_bytes)>;
using SegmentedCompleteCallback = base::OnceCallback<void(bool success, const std::string& error)>;

// Production-grade orchestrator that downloads large range-capable files via
// parallel dynamic segments, writing directly to non-overlapping offsets in a sparse target.
class SegmentedFetchProducer {
 public:
  SegmentedFetchProducer(const GURL& url,
                         const base::FilePath& destination_path,
                         uint64_t total_bytes,
                         const std::string& etag,
                         scoped_refptr<network::SharedURLLoaderFactory> url_loader_factory,
                         SegmentedProgressCallback progress_cb,
                         SegmentedCompleteCallback complete_cb);
  ~SegmentedFetchProducer();

  SegmentedFetchProducer(const SegmentedFetchProducer&) = delete;
  SegmentedFetchProducer& operator=(const SegmentedFetchProducer&) = delete;

  void Start();
  void Pause();
  void Resume();
  void Cancel();

  storage::DownloadFile* target_file() { return target_file_.get(); }
  engine::SegmentScheduler* scheduler() { return scheduler_.get(); }

 private:
  void SpawnWorker(int worker_id);
  void OnWorkerProgress(uint32_t range_id, uint64_t bytes_written);
  void OnWorkerCompleted(int worker_id, uint32_t range_id, bool success, uint64_t bytes_transferred);
  void PersistSidecar();

  GURL url_;
  base::FilePath destination_path_;
  uint64_t total_bytes_;
  std::string etag_;
  scoped_refptr<network::SharedURLLoaderFactory> url_loader_factory_;
  SegmentedProgressCallback progress_cb_;
  SegmentedCompleteCallback complete_cb_;

  std::unique_ptr<storage::DownloadFile> target_file_;
  std::unique_ptr<engine::SegmentScheduler> scheduler_;
  std::vector<std::unique_ptr<engine::SegmentWorker>> workers_;

  bool is_paused_ = false;
  bool is_cancelled_ = false;

  SEQUENCE_CHECKER(sequence_checker_);
  base::WeakPtrFactory<SegmentedFetchProducer> weak_factory_{this};
};

}  // namespace codem37

#endif  // CODEM37_SRC_MINE_FETCHER_SEGMENTED_FETCH_PRODUCER_H_
