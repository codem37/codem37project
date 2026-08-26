# codem37 — Phase 3: Memory Safety Policy for Owned C++/Rust Additions

> **Governing Principle:**
> **Untrusted input gets memory-safe code by default, not memory-safe code by exception.** Any component that parses bytes originating outside the browser process (network responses, filter-list data, IPC payloads, on-disk files) is written in **Rust** unless there is a specific, documented reason it cannot be. C++ is the exception requiring explicit justification.

---

## 1. Subsystem Language Split (Rust vs C++)

| Subsystem | Component Responsibility | Language | Architectural Rationale |
| :--- | :--- | :---: | :--- |
| **`fetcher`** | HTTP response parsing, Range header calculation | **Rust** | Untrusted network input; eliminates parser buffer bugs. |
| **`fetcher`** | Mojo integration, Chromium network-service binding | **C++** | Integration shim using Chromium `base::SequenceBound`. |
| **`protocol`** | `codem37://` scheme/URL parsing & dispatch | **Rust** | Attacker-influenced input parsing in safe Rust. |
| **`protocol`** | `URLLoaderFactory` glue | **C++** | Native integration with `content::` IPC pipeline. |
| **`vault`** | On-disk DB deserializer, KDF/AEAD wrapper | **Rust** | Untrusted file/byte parsing; zeroizing crypto memory. |
| **`vault`** | `mojom::VaultService` KeyedService implementation | **C++** | Multi-profile lifecycle & WebUI Mojo bindings. |
| **`shield`** | Filter-list parsing, Aho-Corasick rule matcher | **Rust** | High-volume remote-updatable filter data parsing. |
| **`shield`** | `mojom::ShieldService` KeyedService implementation | **C++** | WebUI dashboard bindings & per-site prefs. |

---

## 2. Pointer & Memory Ownership Rules

1. **`raw_ptr<T>` / MiraclePtr Mandate**:
   - All eligible non-owning pointers to heap-allocated objects in C++ **must** use `raw_ptr<T>`.
   - Bare raw pointers (`T*`) are strictly prohibited for heap objects unless covered by an explicit, documented exception.
2. **Owning Raw Pointers Strictly Forbidden**:
   - Explicit ownership must be modeled via `std::unique_ptr<T>` (single owner) or `scoped_refptr<T>` (shared sequence-safe owner).
3. **PartitionAlloc Enforcement**:
   - All C++ heap allocations must go through PartitionAlloc. Custom `malloc` implementations are forbidden.
4. **Zeroizing Secret Buffers**:
   - Cryptographic keys (VMK, CEKs) and plaintext passwords must use `codem37::SecureBuffer` (C++) or `zeroize::ZeroizeOnDrop` (Rust) to guarantee immediate zeroing on scope exit and prevent presence in crash dumps.

---

## 3. C++/Rust FFI Boundary Rules (CXX)

- **Tooling**: Built exclusively via Chromium's in-tree `cxx` interop framework.
- **Data Transfer**:
  - Strings passed as UTF-8 slices (`&str`, `rust::Str`, `rust::String`). Null-terminated C-strings are prohibited across FFI.
  - Buffers passed as explicit-length slices (`&[u8]`, `rust::Slice<const uint8_t>`).
  - Errors represented as `Result<T, E>` / `rust::Error` discriminated unions.
- **Ownership**:
  - Explicit, single-directional ownership transfer per call.
  - Memory allocated in Rust is freed via Rust-generated destructors; memory allocated in C++ is freed by C++.
- **Minimal Surface**: Capability-oriented signatures (`parse_filter_list(bytes) -> Result<FilterSet>`), never generic dispatchers.

---

## 4. Concurrency & Lifetime

- **Sequence-Bound**: Every new object is sequence-bound by default with `SEQUENCE_CHECKER`.
- **Background Sequences**: Crypto KDF operations and segmented download parsing run on dedicated background `base::SequencedTaskRunner`s to prevent blocking the UI thread.
- **Async Lifetimes**: Asynchronous callbacks capture `base::WeakPtr<T>` to avoid use-after-free upon profile or service destruction.

---

## 5. Security Review & Static Analysis Enforcement

### Review Bar (§113)
The following modifications strictly require the developer's sign-off and an entry in [`docs/security-review-log.md`](file:///c:/Users/manoh/codem37/docs/security-review-log.md):
1. Any new `unsafe` Rust block (with mandatory `// SAFETY:` rationale).
2. Any new FFI boundary definition.
3. Any `raw_ptr` exception.
4. Any C++ implementation of a security-sensitive parser.

### CI Static Analysis
- [`tools/check_memory_safety.py`](file:///c:/Users/manoh/codem37/tools/check_memory_safety.py) runs on all PRs to catch forbidden owning raw pointers, unwrapped heap pointers, and unapproved `unsafe` blocks.
