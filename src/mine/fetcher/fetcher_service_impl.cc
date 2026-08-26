// Copyright (c) 2026 The codem37 Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "src/mine/fetcher/fetcher_service_impl.h"

#include <utility>

#include "base/task/sequenced_task_runner.h"

namespace codem37 {

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

  // Simulate background chunked download completion
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

}  // namespace codem37
