// Copyright 2026 The codem37 Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "mine/fetcher/engine/segment_worker.h"

#include <utility>
#include "base/logging.h"
#include "base/strings/stringprintf.h"
#include "mine/fetcher/engine/segment_scheduler.h"
#include "mine/fetcher/storage/download_file.h"
#include "net/http/http_response_headers.h"
#include "net/traffic_annotation/network_traffic_annotation.h"
#include "services/network/public/cpp/resource_request.h"
#include "services/network/public/cpp/shared_url_loader_factory.h"

namespace mine {
namespace engine {

namespace {

const net::NetworkTrafficAnnotationTag kSegmentedFetchAnnotation =
    net::DefineNetworkTrafficAnnotation("codem37_segmented_fetch", R"(
      semantics {
        sender: "codem37 Native Parallel Download Engine"
        description:
          "Fetches a designated byte range of a resource directly to disk."
        trigger:
          "User initiated segmented download from codem37 browser or WebUI."
        data: "Range header specifying requested chunk boundaries."
        destination: WEBSITE
      }
      policy {
        cookies_allowed: YES
        setting: "This feature cannot be disabled."
      }
    )");

}  // namespace

SegmentWorker::SegmentWorker(
    int worker_id,
    scoped_refptr<network::SharedURLLoaderFactory> url_loader_factory,
    storage::DownloadFile* target_file,
    ProgressCallback progress_callback,
    CompletionCallback completion_callback)
    : worker_id_(worker_id),
      url_loader_factory_(std::move(url_loader_factory)),
      target_file_(target_file),
      progress_callback_(std::move(progress_callback)),
      completion_callback_(std::move(completion_callback)) {}

SegmentWorker::~SegmentWorker() {
  Cancel();
}

void SegmentWorker::StartRange(
    const GURL& url,
    const DownloadRange& range,
    const std::optional<std::string>& expected_etag) {
  Cancel();

  is_busy_ = true;
  current_range_id_ = range.id;
  current_offset_ = range.start + range.completed;
  expected_end_ = range.end;
  bytes_transferred_ = 0;
  expected_etag_ = expected_etag;

  if (current_offset_ > expected_end_) {
    // Already fully completed
    is_busy_ = false;
    if (completion_callback_) {
      std::move(completion_callback_).Run(worker_id_, current_range_id_, true, 0);
    }
    return;
  }

  auto resource_request = std::make_unique<network::ResourceRequest>();
  resource_request->url = url;
  resource_request->method = "GET";
  resource_request->credentials_mode = network::mojom::CredentialsMode::kInclude;

  // Add HTTP Range header: "bytes=start-end"
  std::string range_header =
      base::StringPrintf("bytes=%" PRIu64 "-%" PRIu64, current_offset_, expected_end_);
  resource_request->headers.SetHeader(net::HttpRequestHeaders::kRange, range_header);

  url_loader_ = network::SimpleURLLoader::Create(std::move(resource_request),
                                                 kSegmentedFetchAnnotation);

  url_loader_->SetOnResponseStartedCallback(base::BindOnce(
      &SegmentWorker::OnResponseStarted, weak_factory_.GetWeakPtr()));

  // Stream data pipe directly into memory buffer / disk
  url_loader_->DownloadAsStream(url_loader_factory_.get(), this);
}

void SegmentWorker::OnResponseStarted(
    const GURL& final_url,
    const network::mojom::URLResponseHead& response_head) {
  if (!response_head.headers) {
    Cancel();
    if (completion_callback_) {
      std::move(completion_callback_).Run(worker_id_, current_range_id_, false, 0);
    }
    return;
  }

  int response_code = response_head.headers->response_code();
  if (response_code != 206 && response_code != 200) {
    LOG(ERROR) << "[codem37::SegmentWorker " << worker_id_
               << "] Server rejected range request with HTTP " << response_code;
    Cancel();
    if (completion_callback_) {
      std::move(completion_callback_).Run(worker_id_, current_range_id_, false, 0);
    }
    return;
  }

  // Verify ETag if expected
  if (expected_etag_.has_value()) {
    std::string etag;
    if (response_head.headers->EnumerateHeader(nullptr, "ETag", &etag)) {
      if (etag != *expected_etag_) {
        LOG(ERROR) << "[codem37::SegmentWorker " << worker_id_
                   << "] ETag mismatch! Expected: " << *expected_etag_
                   << " vs Received: " << etag;
        Cancel();
        if (completion_callback_) {
          std::move(completion_callback_).Run(worker_id_, current_range_id_, false, 0);
        }
        return;
      }
    }
  }
}

void SegmentWorker::OnDataAvailable(base::span<const uint8_t> data) {
  if (data.empty() || !target_file_) {
    return;
  }

  // Phase 0 / 5 Invariant: Write directly into preallocated target file at designated offset
  bool write_ok = target_file_->WriteAt(current_offset_, data);
  if (!write_ok) {
    LOG(ERROR) << "[codem37::SegmentWorker " << worker_id_
               << "] Direct offset disk write failed at: " << current_offset_;
    Cancel();
    if (completion_callback_) {
      std::move(completion_callback_).Run(worker_id_, current_range_id_, false, bytes_transferred_);
    }
    return;
  }

  current_offset_ += data.size();
  bytes_transferred_ += data.size();

  if (progress_callback_) {
    progress_callback_.Run(current_range_id_, data.size());
  }
}

void SegmentWorker::OnDataComplete() {
  is_busy_ = false;
  bool success = (url_loader_ && url_loader_->NetError() == net::OK);
  if (completion_callback_) {
    std::move(completion_callback_).Run(worker_id_, current_range_id_, success, bytes_transferred_);
  }
}

void SegmentWorker::Cancel() {
  is_busy_ = false;
  url_loader_.reset();
  pipe_drainer_.reset();
}

}  // namespace engine
}  // namespace mine
