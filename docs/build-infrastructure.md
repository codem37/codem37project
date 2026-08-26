# codem37 — Build Infrastructure Documentation (Phase 1)

This document specifies the build infrastructure, toolchain pinning, GN presets, CI matrix, binary size & startup budgets, and acceptance criteria for **codem37**, a privacy-focused true fork of Chromium for Windows 10/11 and Linux x64.

---

## 1. Development & Build Host Environment

- **Primary Development OS:** Ubuntu 22.04 LTS (x64).
- **Secondary Target:** Windows 11 23H2+ (built via native Windows CI runner, supporting runtime compatibility down to Windows 10 22H2).
- **Hardware Requirements:**
  - *Minimum:* 16 CPU cores, 32 GB RAM, 300 GB free NVMe SSD.
  - *Comfortable:* 32 CPU cores, 64 GB RAM, 500 GB NVMe SSD.
- **Continuous Integration:** GitHub Actions using dedicated self-hosted runners on persistent x64 compute instances.

---

## 2. Pinned Toolchains & Dependencies

| Toolchain Component | Pinned Version Source | Policy |
| :--- | :--- | :--- |
| **Chromium Source** | [`CHROMIUM_VERSION`](file:///c:/Users/manoh/codem37/CHROMIUM_VERSION) | Tagged milestone release |
| **depot_tools** | [`DEPOT_TOOLS_VERSION`](file:///c:/Users/manoh/codem37/DEPOT_TOOLS_VERSION) | Pinned commit hash; `DEPOT_TOOLS_UPDATE=0` |
| **Clang / LLVM** | Chromium `src/DEPS` | Hermetic bundled toolchain only |
| **Windows SDK** | Chromium `src/build/vs_toolchain.py` | Pinned official Windows 11 SDK |
| **Linux Sysroot** | Chromium `src/build/linux/sysroot_scripts/` | Pinned Debian/Ubuntu sysroot |
| **Python** | Python 3.10+ | Pinned `depot_tools` hermetic Python |

---

## 3. Repository & Infrastructure Structure

```text
/
├── src/
│   ├── mine/                    # Net-new owned components (browser runtime)
│   │   ├── vault/               # Hardware-bound vault & credentials
│   │   ├── shield/              # Privacy & content filtering engine
│   │   ├── fetcher/             # Isolated network fetcher
│   │   └── protocols/           # Custom codem37:// protocol handlers
│   └── ...                      # Forked Chromium tree (in-tree edits)
├── build/
│   ├── gn/                      # Version-controlled GN arg presets
│   │   ├── common.gn            # Locked base security & privacy flags
│   │   ├── debug/args.gn        # Fast debug configuration
│   │   ├── release/args.gn      # Optimized release configuration
│   │   ├── component/args.gn    # Component build for rapid linking
│   │   ├── official/args.gn     # Production official build preset
│   │   ├── asan/args.gn         # AddressSanitizer preset (Linux)
│   │   └── ubsan/args.gn        # UndefinedBehaviorSanitizer preset (Linux)
│   ├── gclient/                 # Pinned .gclient configuration
│   ├── scripts/                 # Build wrappers and environment drivers
│   ├── performance/             # Startup & runtime benchmark harness
│   └── size/                    # Binary size tracking and history
├── ci/
│   ├── workflows/               # GitHub Actions workflow YAMLs
│   └── self-hosted/             # Runner provisioning scripts & Dockerfile
├── tools/                       # Rebase helpers and license generator wrappers
└── docs/                        # Build & architectural specifications
```

---

## 4. GN Configuration Presets

### Base Locked Policy (`build/gn/common.gn`)
Mandatory flags locked across all presets that **cannot be silently overridden**:
- `enable_nacl = false`
- `google_api_key = ""`
- `google_default_client_id = ""`
- `google_default_client_secret = ""`
- `enable_widevine = true`
- `use_official_google_api_keys = false`
- `enable_remoting = false`
- `enable_reporting = false`

### Configurations
- **Debug:** `is_debug=true`, `dcheck_always_on=true`, `symbol_level=1`, `is_component_build=false`.
- **Release:** `is_debug=false`, `dcheck_always_on=false`, `symbol_level=1`, `is_official_build=false`.
- **Component:** `is_debug=true`, `is_component_build=true`, `symbol_level=0` (rapid local inner loop).
- **Official:** `is_debug=false`, `is_official_build=true`, `symbol_level=0`, `chrome_pgo_phase=0`.
- **ASan / UBSan:** `is_asan=true` / `is_ubsan=true`, scoped to owned code and Mojo boundaries.

---

## 5. Continuous Integration (CI) Matrix

| Platform | Debug (PR) | Release (PR) | Smoke Test (PR) | ASan / UBSan (Nightly) | Clean Reproducibility (Weekly) |
| :--- | :---: | :---: | :---: | :---: | :---: |
| **Linux x64 (Ubuntu 22.04)** | ✓ | ✓ | ✓ | ✓ | ✓ |
| **Windows x64 (Win 11 Runner)** | ✓ | ✓ | ✓ | — | — |

- **Concurrency:** 1 job per platform per branch.
- **Max Runtime SLA:** 90 minutes for warm-cache PR builds; hard failure if > 180 minutes.
- **Build Isolation:** Compilation runs in an offline environment (`--offline` / disabled network egress) after explicit dependency pre-fetching.

---

## 6. Performance & Size Budgets

### Binary Size Budget
- **Ceiling:** Maximum **+15%** vs. from-source stock Chromium baseline at the same milestone.
- **Tracked Targets:** Browser executable (`codem37`), Renderer, GPU process, and compressed installer/package.
- **Rolling Thresholds:**
  - *Warning:* `+3%` increase over previous merge cycle.
  - *Failure:* `+8%` increase over previous merge cycle (or breaching the +15% absolute ceiling).

### Startup Time Budget
- **Metric:** `DOMContentLoaded` fired for a fixed local test page (`about:blank` / `chrome://newtab`).
- **Sample Protocol:** 10 runs, discard run 1 (cache warm-up), average remaining 9 runs.
- **Primary Acceptance Metric:** p50 latency (p90 recorded for observation).
- **Rolling Thresholds:**
  - *Warning:* `+10%` p50 latency increase over baseline.
  - *Failure:* `+25%` p50 latency increase over baseline.
- **Execution:** Nightly CI schedule only (to eliminate PR noise on shared runners).

---

## 7. Sanitizer Strategy (ASan & UBSan)

- **Scope:** Net-new owned modules in `src/mine/` (`vault`, `shield`, `fetcher`, `protocols`) and immediate Mojo IPC boundaries.
- **Platforms:** Linux x64 (Nightly).
- **Release Gating:** Any new ASan/UBSan finding in `src/mine/` strictly blocks release.
- **Suppression Policy:** Suppressions require explicit issue links, rationale, and expiry dates recorded in `docs/sanitizer_suppressions.md`.

---

## 8. Build Caching (`sccache`)

- **Backend:** Local persistent SSD storage on self-hosted runners (`~/.cache/codem37-build` or dedicated NVMe cache volume).
- **Cache Size:** 100 GB minimum per runner.
- **Cache Invalidation:** Automated cache directory partition per Chromium milestone bump; manual purge option available in CI.

---

## 9. Phase 1 Acceptance Criteria

1. `codem37-main` checks out cleanly on Ubuntu 22.04 and builds Debug + Release using committed scripts.
2. `codem37-main` checks out cleanly on Windows 11 runner and builds Debug + Release using committed scripts.
3. `depot_tools` and toolchain revisions are pinned in version files.
4. GN presets exist in `build/gn/` with locked security and privacy parameters.
5. Nightly ASan + UBSan runs succeed on Linux for owned modules.
6. Binary size tracking and startup benchmark harnesses are wired to CI with automated regression warnings.
7. Build artifacts (binaries and separate symbol packages) are archived per build (90 days PR/nightly, indefinite releases).
8. Weekly clean-checkout reproducibility job passes.
9. Network egress is blocked during compile phases.
