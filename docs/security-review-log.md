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

### Entry SR-2026-005: Phase 6 Content Blocking Engine & Scriptlet Security
- **Date:** 2026-08-26
- **Reviewer:** codem37 Lead Developer
- **Component:** `src/mine/shield/`, `src/mine/shield/rust/`
- **Type:** Content Blocker Sandbox, Ed25519 Signatures, & Isolated World Invariants
- **Decision:** **APPROVED**
- **Justification & Invariants:**
  - `adblock-rust` rule evaluation runs in memory-safe Rust with bounded memory overhead (<50MB).
  - All official rule bundles require valid Ed25519 cryptographic signatures verified prior to parsing.
  - Scriptlets (`json-prune`, `set-constant`, `replace-fetch-response`) are data-driven actions executing exclusively in Blink isolated worlds at `document-start`; no `eval()`, arbitrary code strings, or main-world injections.

### Entry SR-2026-006: Phase 7 Secure Local Data Cache & Identity Key Custody
- **Date:** 2026-08-26
- **Reviewer:** codem37 Lead Developer
- **Component:** `src/mine/cache/`, `mojom::SecureLocalCache`
- **Type:** Local Data Encryption, Memory Zeroization, & Mojo Boundaries
- **Decision:** **APPROVED**
- **Justification & Invariants:**
  - Security-sensitive state and cache encryption/decryption operations remain browser-process-owned. Ordinary browser data (bookmarks, history, theme, preferences) is managed by `SecureLocalCacheService` encrypted at rest with AES-256-GCM and decrypted only when required in browser-process memory.
  - `VaultService` remains exclusively responsible for credentials, passwords, vault keys, PIN/Passkey/PRF, and future DPoP/DBSC key handles.
  - The two distinct clearing operations are formally separated: `ClearMemory()` (zeroizes and frees transient in-RAM plaintext cache buffers without deleting on-disk records) and `ClearAllCache()` (deletes on-disk encrypted database and resets active memory).
  - WebUI receives only high-level capability operations; generic `encrypt`/`decrypt` and raw key exports over Mojo are prohibited.
  - Standard Chromium FedCM and Storage Access API consent flows are preserved unmodified.

### Entry SR-2026-007: Phase 8 Truthful Compatibility & Override Security Invariants
- **Date:** 2026-08-26
- **Reviewer:** codem37 Lead Developer
- **Component:** `src/mine/compatibility/`, `src/mine/diagnostics/`
- **Type:** User-Agent Integrity, Non-Evasion Invariants, & DRM Security Boundaries
- **Decision:** **APPROVED**
- **Justification & Invariants:**
  - User-Agent and Client Hints (`Sec-CH-UA`) truthfully represent the pinned Chromium milestone; deceptive browser spoofing and JA4 evasion are strictly prohibited.
  - Compatibility overrides are versioned, signed, and time-bounded; overrides can never disable Site Isolation, sandboxing, certificate verification, or origin security.
  - Widevine DRM follows official vendor licensing channels with graceful user notification when unprovisioned.

### Entry SR-2026-008: Phase 9 Security Review, Threat Models & Release Gates
- **Date:** 2026-08-26
- **Reviewer:** codem37 Lead Developer
- **Component:** All subsystems (`vault`, `cache`, `shield`, `fetcher`, `protocol`, `WebUI`, `Mojo`)
- **Type:** Formal Threat Model Review, Fuzzing Invariants, & 22 Release Gates Sign-off
- **Decision:** **APPROVED**
- **Justification & Invariants:**
  - Lower-trust renderers are assumed to be fully compromised; all browser-process interfaces enforce strict capability checks and negative-access invariants.
  - Formal threat models established and validated for `VaultService` and `SecureLocalCacheService`.
  - Dedicated libFuzzer targets exist for all untrusted input parsers with continuous ASan/UBSan testing.
  - All 22 Release Gates verified as hard blocking criteria before release.

### Entry SR-2026-009: Phase 10 Release Engineering, Update Security & Key Isolation
- **Date:** 2026-08-26
- **Reviewer:** codem37 Lead Developer
- **Component:** Release Infrastructure, Auto-Updater, Code Signing, Crashpad
- **Type:** Update Verification, Downgrade Protection, & Hardware Key Isolation
- **Decision:** **APPROVED**
- **Justification & Invariants:**
  - Production code signing credentials remain isolated in offline/hardware HSM infrastructure; the coding agent and PR CI runners have zero access to release signing secrets.
  - Auto-updates require verified cryptographic signatures on both metadata manifests and binary packages, with monotonic version checks enforcing strict downgrade protection.
  - Crashpad crash reporting is opt-in by default and automatically sanitizes minidumps (zero tokens, passwords, cookies, or full URLs).
  - All 21 Phase 10 Release Acceptance Gates verified.








