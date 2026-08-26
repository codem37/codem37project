// Copyright (c) 2026 The codem37 Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CODEM37_SRC_MINE_FETCHER_FETCHER_SERVICE_IMPL_H_
#define CODEM37_SRC_MINE_FETCHER_FETCHER_SERVICE_IMPL_H_

#include <map>
#include <memory>
#include <string>
#include <vector>

#include "base/files/file_path.h"
#include "base/memory/raw_ptr.h"
#include "base/memory/weak_ptr.h"
#include "base/sequence_checker.h"
#include "base/timer/timer.h"
#include "mojo/public/cpp/bindings/receiver_set.h"
#include "mojo/public/cpp/bindings/remote_set.h"
#include "src/mine/fetcher/fetcher_service.h"
#include "src/mine/fetcher/segmented_fetch_producer.h"

namespace content {
class BrowserContext;
}

namespace codem37 {

class FetcherServiceImpl : public FetcherService {
 public:
  explicit FetcherServiceImpl(content::BrowserContext* context);
  ~FetcherServiceImpl() override;

  FetcherServiceImpl(const FetcherServiceImpl&) = delete;
  FetcherServiceImpl& operator=(const FetcherServiceImpl&) = delete;

  // FetcherService implementation:
  void BindReceiver(
      mojo::PendingReceiver<fetcher::mojom::MineFetcher> receiver,
      const url::Origin& caller_origin) override;

  void StartSegmentedFetch(const GURL& url,
                           const base::FilePath& destination_path,
                           uint64_t total_bytes,
                           const std::string& etag,
                           FetchCompletionCallback callback) override;
  void CancelFetch(const GURL& url) override;

  // mojom::MineFetcher implementation:
  void AddObserver(
      mojo::PendingRemote<fetcher::mojom::MineFetcherObserver> observer) override;
  void GetSnapshot(GetSnapshotCallback callback) override;
  void StartFromUrl(const GURL& url,
                    const std::optional<std::string>& suggested_filename,
                    StartFromUrlCallback callback) override;
  void Pause(uint64_t download_id, PauseCallback callback) override;
  void Resume(uint64_t download_id, ResumeCallback callback) override;
  void Cancel(uint64_t download_id, CancelCallback callback) override;
  void Remove(uint64_t download_id, RemoveCallback callback) override;
  void OpenContainingFolder(uint64_t download_id,
                            OpenContainingFolderCallback callback) override;

  // KeyedService lifecycle:
  void Shutdown() override;

 private:
  void FlushCoalescedProgress();

  SEQUENCE_CHECKER(sequence_checker_);

  raw_ptr<content::BrowserContext> context_;
  std::map<uint64_t, fetcher::mojom::DownloadItemSnapshotPtr> active_downloads_;
  std::map<uint64_t, std::unique_ptr<SegmentedFetchProducer>> active_producers_;

  mojo::ReceiverSet<fetcher::mojom::MineFetcher, url::Origin> receivers_;
  mojo::RemoteSet<fetcher::mojom::MineFetcherObserver> observers_;

  base::RepeatingTimer progress_timer_;
  uint64_t next_download_id_ = 1;

  base::WeakPtrFactory<FetcherServiceImpl> weak_factory_{this};
};

}  // namespace codem37

#endif  // CODEM37_SRC_MINE_FETCHER_FETCHER_SERVICE_IMPL_H_
