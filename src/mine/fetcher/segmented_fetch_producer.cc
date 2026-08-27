// Copyright (c) 2026 The codem37 Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "mine/fetcher/segmented_fetch_producer.h"

#include <utility>
#include "base/logging.h"
#include "services/network/public/cpp/shared_url_loader_factory.h"

namespace codem37 {

SegmentedFetchProducer::SegmentedFetchProducer(
    const GURL& url,
    const base::FilePath& destination_path,
    uint64_t total_bytes,
    const std::string& etag,
    scoped_refptr<network::SharedURLLoaderFactory> url_loader_factory,
    SegmentedProgressCallback progress_cb,
    SegmentedCompleteCallback complete_cb)
    : url_(url),
      destination_path_(destination_path),
      total_bytes_(total_bytes),
      etag_(etag),
      url_loader_factory_(std::move(url_loader_factory)),
      progress_cb_(std::move(progress_cb)),
      complete_cb_(std::move(complete_cb)) {
  target_file_ = storage::DownloadFile::Create(destination_path_, total_bytes_);
  scheduler_ = std::make_unique<engine::SegmentScheduler>(total_bytes_, 4);

  // Check for existing sidecar (.c37state)
  auto loaded_state = storage::DownloadStateStore::Load(destination_path_);
  if (loaded_state.has_value() && loaded_state->url == url_ &&
      loaded_state->total_bytes == total_bytes_) {
    std::vector<engine::DownloadRange> ranges;
    uint32_t id = 1;
    for (const auto& r : loaded_state->ranges) {
      engine::DownloadRange range;
      range.id = id++;
      range.start = r.start;
      range.end = r.end;
      range.completed = r.completed;
      range.status = r.is_finished ? engine::RangeStatus::kCompleted
                                   : engine::RangeStatus::kPending;
      ranges.push_back(range);
    }
    scheduler_->LoadRanges(ranges);
  }
}

SegmentedFetchProducer::~SegmentedFetchProducer() {
  Cancel();
}

void SegmentedFetchProducer::Start() {
  DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);
  if (!target_file_ || is_cancelled_) {
    if (complete_cb_) {
      std::move(complete_cb_).Run(false, "Failed to initialize target storage.");
    }
    return;
  }

  is_paused_ = false;
  // Spawn initial set of parallel workers
  for (size_t i = 0; i < 4; ++i) {
    SpawnWorker(static_cast<int>(i));
  }
}

void SegmentedFetchProducer::SpawnWorker(int worker_id) {
  if (is_paused_ || is_cancelled_ || !scheduler_) {
    return;
  }

  auto next_range = scheduler_->GetNextWorkRange(worker_id);
  if (!next_range.has_value()) {
    if (scheduler_->IsAllCompleted()) {
      target_file_->Flush();
      storage::DownloadStateStore::Remove(destination_path_);
      if (complete_cb_) {
        std::move(complete_cb_).Run(true, "");
      }
    }
    return;
  }

  auto worker = std::make_unique<engine::SegmentWorker>(
      worker_id, url_loader_factory_, target_file_.get(),
      base::BindRepeating(&SegmentedFetchProducer::OnWorkerProgress,
                          weak_factory_.GetWeakPtr()),
      base::BindOnce(&SegmentedFetchProducer::OnWorkerCompleted,
                     weak_factory_.GetWeakPtr()));

  auto* worker_ptr = worker.get();
  workers_.push_back(std::move(worker));

  std::optional<std::string> expected_etag =
      etag_.empty() ? std::nullopt : std::make_optional(etag_);
  worker_ptr->StartRange(url_, *next_range, expected_etag);
}

void SegmentedFetchProducer::OnWorkerProgress(uint32_t range_id, uint64_t bytes_written) {
  DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);
  if (scheduler_) {
    scheduler_->UpdateProgress(range_id, bytes_written);
    if (progress_cb_) {
      progress_cb_.Run(scheduler_->GetTotalCompletedBytes(), total_bytes_);
    }
  }
}

void SegmentedFetchProducer::OnWorkerCompleted(
    int worker_id, uint32_t range_id, bool success, uint64_t bytes_transferred) {
  DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);
  if (!scheduler_) return;

  if (success) {
    scheduler_->MarkCompleted(range_id);
    PersistSidecar();
  } else {
    scheduler_->MarkFailed(range_id);
  }

  if (scheduler_->IsAllCompleted()) {
    target_file_->Flush();
    storage::DownloadStateStore::Remove(destination_path_);
    if (complete_cb_) {
      std::move(complete_cb_).Run(true, "");
    }
  } else {
    // Worker claims next available or split range
    SpawnWorker(worker_id);
  }
}

void SegmentedFetchProducer::PersistSidecar() {
  if (!scheduler_) return;
  storage::PersistedDownloadState state;
  state.url = url_;
  state.destination = destination_path_;
  state.total_bytes = total_bytes_;
  state.etag = etag_;

  auto snapshot = scheduler_->GetSnapshot();
  for (const auto& r : snapshot) {
    storage::PersistedRangeState p_range;
    p_range.start = r.start;
    p_range.end = r.end;
    p_range.completed = r.completed;
    p_range.is_finished = (r.status == engine::RangeStatus::kCompleted);
    state.ranges.push_back(p_range);
  }
  storage::DownloadStateStore::Save(destination_path_, state);
}

void SegmentedFetchProducer::Pause() {
  is_paused_ = true;
  for (auto& w : workers_) {
    w->Cancel();
  }
  workers_.clear();
  PersistSidecar();
}

void SegmentedFetchProducer::Resume() {
  if (is_paused_) {
    Start();
  }
}

void SegmentedFetchProducer::Cancel() {
  is_cancelled_ = true;
  for (auto& w : workers_) {
    w->Cancel();
  }
  workers_.clear();
  if (target_file_) {
    target_file_->Close();
  }
}

}  // namespace codem37
