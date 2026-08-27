// Copyright 2026 The codem37 Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "mine/fetcher/storage/download_state_store.h"

#include "base/files/file_util.h"
#include "base/json/json_reader.h"
#include "base/json/json_writer.h"
#include "base/logging.h"
#include "base/values.h"

namespace mine {
namespace storage {

base::FilePath DownloadStateStore::GetStateFilePath(const base::FilePath& target_path) {
  return target_path.AddExtension(FILE_PATH_LITERAL(".c37state"));
}

bool DownloadStateStore::Save(const base::FilePath& target_path,
                              const PersistedDownloadState& state) {
  base::Value::Dict root;
  root.Set("version", static_cast<int>(state.version));
  root.Set("url", state.url.spec());
  root.Set("destination", state.destination.AsUTF8Unsafe());
  root.Set("total_bytes", static_cast<double>(state.total_bytes));
  root.Set("etag", state.etag);
  root.Set("last_modified", state.last_modified);

  base::Value::List ranges_list;
  for (const auto& r : state.ranges) {
    base::Value::Dict range_dict;
    range_dict.Set("start", static_cast<double>(r.start));
    range_dict.Set("end", static_cast<double>(r.end));
    range_dict.Set("completed", static_cast<double>(r.completed));
    range_dict.Set("is_finished", r.is_finished);
    ranges_list.Append(std::move(range_dict));
  }
  root.Set("ranges", std::move(ranges_list));

  std::string json_output;
  if (!base::JSONWriter::Write(root, &json_output)) {
    LOG(ERROR) << "[codem37::StateStore] Failed to serialize download state.";
    return false;
  }

  base::FilePath state_path = GetStateFilePath(target_path);
  base::FilePath temp_path = state_path.AddExtension(FILE_PATH_LITERAL(".tmp"));

  // 1. Write to temporary file
  if (!base::WriteFile(temp_path, json_output)) {
    LOG(ERROR) << "[codem37::StateStore] Failed to write temp state file: "
               << temp_path.value();
    return false;
  }

  // 2. Atomically replace destination state file
  if (!base::ReplaceFile(temp_path, state_path, nullptr)) {
    LOG(ERROR) << "[codem37::StateStore] Atomic rename failed for state file: "
               << state_path.value();
    base::DeleteFile(temp_path);
    return false;
  }

  return true;
}

std::optional<PersistedDownloadState> DownloadStateStore::Load(
    const base::FilePath& target_path) {
  base::FilePath state_path = GetStateFilePath(target_path);
  if (!base::PathExists(state_path)) {
    return std::nullopt;
  }

  std::string content;
  if (!base::ReadFileToString(state_path, &content)) {
    LOG(ERROR) << "[codem37::StateStore] Failed to read state file: "
               << state_path.value();
    return std::nullopt;
  }

  auto parsed = base::JSONReader::Read(content);
  if (!parsed || !parsed->is_dict()) {
    LOG(ERROR) << "[codem37::StateStore] Corrupted JSON in state file: "
               << state_path.value();
    return std::nullopt;
  }

  const base::Value::Dict& dict = parsed->GetDict();
  PersistedDownloadState state;
  state.version = dict.FindInt("version").value_or(1);
  state.url = GURL(dict.FindString("url") ? *dict.FindString("url") : "");
  state.destination = base::FilePath::FromUTF8Unsafe(
      dict.FindString("destination") ? *dict.FindString("destination") : "");
  state.total_bytes = static_cast<uint64_t>(
      dict.FindDouble("total_bytes").value_or(0.0));
  state.etag = dict.FindString("etag") ? *dict.FindString("etag") : "";
  state.last_modified = dict.FindString("last_modified")
                            ? *dict.FindString("last_modified")
                            : "";

  const base::Value::List* ranges_list = dict.FindList("ranges");
  if (ranges_list) {
    for (const auto& item : *ranges_list) {
      if (item.is_dict()) {
        const auto& r_dict = item.GetDict();
        PersistedRangeState range;
        range.start = static_cast<uint64_t>(
            r_dict.FindDouble("start").value_or(0.0));
        range.end = static_cast<uint64_t>(
            r_dict.FindDouble("end").value_or(0.0));
        range.completed = static_cast<uint64_t>(
            r_dict.FindDouble("completed").value_or(0.0));
        range.is_finished = r_dict.FindBool("is_finished").value_or(false);
        state.ranges.push_back(range);
      }
    }
  }

  return state;
}

bool DownloadStateStore::Remove(const base::FilePath& target_path) {
  base::FilePath state_path = GetStateFilePath(target_path);
  if (base::PathExists(state_path)) {
    return base::DeleteFile(state_path);
  }
  return true;
}

}  // namespace storage
}  // namespace mine
