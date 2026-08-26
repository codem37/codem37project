// Copyright (c) 2026 The codem37 Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CODEM37_SRC_MINE_FETCHER_FETCHER_SERVICE_H_
#define CODEM37_SRC_MINE_FETCHER_FETCHER_SERVICE_H_

#include <string>
#include "base/files/file_path.h"
#include "base/functional/callback.h"
#include "components/keyed_service/core/keyed_service.h"
#include "mojo/public/cpp/bindings/pending_receiver.h"
#include "src/mine/fetcher/mojom/fetcher.mojom.h"
#include "url/gurl.h"
#include "url/origin.h"

namespace codem37 {

struct FetchResult {
  bool success = false;
  int response_code = 0;
  size_t bytes_received = 0;
  std::string error_message;
};

using FetchCompletionCallback = base::OnceCallback<void(FetchResult)>;

// Abstract interface for the Fetcher Service.
class FetcherService : public KeyedService, public fetcher::mojom::MineFetcher {
 public:
  ~FetcherService() override = default;

  virtual void BindReceiver(
      mojo::PendingReceiver<fetcher::mojom::MineFetcher> receiver,
      const url::Origin& caller_origin) = 0;

  virtual void StartSegmentedFetch(const GURL& url,
                                   const base::FilePath& destination_path,
                                   uint64_t total_bytes,
                                   const std::string& etag,
                                   FetchCompletionCallback callback) = 0;
  virtual void CancelFetch(const GURL& url) = 0;
};

}  // namespace codem37

#endif  // CODEM37_SRC_MINE_FETCHER_FETCHER_SERVICE_H_
