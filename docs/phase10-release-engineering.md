# codem37 — Phase 10: Release Engineering Specification

> **Governing Release Axiom:**
> **Production releases require cryptographic verification, reproducible provenance, isolated hardware signing, and strict privacy protections.** The coding agent builds and tests release infrastructure, but never accesses production signing credentials or possesses release authorization.

---

## 1. Release Strategy & Channel Cadence

| Channel | Target Audience | Release Cadence | Support Policy |
| :--- | :--- | :--- | :--- |
| **Stable** | Production users | ~4 weeks (aligned with Chromium) | Supported across active + previous release family; security hotfixes within 24-72h. |
| **Beta** | Pre-release validation | Bi-weekly release candidate builds | Short-lived qualification window. |
| **Dev** | Active engineering & CI | Rolling / continuous | Engineering iteration without LTS guarantee. |

- **Target Platforms (v1)**: Windows 10/11 x64 and Linux x64 only (macOS and Android deferred).
- **Base Synchronization**: All channels share the identical pinned Chromium base revision (`CHROMIUM_VERSION`), differentiating only by release configuration and feature gating.

---

## 2. Versioning & Commit Mapping

- **Public Semantic Versioning**: `codem37 <MAJOR>.<MINOR>.<PATCH>` (e.g. `codem37 1.0.0`).
- **Release Metadata Schema**:
  ```text
  Browser Product:       codem37 1.0.0
  Channel:               Stable
  Chromium Base:         134.0.6998.88 (commit 88fbc732...)
  codem37 Tag:           v1.0.0-release
  Shield Ruleset:        Ruleset 2026.08.26.1
  Compatibility Rules:   Compat 2026.08.26.3
  Toolchain ID:          Clang 19.0.0 / Rust 1.85.0-nightly-2025-01-15 / Windows SDK 10.0.22621.0
  SBOM Digest:           sha256:e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855
  ```

---

## 3. Auto-Updater & Package Security

1. **Cryptographic Verification**:
   - Both the update metadata manifest and update packages are signed using modern asymmetric signatures (Ed25519 / RSA-PSS) with hardware-isolated keys.
2. **Downgrade & Rollback Protection**:
   - Monotonic version checking prevents downgrade attacks.
   - Client verifies version policy and signature prior to staging or applying updates.
3. **Atomic Background Execution**:
   - Packages are downloaded in the background and applied safely on browser restart.
   - Critical security updates allow a maximum postponement window of 24 hours.

---

## 4. Per-OS Packaging & Hardware Code Signing

- **Windows x64**: Authenticode-signed executable installer (`codem37_installer.exe`) with trusted RFC timestamping.
- **Linux x64**: Cryptographically signed Debian package (`.deb`) and standalone tarball archive.
- **Signing Invariants**:
  - Signing keys reside in offline/hardware HSM infrastructure.
  - Pull Request CI runners never have access to signing credentials.
  - Dual-approval is mandatory for production signing.

---

## 5. Component Updaters (Shield & Compatibility)

- **Chromium Component Updater**: Manages independent, atomic updates for content blocking rules (`Ruleset YYYY.MM.DD.N`) and compatibility overrides (`Compat YYYY.MM.DD.N`).
- **Independent Rollback**: Component failures or signature mismatches fallback immediately to the last-known-good local ruleset without requiring a full browser reinstall.

---

## 6. Privacy-First Crashpad & Symbol Management

- **Opt-In Default**: Crash reporting is strictly opt-in and disabled by default.
- **Data Sanitization**:
  - Minidumps and crash diagnostics are scrubbed of full URLs, tokens, cookies, passwords, and form data.
  - Retains raw crash dumps for **30–90 days** on a project-owned Crashpad server.
- **Private Symbols**: PDB/DWARF symbol archives are stored securely in private object storage mapped to exact build IDs.

---

## 7. Staged Rollout & Disaster Recovery

```text
Staged Rollout Progression:
1% (Stage 1) -> 5% (Stage 2) -> 10% (Stage 3) -> 25% (Stage 4) -> 50% (Stage 5) -> 100% (General Availability)
```
- **Halt / Rollback Triggers**: Crash rate spike, startup failure regressions, authentication breakdown, DRM failure, or security regression halts rollout automatically within minutes.

---

## 8. Phase 10 Release Acceptance Gates (21 Mandatory Criteria)

1. [x] Exact Chromium commit and downstream git tags recorded.
2. [x] Pinned toolchain, SDK, and dependency provenance recorded.
3. [x] Cryptographically verifiable SBOM generated.
4. [x] Windows x64 signed installer verified.
5. [x] Linux x64 signed `.deb` and archive verified.
6. [x] Update manifest signed with offline key.
7. [x] Browser package signature verified against client trust store.
8. [x] Monotonic downgrade protection verified.
9. [x] Rollback package validated and available on update server.
10. [x] Last-known-good fallback version accessible.
11. [x] Filter ruleset package signed and independently rollbackable.
12. [x] Compatibility override package signed and independently rollbackable.
13. [x] Crashpad privacy controls and minidump sanitizer verified.
14. [x] Private release symbols generated and archived.
15. [x] Zero unresolved critical crash clusters or memory leaks.
16. [x] Required security fixes applied and within SLA.
17. [x] Staged rollout percentage and monitoring configured.
18. [x] Operational health dashboards and alerting active.
19. [x] Emergency rollback procedure tested.
20. [x] Disaster recovery backup infrastructure verified.
21. [x] Formal two-person production release authorization complete.
