// Copyright (c) 2026 The codem37 Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "src/mine/fetcher/segmented_fetch_producer.h"

#include <algorithm>
#include <utility>

#include "base/files/file_util.h"
#include "base/json/json_reader.h"
#include "base/json/json_writer.h"
#include "base/task/sequenced_task_runner.h"
#include "base/values.h"

namespace codem37 {

namespace {

constexpr size_t kMaxConcurrency = 8;
constexpr uint64_t kMinChunkSize = 8 * 1024 * 1024; // 8 MB

}  // namespace

SegmentedFetchProducer::SegmentedFetchProducer(
    const GURL& url,
    const base::FilePath& destination_path,
    uint64_t total_bytes,
    const std::string& etag,
    SegmentedProgressCallback progress_cb,
    SegmentedCompleteCallback complete_cb)
    : url_(url),
      destination_path_(destination_path),
      sidecar_path_(destination_path.AddExtension(FILE_PATH_LITERAL(".c37state"))),
      total_bytes_(total_bytes),
      etag_(etag),
      progress_cb_(std::move(progress_cb)),
      complete_cb_(std::move(complete_cb)) {
  DETACH_FROM_SEQUENCE(sequence_checker_);
}

SegmentedFetchProducer::~SegmentedFetchProducer() {
  Cancel();
}

void SegmentedFetchProducer::InitializeSparseFile() {
  DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);

  destination_file_.Initialize(
      destination_path_,
      base::File::FLAG_OPEN_ALWAYS | base::File::FLAG_READ | base::File::FLAG_WRITE);

  if (destination_file_.IsValid() && total_bytes_ > 0) {
    // Set length to allocate/sparse-size
    destination_file_.SetLength(total_bytes_);
  }
}

void SegmentedFetchProducer::Start() {
  DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);

  InitializeSparseFile();
  if (!destination_file_.IsValid()) {
    if (complete_cb_) {
      std::move(complete_cb_).Run(false, "Failed to open destination file");
    }
    return;
  }

  // Plan segments
  size_t possible_chunks = total_bytes_ / std::max<uint64_t>(1, kMinChunkSize);
  size_t num_segments = std::clamp<size_t>(possible_chunks, 1, kMaxConcurrency);

  uint64_t chunk_size = total_bytes_ / num_segments;
  segments_.clear();

  for (size_t i = 0; i < num_segments; ++i) {
    SegmentRange seg;
    seg.index = i;
    seg.start_byte = i * chunk_size;
    seg.end_byte = (i == num_segments - 1) ? (total_bytes_ - 1) : (seg.start_byte + chunk_size - 1);
    seg.received_bytes = 0;
    seg.is_completed = false;
    segments_.push_back(seg);
  }

  SaveSidecarState(destination_path_, url_, total_bytes_, etag_, segments_);

  // Simulate chunk writes directly to offsets
  base::SequencedTaskRunner::GetCurrentDefault()->PostTask(
      FROM_HERE,
      base::BindOnce(
          [](base::WeakPtr<SegmentedFetchProducer> self) {
            if (!self || self->is_cancelled_ || self->is_paused_) return;
            for (auto& seg : self->segments_) {
              seg.received_bytes = (seg.end_byte - seg.start_byte + 1);
              seg.is_completed = true;
            }
            self->CheckOverallCompletion();
          },
          weak_factory_.GetWeakPtr()));
}

void SegmentedFetchProducer::Pause() {
  DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);
  is_paused_ = true;
  SaveSidecarState(destination_path_, url_, total_bytes_, etag_, segments_);
}

void SegmentedFetchProducer::Resume() {
  DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);
  is_paused_ = false;
  Start();
}

void SegmentedFetchProducer::Cancel() {
  DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);
  is_cancelled_ = true;
  if (destination_file_.IsValid()) {
    destination_file_.Close();
  }
}

void SegmentedFetchProducer::CheckOverallCompletion() {
  DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);
  if (is_cancelled_) return;

  bool all_done = true;
  uint64_t total_recv = 0;

  for (const auto& seg : segments_) {
    total_recv += seg.received_bytes;
    if (!seg.is_completed) {
      all_done = false;
    }
  }

  if (progress_cb_) {
    progress_cb_.Run(total_recv, total_bytes_);
  }

  if (all_done) {
    if (destination_file_.IsValid()) {
      destination_file_.Flush();
      destination_file_.Close();
    }
    base::DeleteFile(sidecar_path_);

    if (complete_cb_) {
      std::move(complete_cb_).Run(true, "");
    }
  }
}

// static
bool SegmentedFetchProducer::SaveSidecarState(
    const base::FilePath& destination_path,
    const GURL& url,
    uint64_t total_bytes,
    const std::string& etag,
    const std::vector<SegmentRange>& segments) {
  base::FilePath sidecar = destination_path.AddExtension(FILE_PATH_LITERAL(".c37state"));
  base::Value::Dict root;
  root.Set("url", url.spec());
  root.Set("total_bytes", static_cast<double>(total_bytes));
  root.Set("etag", etag);

  base::Value::List seg_list;
  for (const auto& s : segments) {
    base::Value::Dict d;
    d.Set("index", static_cast<int>(s.index));
    d.Set("start_byte", static_cast<double>(s.start_byte));
    d.Set("end_byte", static_cast<double>(s.end_byte));
    d.Set("received_bytes", static_cast<double>(s.received_bytes));
    d.Set("is_completed", s.is_completed);
    seg_list.Append(std::move(d));
  }
  root.Set("segments", std::move(seg_list));

  std::string json_str;
  if (base::JSONWriter::Write(root, &json_str)) {
    return base::WriteFile(sidecar, json_str);
  }
  return false;
}

// static
bool SegmentedFetchProducer::LoadSidecarState(
    const base::FilePath& destination_path,
    std::string& out_etag,
    std::vector<SegmentRange>& out_segments) {
  base::FilePath sidecar = destination_path.AddExtension(FILE_PATH_LITERAL(".c37state"));
  std::string content;
  if (!base::ReadFileToString(sidecar, &content)) {
    return false;
  }

  auto parsed = base::JSONReader::Read(content);
  if (!parsed || !parsed->is_dict()) {
    return false;
  }

  const base::Value::Dict& dict = parsed->GetDict();
  const std::string* etag_ptr = dict.FindString("etag");
  if (etag_ptr) {
    out_etag = *etag_ptr;
  }

  const base::Value::List* seg_list = dict.FindList("segments");
  if (!seg_list) return false;

  out_segments.clear();
  for (const auto& v : *seg_list) {
    if (!v.is_dict()) continue;
    const base::Value::Dict& d = v.GetDict();
    SegmentRange r;
    r.index = d.FindInt("index").value_or(0);
    r.start_byte = static_cast<uint64_t>(d.FindDouble("start_byte").value_or(0.0));
    r.end_byte = static_cast<uint64_t>(d.FindDouble("end_byte").value_or(0.0));
    r.received_bytes = static_cast<uint64_t>(d.FindDouble("received_bytes").value_or(0.0));
    r.is_completed = d.FindBool("is_completed").value_or(false);
    out_segments.push_back(r);
  }

  return true;
}

}  // namespace codem37
