// Copyright 2026 The codem37 Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef MINE_FETCHER_STORAGE_SPARSE_FILE_ALLOCATOR_POSIX_H_
#define MINE_FETCHER_STORAGE_SPARSE_FILE_ALLOCATOR_POSIX_H_

#include "build/build_config.h"

#if BUILDFLAG(IS_POSIX)
#include <cstdint>
#include "base/containers/span.h"

namespace mine {
namespace storage {

class SparseFileAllocatorPosix {
 public:
  static bool Allocate(int fd, uint64_t total_size);
  static bool WriteAt(int fd, uint64_t offset, base::span<const uint8_t> data);
};

}  // namespace storage
}  // namespace mine

#endif  // BUILDFLAG(IS_POSIX)
#endif  // MINE_FETCHER_STORAGE_SPARSE_FILE_ALLOCATOR_POSIX_H_
