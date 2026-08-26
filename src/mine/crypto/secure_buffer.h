// Copyright (c) 2026 The codem37 Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CODEM37_SRC_MINE_CRYPTO_SECURE_BUFFER_H_
#define CODEM37_SRC_MINE_CRYPTO_SECURE_BUFFER_H_

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

namespace codem37 {

// RAII container for cryptographic keys and plaintext credentials.
// Guarantees zeroization of underlying memory upon destruction.
class SecureBuffer {
 public:
  SecureBuffer();
  explicit SecureBuffer(size_t size);
  SecureBuffer(const uint8_t* data, size_t size);
  explicit SecureBuffer(const std::string& str);
  ~SecureBuffer();

  SecureBuffer(const SecureBuffer&) = delete;
  SecureBuffer& operator=(const SecureBuffer&) = delete;

  SecureBuffer(SecureBuffer&& other) noexcept;
  SecureBuffer& operator=(SecureBuffer&& other) noexcept;

  void Resize(size_t new_size);
  void ClearAndZeroize();

  uint8_t* data() { return buffer_.data(); }
  const uint8_t* data() const { return buffer_.data(); }
  size_t size() const { return buffer_.size(); }
  bool empty() const { return buffer_.empty(); }

  std::string AsString() const;

 private:
  std::vector<uint8_t> buffer_;
};

}  // namespace codem37

#endif  // CODEM37_SRC_MINE_CRYPTO_SECURE_BUFFER_H_
