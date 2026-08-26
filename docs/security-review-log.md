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

### Entry SR-2026-003: Phase 4 MineFetcher Mojo Interface & Scheme Privilege Model
- **Date:** 2026-08-26
- **Reviewer:** codem37 Lead Developer
- **Component:** `src/mine/fetcher/mojom/fetcher.mojom`, `src/mine/protocols/`
- **Type:** Mojo Interface Approval & Custom Scheme Privilege Model
- **Decision:** **APPROVED**
- **Justification & Invariants:**
  - `mojom::MineFetcher` exposes only opaque numeric download IDs (`uint64 download_id`); renderers cannot directly manipulate C++ `DownloadItem` pointers or raw filesystem locations.
  - Custom scheme `codem37://` is restricted strictly to internal browser UI asset and WebUI routing; cross-origin web access and arbitrary script embedding are denied by default.

### Entry SR-2026-004: Phase 5 Segmented Fetch & DownloadManager Authority
- **Date:** 2026-08-26
- **Reviewer:** codem37 Lead Developer
- **Component:** `src/mine/fetcher/segmented_fetch_producer`, `mojom::MineFetcher`
- **Type:** Architectural Authority & Reassembly Security Review
- **Decision:** **APPROVED**
- **Justification & Invariants:**
  - `DownloadManager` remains the single authoritative external record of all download states.
  - `SegmentedFetchProducer` writes directly to preallocated sparse destination file offsets using non-overlapping ranges; concurrent writes require no mutex locks.
  - Resource consistency is strictly verified via `ETag` and `Content-Range`; mismatching responses are rejected before writing to disk.


