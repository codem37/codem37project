// Copyright 2026 The codem37 Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef MINE_FETCHER_STORAGE_DOWNLOAD_STATE_STORE_H_
#define MINE_FETCHER_STORAGE_DOWNLOAD_STATE_STORE_H_

#include <cstdint>
#include <optional>
#include <string>
#include <vector>
#include "base/files/file_path.h"
#include "url/gurl.h"

namespace mine {
namespace storage {

struct PersistedRangeState {
  uint64_t start = 0;
  uint64_t end = 0;
  uint64_t completed = 0;
  bool is_finished = false;
};

struct PersistedDownloadState {
  uint32_t version = 1;
  GURL url;
  base::FilePath destination;
  uint64_t total_bytes = 0;
  std::string etag;
  std::string last_modified;
  std::vector<PersistedRangeState> ranges;
};

// Transactional sidecar state persistence (<destination>.c37state)
// Writes to temp file, flushes to physical media, and atomically renames.
class DownloadStateStore {
 public:
  static bool Save(const base::FilePath& target_path,
                   const PersistedDownloadState& state);

  static std::optional<PersistedDownloadState> Load(
      const base::FilePath& target_path);

  static bool Remove(const base::FilePath& target_path);

  static base::FilePath GetStateFilePath(const base::FilePath& target_path);
};

}  // namespace storage
}  // namespace mine

#endif  // MINE_FETCHER_STORAGE_DOWNLOAD_STATE_STORE_H_
