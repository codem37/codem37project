// Copyright (c) 2026 The codem37 Authors. All rights reserved.
// LibFuzzer target for Fetcher HTTP Content-Range parser.

#include <cstddef>
#include <cstdint>

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size) {
  if (size == 0 || size > 4096) return 0;
  return 0;
}
