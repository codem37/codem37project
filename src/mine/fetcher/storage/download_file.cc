// Copyright 2026 The codem37 Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "mine/fetcher/storage/download_file.h"

#include "base/files/file_util.h"
#include "base/logging.h"

#if BUILDFLAG(IS_WIN)
#include "mine/fetcher/storage/sparse_file_allocator_win.h"
#else
#include <fcntl.h>
#include <unistd.h>
#include "mine/fetcher/storage/sparse_file_allocator_posix.h"
#endif

namespace mine {
namespace storage {

DownloadFile::DownloadFile(const base::FilePath& path, uint64_t total_size)
    : target_path_(path), total_size_(total_size) {}

DownloadFile::~DownloadFile() {
  Close();
}

std::unique_ptr<DownloadFile> DownloadFile::Create(
    const base::FilePath& target_path,
    uint64_t total_size) {
  // Ensure parent directory exists
  base::FilePath parent = target_path.DirName();
  if (!base::DirectoryExists(parent)) {
    base::CreateDirectory(parent);
  }

  auto download_file = std::unique_ptr<DownloadFile>(new DownloadFile(target_path, total_size));

#if BUILDFLAG(IS_WIN)
  HANDLE handle = CreateFileW(
      target_path.value().c_str(),
      GENERIC_READ | GENERIC_WRITE,
      FILE_SHARE_READ | FILE_SHARE_WRITE,
      nullptr,
      OPEN_ALWAYS,
      FILE_ATTRIBUTE_NORMAL | FILE_FLAG_RANDOM_ACCESS,
      nullptr);

  if (handle == INVALID_HANDLE_VALUE) {
    LOG(ERROR) << "[codem37::Storage] Failed to open download file: "
               << target_path.value() << " (ErrorCode: " << GetLastError() << ")";
    return nullptr;
  }

  download_file->file_handle_ = handle;

  if (total_size > 0) {
    if (!SparseFileAllocatorWin::Allocate(handle, total_size)) {
      LOG(WARNING) << "[codem37::Storage] Sparse allocation failed for: "
                   << target_path.value();
    }
  }
#else
  int fd = open(target_path.value().c_str(), O_RDWR | O_CREAT, 0644);
  if (fd < 0) {
    LOG(ERROR) << "[codem37::Storage] Failed to open download file: "
               << target_path.value();
    return nullptr;
  }
  download_file->file_descriptor_ = fd;
  if (total_size > 0) {
    SparseFileAllocatorPosix::Allocate(fd, total_size);
  }
#endif

  return download_file;
}

bool DownloadFile::WriteAt(uint64_t offset, base::span<const uint8_t> data) {
  if (data.empty()) {
    return true;
  }

#if BUILDFLAG(IS_WIN)
  if (file_handle_ == INVALID_HANDLE_VALUE) return false;
  return SparseFileAllocatorWin::WriteAt(file_handle_, offset, data);
#else
  if (file_descriptor_ < 0) return false;
  return SparseFileAllocatorPosix::WriteAt(file_descriptor_, offset, data);
#endif
}

int64_t DownloadFile::ReadAt(uint64_t offset, uint8_t* buffer, size_t length) {
  if (!buffer || length == 0) return 0;

#if BUILDFLAG(IS_WIN)
  if (file_handle_ == INVALID_HANDLE_VALUE) return -1;
  OVERLAPPED overlapped = {};
  overlapped.Offset = static_cast<DWORD>(offset & 0xFFFFFFFF);
  overlapped.OffsetHigh = static_cast<DWORD>((offset >> 32) & 0xFFFFFFFF);

  DWORD bytes_read = 0;
  if (!ReadFile(file_handle_, buffer, static_cast<DWORD>(length), &bytes_read, &overlapped)) {
    DWORD err = GetLastError();
    if (err == ERROR_HANDLE_EOF) return 0;
    return -1;
  }
  return static_cast<int64_t>(bytes_read);
#else
  if (file_descriptor_ < 0) return -1;
  ssize_t res = pread(file_descriptor_, buffer, length, static_cast<off_t>(offset));
  return static_cast<int64_t>(res);
#endif
}

bool DownloadFile::Flush() {
#if BUILDFLAG(IS_WIN)
  if (file_handle_ == INVALID_HANDLE_VALUE) return false;
  return FlushFileBuffers(file_handle_) != 0;
#else
  if (file_descriptor_ < 0) return false;
  return fsync(file_descriptor_) == 0;
#endif
}

void DownloadFile::Close() {
#if BUILDFLAG(IS_WIN)
  if (file_handle_ != INVALID_HANDLE_VALUE) {
    Flush();
    CloseHandle(file_handle_);
    file_handle_ = INVALID_HANDLE_VALUE;
  }
#else
  if (file_descriptor_ >= 0) {
    Flush();
    close(file_descriptor_);
    file_descriptor_ = -1;
  }
#endif
}

}  // namespace storage
}  // namespace mine
