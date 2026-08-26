// Copyright (c) 2026 The codem37 Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "src/mine/fetcher/fetcher_service_impl.h"

#include <utility>

#include "base/files/file_path.h"
#include "base/time/time.h"

namespace codem37 {

namespace {

constexpr char kMineFetcherScheme[] = "chrome";
constexpr base::TimeDelta kProgressCoalesceInterval = base::Milliseconds(250);

bool IsAuthorizedFetcherOrigin(const url::Origin& origin) {
  return origin.scheme() == kMineFetcherScheme;
}

}  // namespace

FetcherServiceImpl::FetcherServiceImpl(content::BrowserContext* context)
    : context_(context) {
  DETACH_FROM_SEQUENCE(sequence_checker_);
  progress_timer_.Start(
      FROM_HERE, kProgressCoalesceInterval,
      base::BindRepeating(&FetcherServiceImpl::FlushCoalescedProgress,
                          base::Unretained(this)));
}

FetcherServiceImpl::~FetcherServiceImpl() {
  Shutdown();
}

void FetcherServiceImpl::Shutdown() {
  DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);
  progress_timer_.Stop();
  active_producers_.clear();
  active_downloads_.clear();
  receivers_.Clear();
  observers_.Clear();
}

void FetcherServiceImpl::BindReceiver(
    mojo::PendingReceiver<fetcher::mojom::MineFetcher> receiver,
    const url::Origin& caller_origin) {
  if (!IsAuthorizedFetcherOrigin(caller_origin)) {
    return;
  }
  receivers_.Add(this, std::move(receiver), caller_origin);
}

void FetcherServiceImpl::StartSegmentedFetch(
    const GURL& url,
    const base::FilePath& destination_path,
    uint64_t total_bytes,
    const std::string& etag,
    FetchCompletionCallback callback) {
  DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);

  uint64_t id = next_download_id_++;
  auto snapshot = fetcher::mojom::DownloadItemSnapshot::New();
  snapshot->download_id = id;
  snapshot->url = url;
  snapshot->display_filename = destination_path.BaseName().AsUTF8Unsafe();
  snapshot->total_bytes = total_bytes;
  snapshot->received_bytes = 0;
  snapshot->state = fetcher::mojom::DownloadState::kInProgress;
  snapshot->percent_complete = 0;
  snapshot->current_speed_bps = 10 * 1024 * 1024; // Simulated 10 MB/s
  snapshot->estimated_time_remaining_sec = (total_bytes > 0) ? (total_bytes / snapshot->current_speed_bps) : 0;
  snapshot->is_segmented = (total_bytes >= 100 * 1024 * 1024);
  snapshot->start_time_unix = base::Time::Now().ToTimeT();

  for (auto& observer : observers_) {
    observer->OnDownloadCreated(snapshot.Clone());
  }

  active_downloads_[id] = std::move(snapshot);

  auto producer = std::make_unique<SegmentedFetchProducer>(
      url, destination_path, total_bytes, etag,
      base::BindRepeating(
          [](base::WeakPtr<FetcherServiceImpl> self, uint64_t d_id,
             uint64_t recv, uint64_t tot) {
            if (!self) return;
            auto it = self->active_downloads_.find(d_id);
            if (it != self->active_downloads_.end()) {
              it->second->received_bytes = recv;
              it->second->percent_complete = (tot > 0) ? static_cast<int32_t>((recv * 100) / tot) : 0;
            }
          },
          weak_factory_.GetWeakPtr(), id),
      base::BindOnce(
          [](base::WeakPtr<FetcherServiceImpl> self, uint64_t d_id,
             FetchCompletionCallback cb, bool success, const std::string& err) {
            if (!self) return;
            auto it = self->active_downloads_.find(d_id);
            if (it != self->active_downloads_.end()) {
              it->second->state = success ? fetcher::mojom::DownloadState::kCompleted
                                          : fetcher::mojom::DownloadState::kInterrupted;
              // Immediate push for terminal event
              for (auto& observer : self->observers_) {
                observer->OnDownloadUpdated(it->second.Clone());
              }
            }
            FetchResult res;
            res.success = success;
            res.response_code = success ? 200 : 500;
            res.error_message = err;
            std::move(cb).Run(res);
          },
          weak_factory_.GetWeakPtr(), id, std::move(callback)));

  producer->Start();
  active_producers_[id] = std::move(producer);
}

void FetcherServiceImpl::CancelFetch(const GURL& url) {
  DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);
  for (auto it = active_downloads_.begin(); it != active_downloads_.end(); ++it) {
    if (it->second->url == url) {
      Cancel(it->first, base::DoNothing());
      break;
    }
  }
}

void FetcherServiceImpl::AddObserver(
    mojo::PendingRemote<fetcher::mojom::MineFetcherObserver> observer) {
  observers_.Add(std::move(observer));
}

void FetcherServiceImpl::GetSnapshot(GetSnapshotCallback callback) {
  std::vector<fetcher::mojom::DownloadItemSnapshotPtr> list;
  for (const auto& [id, snapshot] : active_downloads_) {
    list.push_back(snapshot.Clone());
  }
  std::move(callback).Run(std::move(list));
}

void FetcherServiceImpl::StartFromUrl(
    const GURL& url,
    const std::optional<std::string>& suggested_filename,
    StartFromUrlCallback callback) {
  uint64_t id = next_download_id_++;
  auto snapshot = fetcher::mojom::DownloadItemSnapshot::New();
  snapshot->download_id = id;
  snapshot->url = url;
  snapshot->display_filename = suggested_filename.value_or("download.bin");
  snapshot->total_bytes = 0;
  snapshot->received_bytes = 0;
  snapshot->state = fetcher::mojom::DownloadState::kInProgress;
  snapshot->percent_complete = 0;
  snapshot->current_speed_bps = 0;
  snapshot->estimated_time_remaining_sec = 0;
  snapshot->is_segmented = false;
  snapshot->start_time_unix = base::Time::Now().ToTimeT();

  for (auto& observer : observers_) {
    observer->OnDownloadCreated(snapshot.Clone());
  }

  active_downloads_[id] = std::move(snapshot);
  std::move(callback).Run(id);
}

void FetcherServiceImpl::Pause(uint64_t download_id, PauseCallback callback) {
  auto it = active_downloads_.find(download_id);
  if (it == active_downloads_.end()) {
    std::move(callback).Run(false);
    return;
  }
  it->second->state = fetcher::mojom::DownloadState::kPaused;
  auto prod_it = active_producers_.find(download_id);
  if (prod_it != active_producers_.end()) {
    prod_it->second->Pause();
  }
  for (auto& observer : observers_) {
    observer->OnDownloadUpdated(it->second.Clone());
  }
  std::move(callback).Run(true);
}

void FetcherServiceImpl::Resume(uint64_t download_id, ResumeCallback callback) {
  auto it = active_downloads_.find(download_id);
  if (it == active_downloads_.end()) {
    std::move(callback).Run(false);
    return;
  }
  it->second->state = fetcher::mojom::DownloadState::kInProgress;
  auto prod_it = active_producers_.find(download_id);
  if (prod_it != active_producers_.end()) {
    prod_it->second->Resume();
  }
  for (auto& observer : observers_) {
    observer->OnDownloadUpdated(it->second.Clone());
  }
  std::move(callback).Run(true);
}

void FetcherServiceImpl::Cancel(uint64_t download_id, CancelCallback callback) {
  auto it = active_downloads_.find(download_id);
  if (it == active_downloads_.end()) {
    std::move(callback).Run(false);
    return;
  }
  it->second->state = fetcher::mojom::DownloadState::kCancelled;
  auto prod_it = active_producers_.find(download_id);
  if (prod_it != active_producers_.end()) {
    prod_it->second->Cancel();
    active_producers_.erase(prod_it);
  }
  for (auto& observer : observers_) {
    observer->OnDownloadUpdated(it->second.Clone());
  }
  std::move(callback).Run(true);
}

void FetcherServiceImpl::Remove(uint64_t download_id, RemoveCallback callback) {
  auto it = active_downloads_.find(download_id);
  if (it == active_downloads_.end()) {
    std::move(callback).Run(false);
    return;
  }
  for (auto& observer : observers_) {
    observer->OnDownloadDestroyed(download_id);
  }
  active_downloads_.erase(it);
  active_producers_.erase(download_id);
  std::move(callback).Run(true);
}

void FetcherServiceImpl::OpenContainingFolder(uint64_t download_id,
                                              OpenContainingFolderCallback callback) {
  std::move(callback).Run(true);
}

void FetcherServiceImpl::FlushCoalescedProgress() {
  DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);
  for (const auto& [id, snapshot] : active_downloads_) {
    if (snapshot->state == fetcher::mojom::DownloadState::kInProgress) {
      for (auto& observer : observers_) {
        observer->OnDownloadUpdated(snapshot.Clone());
      }
    }
  }
}

}  // namespace codem37
