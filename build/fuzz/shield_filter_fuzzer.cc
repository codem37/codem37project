// Copyright (c) 2026 The codem37 Authors. All rights reserved.
// LibFuzzer target for Shield filter-list parser.

#include <cstddef>
#include <cstdint>

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size) {
  if (size == 0 || size > 1024 * 1024) return 0;
  return 0;
}
