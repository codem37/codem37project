// Copyright 2026 The codem37 Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef MINE_FETCHER_ENGINE_SEGMENT_WORKER_H_
#define MINE_FETCHER_ENGINE_SEGMENT_WORKER_H_

#include <cstdint>
#include <memory>
#include <optional>
#include <string>
#include "base/functional/callback.h"
#include "base/memory/scoped_refptr.h"
#include "base/memory/weak_ptr.h"
#include "mojo/public/cpp/system/data_pipe_drainer.h"
#include "services/network/public/cpp/simple_url_loader.h"
#include "url/gurl.h"

namespace network {
class SharedURLLoaderFactory;
}

namespace mine {
namespace storage {
class DownloadFile;
}

namespace engine {

struct DownloadRange;

class SegmentWorker : public mojo::DataPipeDrainer::Client {
 public:
  using CompletionCallback = base::OnceCallback<void(int worker_id,
                                                      uint32_t range_id,
                                                      bool success,
                                                      uint64_t bytes_transferred)>;
  using ProgressCallback = base::RepeatingCallback<void(uint32_t range_id,
                                                        uint64_t bytes_written)>;

  SegmentWorker(int worker_id,
                scoped_refptr<network::SharedURLLoaderFactory> url_loader_factory,
                storage::DownloadFile* target_file,
                ProgressCallback progress_callback,
                CompletionCallback completion_callback);
  ~SegmentWorker() override;

  // Starts fetching the assigned work range
  void StartRange(const GURL& url,
                  const DownloadRange& range,
                  const std::optional<std::string>& expected_etag);

  // Cancels any in-flight transfer
  void Cancel();

  int worker_id() const { return worker_id_; }
  bool is_busy() const { return is_busy_; }

  // mojo::DataPipeDrainer::Client implementation:
  void OnDataAvailable(base::span<const uint8_t> data) override;
  void OnDataComplete() override;

 private:
  void OnResponseStarted(const GURL& final_url,
                         const network::mojom::URLResponseHead& response_head);

  int worker_id_;
  scoped_refptr<network::SharedURLLoaderFactory> url_loader_factory_;
  storage::DownloadFile* target_file_; // Weak ref owned by SegmentedFetchProducer
  ProgressCallback progress_callback_;
  CompletionCallback completion_callback_;

  bool is_busy_ = false;
  uint32_t current_range_id_ = 0;
  uint64_t current_offset_ = 0;
  uint64_t expected_end_ = 0;
  uint64_t bytes_transferred_ = 0;
  std::optional<std::string> expected_etag_;

  std::unique_ptr<network::SimpleURLLoader> url_loader_;
  std::unique_ptr<mojo::DataPipeDrainer> pipe_drainer_;
  base::WeakPtrFactory<SegmentWorker> weak_factory_{this};
};

}  // namespace engine
}  // namespace mine

#endif  // MINE_FETCHER_ENGINE_SEGMENT_WORKER_H_
