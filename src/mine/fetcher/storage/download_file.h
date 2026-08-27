// Copyright 2026 The codem37 Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef MINE_FETCHER_STORAGE_DOWNLOAD_FILE_H_
#define MINE_FETCHER_STORAGE_DOWNLOAD_FILE_H_

#include <cstdint>
#include <memory>
#include <string>
#include "base/containers/span.h"
#include "base/files/file_path.h"
#include "build/build_config.h"

#if BUILDFLAG(IS_WIN)
#include <windows.h>
#endif

namespace mine {
namespace storage {

// Manages preallocated sparse direct-offset disk writes for segmented downloads.
class DownloadFile {
 public:
  static std::unique_ptr<DownloadFile> Create(const base::FilePath& target_path,
                                              uint64_t total_size);
  ~DownloadFile();

  // Non-copyable, non-movable
  DownloadFile(const DownloadFile&) = delete;
  DownloadFile& operator=(const DownloadFile&) = delete;

  // Writes `data` to `offset`. Lock-free across non-overlapping ranges.
  bool WriteAt(uint64_t offset, base::span<const uint8_t> data);

  // Reads `data` from `offset` (used for local media playback data source).
  int64_t ReadAt(uint64_t offset, uint8_t* buffer, size_t length);

  // Commits pending filesystem caches to disk.
  bool Flush();

  // Closes the file handle.
  void Close();

  const base::FilePath& path() const { return target_path_; }
  uint64_t total_size() const { return total_size_; }

 private:
  explicit DownloadFile(const base::FilePath& path, uint64_t total_size);

  base::FilePath target_path_;
  uint64_t total_size_;

#if BUILDFLAG(IS_WIN)
  HANDLE file_handle_ = INVALID_HANDLE_VALUE;
#else
  int file_descriptor_ = -1;
#endif
};

}  // namespace storage
}  // namespace mine

#endif  // MINE_FETCHER_STORAGE_DOWNLOAD_FILE_H_
