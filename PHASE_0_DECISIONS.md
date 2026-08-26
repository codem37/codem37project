# codem37 — Phase 0 Decisions (True Fork)

**Context:** solo developer, moving off Electron, targeting Windows 10/11 + Linux x64, privacy-focused. This revision commits to a **true fork**: you own a full copy of the Chromium source under your own git history, you modify files directly anywhere in the tree (not routed through a separate patch-overlay directory), and you own your browser's UI/shell rather than treating Chromium's `views`/WebUI shell as something you only lightly reskin.

---

## What "true fork" changes mechanically

A true fork means three concrete differences from a downstream-patch-layer approach:

1. **You mirror the Chromium `src` repository into your own git hosting** and do development directly on your own long-lived branch, rather than treating Chromium as an external dependency fetched at build time.
2. **You edit Chromium's own files in place** — `chrome/browser/ui/*`, `content/*`, branding constants, build files — as ordinary commits in your history, not as a separately-tracked patch series applied on top of an untouched checkout.
3. **Rebasing upstream becomes a merge, not a patch-reapply.** Periodically you add the official Chromium repo as a remote, fetch a new milestone tag, and `git merge` (or rebase) it into your branch, resolving conflicts by hand where your direct edits collide with upstream's.

The tradeoff: conflict resolution on a true fork is harder than reapplying a clean patch series because changes are interleaved in the same files. Maintainability requires discipline: **replace the UI incrementally, not all at once.** Own the whole tree, but don't rewrite the whole tree in month one.

---

## A. Product definition

- **Name:** codem37.
- **Primary target:** Privacy-focused browser, single clear differentiator for v1.
- **Differentiator vs Chrome/Edge/Brave/Vivaldi:** Stricter default privacy posture than Brave (no telemetry at all, not opt-out; no bundled crypto/rewards product) combined with a fully custom native shell and hardware-bound vault/session-security architecture — *"the privacy browser built and owned end-to-end, not just configured differently."*
- **Primary user:** Privacy-conscious individual power user for v1.
- **OS support:** Windows 10/11 and Linux x64 (macOS skipped in Phase 0; ARM64 post-v1).
- **General-purpose vs controlled:** General-purpose consumer browser.

## B. Chromium base

- **Initial milestone:** Pinned to a specific numbered stable milestone tag (recorded in [`CHROMIUM_VERSION`](file:///c:/Users/manoh/codem37/CHROMIUM_VERSION)).
- **Exact commit/tag:** Official tagged stable release commit.
- **Single vs multiple Chromium versions:** Single version at a time — `codem37-main` always reflects one merged-in Chromium milestone plus codem37 commits.
- **Rebase cadence:** Every second stable release (~8 weeks) as a floor.
- **Security patch lag:** <= 14 days for critical/actively-exploited CVEs via direct cherry-pick into `codem37-main`.

## C. Fork strategy

- **Model:** True fork. Mirror Chromium `src` git history into private GitHub repo.
- **In-tree direct edits:** Default mechanism for branding, UI, security integrations, and build configs.
- **Role of `src/mine/`:** Holds net-new self-contained components (Vault, Shield, Fetcher, custom protocol handlers).
- **Commit discipline:** Commit prefix `[codem37] <component>: <change>` explaining the architectural rationale and referencing a tracked issue.
- **Diff policy:** Prefer minimal, surgical diffs within upstream files over wholesale rewrites to minimize future 8-week merge conflict burdens.

## D. Repository and source control

- **Hosting:** GitHub (private repo).
- **Branch naming:**
  - `codem37-main`: Main integrated development branch.
  - `upstream-tracking`: Clean branch tracking upstream Chromium stable tags without local edits.
  - `release/<version>`: Cut from `codem37-main` at release time.
  - `feature/<issue>-<name>`: Isolated feature branches.
- **Upstream tracking:** Chromium remote (`https://chromium.googlesource.com/chromium/src.git`).
- **Milestone tags:** Tagged on `codem37-main` after each successful milestone merge (`chromium-M<xxx>-merged`).
- **Proprietary code separation:** Any future closed-source backends live in distinct repositories, consumed as build dependencies.

## E. Licensing and legal

- **Code license:** BSD-3-Clause for all original and modified code inside the tree.
- **Headers:** Preserve upstream headers; append copyright line for substantial modifications.
- **Third-party attribution:** Automated generation via `python3 tools/licenses/licenses.py credits` during release builds.
- **Proprietary binaries:** Widevine CDM requires Google license agreement; H.264/AAC subject to standard patent licensing.
- **Google services stripped:** Google Sync backend, telemetry/variations pings, sign-in promotional nags.
- **Entity:** Formalize as LLC/company prior to public distribution.

## F. Branding

- **Product Name:** `codem37`
- **Application ID:** `com.codem37.browser`
- **Executable:** `codem37` (Windows: `codem37.exe`, Linux: `codem37`)
- **Package IDs:**
  - Windows: AppUserModelID `com.codem37.browser`, MSI/EXE product name `codem37`
  - Linux: Package `codem37-browser`, desktop entry `com.codem37.browser`
- **Branding Assets:** Flat SVG master located in `chrome/app/theme/codem37/` exported via build pipeline.

## G. Build system

- **Primary build host:** Linux (fast compilation); Windows builds via CI runner or cross-toolchain.
- **Hardware baseline:** 16+ cores, 32–64GB RAM, 300GB+ SSD storage.
- **CI platform:** GitHub Actions with self-hosted runners.
- **Tooling:** Official pinned `depot_tools`, `gn`, and `ninja`.
- **Target configurations:** Release, Debug, Component build (v1); Official and ASan/CFI (post-stabilization).
- **Phase 0 Exit Target:** A `codem37`-branded binary launching on Windows & Linux, loading an HTTPS site, and executing JavaScript.

## H. Secrets & Services

- Minimal/no Google API keys.
- CI secrets stored strictly in GitHub Actions encrypted secrets (never committed).
- Telemetry disabled by default; own Crashpad endpoint for opt-in crash reporting.

## I. Google/Chromium Services

- **Removed/Disabled:** Google Sync, variations/metrics pings, account promotion surfaces, cast-to-Google integrations.
- **Retained:** Site isolation, local hash-prefix Safe Browsing checks, WebAuthn/passkeys, DevTools, extension platform.
- **Extensions:** Unmodified Chrome Web Store compatibility.

## J. Security Model

- **Process Model:** Full multi-process architecture strictly preserved.
- **Site Isolation:** Enabled by default on all desktop targets.
- **Sandbox:** Mandatory in all release builds.
- **Privileged Services:** Browser process trust tier hosts Vault/Shield Mojo interfaces.
- **Fuzzing & Sanitizers:** libFuzzer suites for new components; ASan/UBSan/CFI in CI.
- **Security Patch Policy:** Cherry-pick critical CVEs within <= 14 days.

## K. Updates & Channels

- ~8-week milestone updates; <=14-day security out-of-band updates.
- Auto-update enabled by default with manual opt-out.
- Single `Stable` channel for v1.

## L. Privacy Defaults

- Private-by-default posture.
- Zero telemetry endpoints configured by default.
- 3rd-party cookies blocked by default; storage partitioning enabled.
- Default settings baked directly into Chromium preference and feature flag files.

## M. Authentication & Identity

- Full WebAuthn / Passkey support preserved.
- No mandatory cloud account; local encrypted multi-profile support.

## N. UI Architecture

- **Phased Approach:**
  - *Phase 0–2:* Modified & rebranded native Chromium UI shell (`chrome/browser/ui/views/`) while stabilizing engine services.
  - *Later Phases:* Incrementally transition to fully custom native shell.
- **Native UI:** Toolbar, tab strip, omnibox, window frame, dialogs.
- **WebUI Pages:** Settings, Vault management, Shield dashboard, NTP (`chrome://` pages via TypeScript + Lit conventions).

## O. Architecture Boundaries

- **`//third_party/blink/*`:** Hard rule against direct edits to preserve web compatibility and avoid security divergence.
- **`//v8/*`:** Hard rule against direct edits.
- **`//chrome/browser/*`:** Direct in-tree edits allowed for UI, preferences, branding, and Mojo integrations.
- **`src/mine/`:** Net-new self-contained modules (`vault/`, `shield/`, `fetcher/`, `protocols/`).

## P. Extension Compatibility

- Manifest V3 standard with unmodified Chrome Web Store install flow.
- Regression testing: Netflix (Widevine CDM) and Google Sign-In (FedCM/OAuth) flows.

## Q. Distribution

- Direct downloads (Windows MSI/EXE installer, Linux deb/rpm/Flatpak).
- Mandatory code signing with hardware token / secure CI key vault before release.

## R. Testing & Acceptance (Phase 0)

- Clean build from `codem37-main`.
- Binary launches cleanly on Windows and Linux x64.
- Successful HTTPS navigation and JavaScript execution.
- Verification via `chrome://sandbox` and `chrome://process-internals`.
- Automated smoke test in CI pipeline.

## S. Development Workflow & Guardrails

- **Language Standards:** Chromium C++ Style Guide (`.clang-format`), TypeScript/Lit for WebUI.
- **Commit Convention:** `[codem37] <component>: <description>` referencing issue IDs.
- **AI Agent Guardrails:**
  - **Permitted:** In-tree edits to branding, preferences, UI views, build files (`BUILD.gn`), and `src/mine/`.
  - **Strictly Prohibited without Sign-off:** Direct edits to `//sandbox/*`, site isolation logic, process model internals, `//third_party/blink/*`, and `//v8/*`.
- **Branch Rule:** Work in feature branches, push to fork repository for review; never push directly to `codem37-main` or upstream remotes.
