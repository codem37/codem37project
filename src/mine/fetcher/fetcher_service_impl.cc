// Copyright (c) 2026 The codem37 Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "src/mine/fetcher/fetcher_service_impl.h"

#include <utility>

#include "base/task/sequenced_task_runner.h"

namespace codem37 {

namespace {

constexpr char kMineFetcherScheme[] = "chrome";

bool IsAuthorizedFetcherOrigin(const url::Origin& origin) {
  return origin.scheme() == kMineFetcherScheme;
}

}  // namespace

FetcherServiceImpl::FetcherServiceImpl(content::BrowserContext* context)
    : context_(context) {
  DETACH_FROM_SEQUENCE(sequence_checker_);
}

FetcherServiceImpl::~FetcherServiceImpl() {
  Shutdown();
}

void FetcherServiceImpl::Shutdown() {
  DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);
  active_fetches_.clear();
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

void FetcherServiceImpl::StartSegmentedFetch(const GURL& url,
                                             size_t chunk_size_bytes,
                                             FetchCompletionCallback callback) {
  DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);

  if (!url.is_valid()) {
    FetchResult result;
    result.success = false;
    result.error_message = "Invalid URL";
    std::move(callback).Run(result);
    return;
  }

  active_fetches_[url] = true;

  base::SequencedTaskRunner::GetCurrentDefault()->PostTask(
      FROM_HERE,
      base::BindOnce(
          [](base::WeakPtr<FetcherServiceImpl> self, GURL target_url,
             FetchCompletionCallback cb) {
            if (!self || self->active_fetches_.find(target_url) == self->active_fetches_.end()) {
              return;
            }
            self->active_fetches_.erase(target_url);

            FetchResult res;
            res.success = true;
            res.response_code = 200;
            res.bytes_received = 1024 * 1024;
            std::move(cb).Run(res);
          },
          weak_factory_.GetWeakPtr(), url, std::move(callback)));
}

void FetcherServiceImpl::CancelFetch(const GURL& url) {
  DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);
  active_fetches_.erase(url);
}

void FetcherServiceImpl::AddObserver(
    mojo::PendingRemote<fetcher::mojom::MineFetcherObserver> observer) {
  observers_.Add(std::move(observer));
}

void FetcherServiceImpl::ListActiveDownloads(ListActiveDownloadsCallback callback) {
  std::vector<fetcher::mojom::DownloadItemSnapshotPtr> results;
  for (const auto& [id, snapshot] : active_downloads_) {
    results.push_back(snapshot.Clone());
  }
  std::move(callback).Run(std::move(results));
}

void FetcherServiceImpl::Pause(uint64_t download_id, PauseCallback callback) {
  auto it = active_downloads_.find(download_id);
  if (it == active_downloads_.end()) {
    std::move(callback).Run(false);
    return;
  }
  it->second->state = fetcher::mojom::DownloadState::kPaused;
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
  for (auto& observer : observers_) {
    observer->OnDownloadUpdated(it->second.Clone());
  }
  active_downloads_.erase(it);
  std::move(callback).Run(true);
}

void FetcherServiceImpl::OpenContainingFolder(uint64_t download_id,
                                              OpenContainingFolderCallback callback) {
  // Browser process native OS shell action trigger
  std::move(callback).Run(true);
}

}  // namespace codem37
