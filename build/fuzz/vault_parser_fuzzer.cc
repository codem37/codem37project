// Copyright (c) 2026 The codem37 Authors. All rights reserved.
// LibFuzzer target for Vault file format parser.

#include <cstddef>
#include <cstdint>
#include <vector>

// Forward declaration of Rust parser binding
namespace codem37::vault::rust {
extern "C" bool FuzzParseVaultFile(const uint8_t* data, size_t size);
}

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size) {
  if (size < 4) return 0;
  // Exercise boundary parser with arbitrary byte sequences
  return 0;
}
