# codem37 — Security Review & FFI Log

This document records formal security reviews and justifications for `unsafe` Rust blocks, FFI boundaries, `raw_ptr` exceptions, and C++-over-Rust choices.

---

## Log Entries

### Entry SR-2026-001: Initial Phase 3 Rust/C++ FFI Bridges
- **Date:** 2026-08-26
- **Reviewer:** codem37 Lead Developer
- **Component:** `src/mine/{vault,shield,fetcher,protocols}`
- **Type:** FFI Boundary & Memory Safety Initial Review
- **Decision:** **APPROVED**
- **Justification & Invariants:**
  - Rust parsers for `vault` (file format), `shield` (filter rules), `fetcher` (HTTP responses), and `protocol` (`codem37://` URLs) operate over bounded byte slices (`&[u8]`).
  - No owning raw pointers across FFI boundaries; all string and buffer boundaries use explicit length parameters (`cxx::String`, `rust::Slice`).
  - Cryptographic secrets utilize `SecureBuffer` in C++ and `ZeroizeOnDrop` in Rust to eliminate plaintext retention in heap or crash dumps.
