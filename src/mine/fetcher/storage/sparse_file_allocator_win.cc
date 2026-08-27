// Copyright 2026 The codem37 Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "mine/fetcher/storage/sparse_file_allocator_win.h"

#if BUILDFLAG(IS_WIN)
#include <winioctl.h>
#include "base/logging.h"

namespace mine {
namespace storage {

bool SparseFileAllocatorWin::Allocate(HANDLE file_handle, uint64_t total_size) {
  if (file_handle == INVALID_HANDLE_VALUE || file_handle == nullptr) {
    LOG(ERROR) << "[codem37::Storage] Invalid file handle for sparse allocation.";
    return false;
  }

  // 1. Mark file as sparse on NTFS / ReFS
  DWORD bytes_returned = 0;
  if (!DeviceIoControl(file_handle, FSCTL_SET_SPARSE, nullptr, 0, nullptr, 0,
                       &bytes_returned, nullptr)) {
    // Non-fatal if filesystem doesn't support sparse (falls back to regular allocation)
    VLOG(1) << "[codem37::Storage] FSCTL_SET_SPARSE warning (ErrorCode: "
            << GetLastError() << "), proceeding with standard allocation.";
  }

  // 2. Set logical file size using 64-bit file pointer
  LARGE_INTEGER distance;
  distance.QuadPart = static_cast<LONGLONG>(total_size);
  if (!SetFilePointerEx(file_handle, distance, nullptr, FILE_BEGIN)) {
    LOG(ERROR) << "[codem37::Storage] SetFilePointerEx failed (ErrorCode: "
               << GetLastError() << ")";
    return false;
  }

  if (!SetEndOfFile(file_handle)) {
    LOG(ERROR) << "[codem37::Storage] SetEndOfFile failed (ErrorCode: "
               << GetLastError() << ")";
    return false;
  }

  // Reset file pointer back to start
  distance.QuadPart = 0;
  SetFilePointerEx(file_handle, distance, nullptr, FILE_BEGIN);
  return true;
}

bool SparseFileAllocatorWin::WriteAt(HANDLE file_handle,
                                     uint64_t offset,
                                     base::span<const uint8_t> data) {
  if (file_handle == INVALID_HANDLE_VALUE || data.empty()) {
    return false;
  }

  OVERLAPPED overlapped = {};
  overlapped.Offset = static_cast<DWORD>(offset & 0xFFFFFFFF);
  overlapped.OffsetHigh = static_cast<DWORD>((offset >> 32) & 0xFFFFFFFF);

  DWORD bytes_written = 0;
  if (!WriteFile(file_handle, data.data(), static_cast<DWORD>(data.size()),
                 &bytes_written, &overlapped)) {
    DWORD error = GetLastError();
    if (error != ERROR_IO_PENDING) {
      LOG(ERROR) << "[codem37::Storage] WriteFile at offset " << offset
                 << " failed (ErrorCode: " << error << ")";
      return false;
    }
  }

  return bytes_written == data.size();
}

}  // namespace storage
}  // namespace mine

#endif  // BUILDFLAG(IS_WIN)
