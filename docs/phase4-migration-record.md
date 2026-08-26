# codem37 — Phase 4: Native Chromium Migration Record

> **Governing Boundary:**
> **`chrome-ui` is a control plane, not a data plane.** WebUI renderers send commands and receive narrowly-scoped status/metadata over Mojo. All filesystem, network-socket, cryptographic, and credential operations happen exclusively in the browser process or its owned services (`vault`, `shield`, `fetcher`, `protocol`). Node/Electron is completely removed.

---

## 1. Migration Architecture Summary

All legacy Electron and Node.js APIs have been migrated to native Chromium browser-process constructs:

| Component | Legacy Electron Implementation | Native Chromium Architecture | Mojo Interface |
| :--- | :--- | :--- | :--- |
| **`fetcher`** | `will-download` override + Node range chunker | `content::DownloadManager` observer | [`codem37.fetcher.mojom.MineFetcher`](file:///c:/Users/manoh/codem37/src/mine/fetcher/mojom/fetcher.mojom) |
| **`protocol`** | `protocol.registerSchemesAsPrivileged` | `content::ProtocolHandler` / `URLLoaderFactory` | `codem37://` Scheme Handler |
| **`vault`** | Node.js `crypto` & plaintext JSON file | SQLite + Rust AEAD envelope + `KeyedService` | [`codem37.vault.mojom.VaultService`](file:///c:/Users/manoh/codem37/src/mine/vault/mojom/vault.mojom) |
| **`shield`** | `session.webRequest` Node scripts | Browser-process network interceptor + Rust | [`codem37.shield.mojom.ShieldService`](file:///c:/Users/manoh/codem37/src/mine/shield/mojom/shield.mojom) |

---

## 2. Data Movement & Trust Boundaries

- **Opaque Handles**: Renderers manipulate downloads using opaque `uint64` / `string` IDs (`download_id`), never raw C++ `DownloadItem*` pointers.
- **No Large Binary Buffers over IPC**: Download payload data flows directly: `Network Service` -> `Disk`. Mojo carries only progress percentages, byte counters, and state transitions (`kInProgress`, `kPaused`, `kCompleted`, `kCancelled`).
- **Filesystem Isolation**: File paths are handled exclusively in the browser process. WebUI receives sanitized display names and triggers native OS "Show in Folder" actions without holding raw filesystem handles.

---

## 3. Existing-Data Migration & Safety Protocol

1. **Pre-Migration Backup**: Before any migration routine executes on existing profile data, the database is backed up to `<profile_dir>/vault.db.bak`.
2. **Transactional Execution**: Migration occurs in a temporary file and is atomically swapped into place upon successful validation.
3. **Fail-Safe Rollback**: In case of parsing errors or checksum mismatches, the original file is preserved and the backup remains untouched.
