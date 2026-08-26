# codem37 — Phase 6: Content Blocking Engine Specification

> **Governing Principle:**
> **The content blocking engine (`shield`) is a strict security and privacy boundary against untrusted remote data, not a backdoor for arbitrary code execution.** Filter lists and scriptlets are treated as strictly typed data payloads, parsed exclusively in memory-safe Rust (`adblock-rust`), and applied securely via Chromium's isolated worlds. Remote filter lists must never possess the capability to execute arbitrary, unvalidated JavaScript in the user's browser.

---

## 1. Engine Architecture & Component Boundaries

```text
Remote Update CDN (Ed25519 Signed Bundles)
   │ (6-hour background check, monotonic vYYYYMMDD.X)
   ▼
Browser Process (ShieldServiceImpl)
   ├── Signature Verifier (Hardcoded Ed25519 Public Key)
   ├── Rollback Store (Retains 2 last-known-good bundles in profile)
   │
   ├── Network Interception (URLLoaderRequestInterceptor)
   │      └── Evaluates outgoing requests synchronously (<1ms p95 latency budget)
   │
   └── Rust Core Engine (adblock-rust Integration)
          ├── EasyList / EasyPrivacy / uBO Unbreak Rule Matcher
          ├── Cosmetic Filter Synthesizer (Origin-scoped CSS for OOPIFs)
          └── Pre-compiled Scriptlet Catalog (json-prune, set-constant, replace-fetch-response)
                 └── Injected strictly at document-start into Blink Isolated Worlds
```

---

## 2. Rule-List Sources & Precedence Model

1. **Rule Sources**:
   - **Default Lists**: EasyList, EasyPrivacy, uBO Quick Fixes / Unbreak.
   - **Curated Namespace**: `codem37-unbreak` (highest precedence for rapid hot-fixes e.g. YouTube player shifts).
   - **Optional Lists**: Regional blockers, Fanboy's Annoyances (toggleable in `chrome://shield`).
   - **Custom Remote Lists**: User-added via URL (hard-capped at 20MB raw text to prevent OOM).
2. **Evaluation Precedence**:
   `codem37-unbreak` > User Site Exception (Allow/Block) > Global Filter Lists.

---

## 3. Data-Driven Scriptlets & Isolated World Execution

- **Data-Driven, Not Arbitrary Code**: Remote lists cannot inject arbitrary code. A rule specifies an action identifier (`json-prune`, `set-constant`, `replace-fetch-response`) and argument parameters (e.g., JSON path, constant value).
- **Native Implementation**: The browser maps action identifiers to pre-compiled native routines shipped in the browser binary.
- **Strict Isolated Worlds**: Injected scripts execute in Blink isolated worlds at `document-start` (`ReadyToCommitNavigation`), partitioned from the main page execution context.
- **No Privileged Execution**: Injections are unconditionally skipped on `chrome://` and `chrome-extension://` origins.

---

## 4. Cryptographic Update Verification & Rollback

- **Ed25519 Signatures**: Every official filter bundle is cryptographically verified using a hardcoded Ed25519 public key before any bytes reach the parser.
- **Fail-Safe Rollback**: Tampered, corrupted, or crashing updates are quarantined immediately; `shield` reverts to the previous cached bundle without downtime.
- **Atomic Pointer Swap**: Swapping active rule engines uses atomic shared pointers (`std::shared_ptr` / `Arc`), preventing lock contention with the network stack.

---

## 5. Performance & Telemetry Privacy

- **Performance Budgets**:
  - Evaluation latency: **<1ms p95**.
  - Memory overhead: **<50MB** for compiled rule database.
  - Compilation runs asynchronously off the UI thread via `base::ThreadPool`.
- **Strictly Opt-In Telemetry**:
  - Disabled by default.
  - Transmits **only** stripped eTLD+1 domain, active rule bundle version, browser version, and OS.
  - **Absolute Prohibitions**: NO full URLs, query strings, headers, cookies, or page contents. Stored in a local 7-day rolling SQLite buffer.
