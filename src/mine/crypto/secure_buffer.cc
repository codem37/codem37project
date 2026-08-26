// Copyright (c) 2026 The codem37 Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "src/mine/crypto/secure_buffer.h"

#include <cstring>
#include <utility>

#if defined(_WIN32)
#include <windows.h>
#else
#include <openssl/crypto.h>
#endif

namespace codem37 {

namespace {

void SecureZero(void* ptr, size_t len) {
  if (!ptr || len == 0) return;
#if defined(_WIN32)
  SecureZeroMemory(ptr, len);
#elif defined(OPENSSL_CLEANSE)
  OPENSSL_cleanse(ptr, len);
#else
  volatile uint8_t* p = static_cast<volatile uint8_t*>(ptr);
  while (len--) *p++ = 0;
#endif
}

}  // namespace

SecureBuffer::SecureBuffer() = default;

SecureBuffer::SecureBuffer(size_t size) : buffer_(size, 0) {}

SecureBuffer::SecureBuffer(const uint8_t* data, size_t size) {
  if (data && size > 0) {
    buffer_.assign(data, data + size);
  }
}

SecureBuffer::SecureBuffer(const std::string& str) {
  if (!str.empty()) {
    buffer_.assign(reinterpret_cast<const uint8_t*>(str.data()),
                   reinterpret_cast<const uint8_t*>(str.data() + str.size()));
  }
}

SecureBuffer::~SecureBuffer() {
  ClearAndZeroize();
}

SecureBuffer::SecureBuffer(SecureBuffer&& other) noexcept
    : buffer_(std::move(other.buffer_)) {}

SecureBuffer& SecureBuffer::operator=(SecureBuffer&& other) noexcept {
  if (this != &other) {
    ClearAndZeroize();
    buffer_ = std::move(other.buffer_);
  }
  return *this;
}

void SecureBuffer::Resize(size_t new_size) {
  buffer_.resize(new_size, 0);
}

void SecureBuffer::ClearAndZeroize() {
  if (!buffer_.empty()) {
    SecureZero(buffer_.data(), buffer_.size());
    buffer_.clear();
  }
}

std::string SecureBuffer::AsString() const {
  return std::string(reinterpret_cast<const char*>(buffer_.data()), buffer_.size());
}

}  // namespace codem37
