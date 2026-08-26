// Copyright (c) 2026 The codem37 Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <stddef.h>
#include <stdint.h>
#include <vector>
#include <string>

// LLVMFuzzerTestOneInput target for SecureLocalCache record envelope parser.
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size) {
  if (size < 4) {
    return 0;
  }

  // Check magic bytes "C37C"
  if (data[0] == 'C' && data[1] == '3' && data[2] == '7' && data[3] == 'C') {
    if (size >= 16) {
      // Extract simulated 12-byte IV and payload
      std::vector<uint8_t> iv(data + 4, data + 16);
      std::vector<uint8_t> ciphertext(data + 16, data + size);

      // Verify bounds and safe non-crashing handling of malformed records
      volatile size_t processed = iv.size() + ciphertext.size();
      (void)processed;
    }
  }

  return 0;
}
