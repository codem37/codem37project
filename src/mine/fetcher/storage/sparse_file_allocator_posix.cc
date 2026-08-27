// Copyright 2026 The codem37 Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "mine/fetcher/storage/sparse_file_allocator_posix.h"

#if BUILDFLAG(IS_POSIX)
#include <fcntl.h>
#include <unistd.h>
#include "base/logging.h"

namespace mine {
namespace storage {

bool SparseFileAllocatorPosix::Allocate(int fd, uint64_t total_size) {
  if (fd < 0) {
    return false;
  }
#if defined(FALLOC_FL_KEEP_SIZE)
  if (fallocate(fd, 0, 0, static_cast<off_t>(total_size)) == 0) {
    return true;
  }
#endif
  return ftruncate(fd, static_cast<off_t>(total_size)) == 0;
}

bool SparseFileAllocatorPosix::WriteAt(int fd,
                                       uint64_t offset,
                                       base::span<const uint8_t> data) {
  if (fd < 0 || data.empty()) {
    return false;
  }
  ssize_t written = pwrite(fd, data.data(), data.size(), static_cast<off_t>(offset));
  return written == static_cast<ssize_t>(data.size());
}

}  // namespace storage
}  // namespace mine

#endif  // BUILDFLAG(IS_POSIX)
