// Copyright 2026 The codem37 Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef MINE_FETCHER_STORAGE_SPARSE_FILE_ALLOCATOR_WIN_H_
#define MINE_FETCHER_STORAGE_SPARSE_FILE_ALLOCATOR_WIN_H_

#include "build/build_config.h"

#if BUILDFLAG(IS_WIN)
#include <windows.h>
#include <cstdint>
#include "base/containers/span.h"

namespace mine {
namespace storage {

class SparseFileAllocatorWin {
 public:
  // Marks the file handle as a Windows sparse file (FSCTL_SET_SPARSE)
  // and extends the valid logical length to `total_size`.
  static bool Allocate(HANDLE file_handle, uint64_t total_size);

  // Writes `data` directly to the specified `offset` in the preallocated file.
  // Thread-safe and lock-free across non-overlapping byte ranges.
  static bool WriteAt(HANDLE file_handle,
                      uint64_t offset,
                      base::span<const uint8_t> data);
};

}  // namespace storage
}  // namespace mine

#endif  // BUILDFLAG(IS_WIN)
#endif  // MINE_FETCHER_STORAGE_SPARSE_FILE_ALLOCATOR_WIN_H_
