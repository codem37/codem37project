# codem37 — Phase 4: Electron to Native Migration Audit & Dependency Map

This document establishes the pre-migration baseline audit of legacy Electron/Node architecture patterns, mappings to native Chromium subsystems, and the step-by-step removal sequence.

---

## 1. Legacy Electron Dependency Map

```text
Legacy Architecture (RETIRED)
=============================
chrome-ui (WebUI Renderer)
   │
   └── IPC Bridge (ipcRenderer.invoke / contextBridge)
          │
          ├── vault-ipc     --> Electron main: Node fs, crypto, local JSON storage
          ├── shield-ipc    --> Electron main: session.webRequest, adblock-js
          ├── fetcher-ipc   --> Electron main: DownloadItem.on('will-download'), net.request
          └── protocol-ipc  --> Electron main: protocol.registerSchemesAsPrivileged

Target Native Architecture (MIGRATED)
=====================================
chrome-ui (WebUI Control Plane)
   │
   ├── Mojo Pipes (Narrowly-scoped commands & progress snapshots)
   ▼
Browser Process (Native KeyedServices)
   ├── MineVault     (src/mine/vault/ -> C++ KeyedService + Rust crypto core)
   ├── MineShield    (src/mine/shield/ -> C++ KeyedService + Rust rule engine)
   ├── MineFetcher   (src/mine/fetcher/ -> C++ DownloadManager observer + Rust range parser)
   └── MineProtocol  (src/mine/protocols/ -> content::ProtocolHandler / URLLoaderFactory)
        │
        ├── Network Service (URLLoaderFactory)
        ├── DownloadManager (content::DownloadItem)
        └── Profile Storage (Encrypted on-disk SQLite)
```

---

## 2. Legacy IPC Channel Inventory & Replacement Mapping

| Legacy IPC Channel | Node Operations / APIs Used | Native Replacement Subsystem | Target Mojo Interface |
| :--- | :--- | :--- | :--- |
| `vault:unlock` | Node `crypto.pbkdf2Sync`, `crypto.createDecipheriv` | `VaultServiceImpl` + Rust KDF wrapper | `mojom::VaultService::Unlock` |
| `vault:get-entries` | Node `fs.readFileSync('vault.json')` | `VaultServiceImpl` (Metadata only) | `mojom::VaultService::ListEntriesMetadata` |
| `vault:save-entry` | Node `fs.writeFileSync('vault.json')` | `VaultServiceImpl` (AES-256-GCM envelope) | `mojom::VaultService::AddEntry` |
| `shield:get-rules` | Node `fs.readFile` | `ShieldServiceImpl` + Rust filter engine | `mojom::ShieldService::GetSubscriptions` |
| `shield:set-site` | `session.defaultSession.webRequest` hooks | `ShieldServiceImpl` per-site toggle map | `mojom::ShieldService::SetSiteShieldEnabled` |
| `fetcher:download` | `DownloadItem.pause()`, `DownloadItem.resume()` | `content::DownloadManager` observer | `mojom::MineFetcher::Pause/Resume/Cancel` |
| `protocol:register`| `protocol.registerSchemesAsPrivileged` | `content::ProtocolHandler` | Immutable startup registration |

---

## 3. 8-Step Component Removal Sequence (§163-164)

For each component (`vault` -> `shield` -> `fetcher` -> `protocol`):
1. **Introduce Native Service**: C++ `KeyedService` in `src/mine/<component>/`.
2. **Introduce Mojo Contract**: Formal `.mojom` interface definition with opaque handles.
3. **Migrate `chrome-ui`**: Switch UI JavaScript to invoke Mojo bindings instead of `ipcRenderer`.
4. **Verify Behavior**: Execute unit test suites and performance benchmarks.
5. **Disable Corresponding Node Path**: Deactivate legacy IPC handler.
6. **Remove Node Implementation**: Delete legacy TypeScript/JavaScript files.
7. **Remove Dead Dependencies**: Purge npm/Node packages from build configurations.
8. **Verify No Remaining IPC Path**: Pass [`tools/check_no_electron_ipc.py`](file:///c:/Users/manoh/codem37/tools/check_no_electron_ipc.py) in CI.
