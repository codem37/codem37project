# Contributing to codem37

Welcome to **codem37**, a privacy-focused browser built as a true fork of Chromium for Windows 10/11 and Linux x64.

This document outlines our development workflow, coding standards, branching model, and strict operational guardrails for both human contributors and AI coding agents.

---

## 1. True Fork Philosophy & Architecture

In codem37, we own a full copy of the Chromium source under our own git history:
- **Direct In-Tree Edits**: Modifications to branding, preferences, native UI (`chrome/browser/ui/views/*`), and build targets are made directly in the tree with clear commit histories.
- **Isolated Modules (`src/mine/`)**: Net-new, self-contained components (Vault, Shield, Fetcher, custom protocol handlers) reside in `src/mine/`.
- **Architectural Discipline**: We preserve Blink (`//third_party/blink/*`) and V8 (`//v8/*`) without modifications to maintain web compatibility and leverage upstream security hardening.

---

## 2. Git & Branching Strategy

| Branch | Purpose | Rules |
| :--- | :--- | :--- |
| `codem37-main` | Mainline development branch | Permanently diverged from Chromium `main`. Protected; merged via PRs. |
| `upstream-tracking` | Upstream Chromium mirror | Receives clean fetches/tags from Google's Chromium remote. No local edits. |
| `feature/<issue>-<name>` | Work branch for specific tasks | Branch from `codem37-main`, commit changes, submit for review. |
| `release/<version>` | Release stabilization | Cut from `codem37-main` for release builds and hotfixes. |

---

## 3. Commit Message Convention

Every commit touching the repository must follow this convention:

```text
[codem37] <component>: <summary>

Why: <Architectural rationale explaining why this change was made>
Issue: #<issue_number>
Subsystems-Touched: <comma-separated list of directories>
```

### Examples:
- `[codem37] branding: replace default browser icon assets and product strings`
- `[codem37] privacy: disable default variations and telemetry service endpoints`
- `[codem37] vault: add initial Mojo interface definition in src/mine/vault`

---

## 4. Coding Standards

- **C++**: Follow the official [Chromium C++ Style Guide](https://google.github.io/styleguide/cppguide.html).
  - Enforced via `.clang-format` and `git cl format` / `clang-format`.
  - Prefer modern C++ (C++20), RAII, smart pointers (`std::unique_ptr`, `scoped_refptr`), and Chromium base primitives (`base::BindOnce`, `base::span`).
- **WebUI (TypeScript / Lit)**:
  - Follow Chromium's WebUI toolchain and Lit component conventions.
  - Do NOT introduce external UI frameworks (such as React) into core WebUI pages to avoid merge conflicts with upstream WebUI tooling.
- **Licenses & Headers**:
  - Preserve existing BSD-3-Clause headers in modified upstream files.
  - Append your copyright notice on files receiving substantial modifications:
    `// Copyright (c) 2026 The codem37 Authors. All rights reserved.`

---

## 5. Coding Agent Guardrails (Section S)

Coding agents (including LLM-driven pair programmers) operate under explicit permission tiers:

### ✅ Permitted Operations (Automated / In-Tree)
- Modifying branding resources (`//chrome/app/theme`, `BRANDING` files, version strings).
- Editing default user preferences and feature flag configurations.
- Customizing native UI in `//chrome/browser/ui/views/` and WebUI in `//chrome/browser/resources/`.
- Adding and modifying self-contained modules in `src/mine/`.
- Editing GN build targets (`BUILD.gn`) and dependency lists (`DEPS`).
- Creating feature branches and staging commits.

### 🚫 Strictly Forbidden Without Explicit User Sign-Off
Coding agents are **strictly prohibited** from modifying the following security-critical subsystems without explicit confirmation and review:
1. **Sandbox Subsystems**: Any code in `//sandbox/*` or platform sandbox profiles.
2. **Site Isolation & Process Model**: `//content/browser/child_process_security_policy*`, process allocation logic, or IPC trust boundaries.
3. **Rendering Engine Core**: `//third_party/blink/*`.
4. **JavaScript Engine Core**: `//v8/*`.
5. **Remote Pushes**: Pushing directly to `codem37-main` or pushing to the upstream Chromium remote.

---

## 6. Upstream Rebase & Security Patching Protocol

### 8-Week Milestone Rebase (Floor)
1. Fetch latest stable tag on `upstream-tracking`.
2. Checkout a rebase branch: `git checkout -b rebase/M<target> codem37-main`.
3. Merge `upstream-tracking` tag into the rebase branch.
4. Resolve conflicts file-by-file, consulting commit rationale tags (`[codem37]`).
5. Run full build and automated smoke test suite on Linux & Windows.
6. Merge into `codem37-main` and tag `chromium-M<target>-merged`.
7. Update [`CHROMIUM_VERSION`](file:///c:/Users/manoh/codem37/CHROMIUM_VERSION).

### <= 14-Day Critical CVE Cherry-Picks
- For zero-day / critical CVEs, cherry-pick the exact upstream security fix commit directly onto `codem37-main` out-of-band without waiting for the full 8-week milestone merge.
