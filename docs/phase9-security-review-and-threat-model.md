# codem37 — Phase 9: Comprehensive Security Review, Threat Models & Invariants

> **Governing Security Axiom:**
> **Lower-trust execution contexts (content renderers, WebUI pages, extensions) are assumed to be fully compromised.** Browser-process services (`VaultService`, `SecureLocalCacheService`, `ShieldService`, `FetcherService`, `IdentityService`) enforce strict capability checks and memory-safe validation. Security boundaries are never weakened, disabled, or bypassed for compatibility.

---

## 1. Threat Model & Asset Hierarchy

```text
Threat Model Hierarchy
======================
[ Untrusted Input / Adversarial Boundaries ]
├── Malicious Websites & Scripts (Cross-Site Scripting, CSRF, Malicious Payloads)
├── Fully Compromised Content Renderers (Arbitrary Memory Read/Write in Renderer)
├── Malicious Browser Extensions (Hostile IPC invocations, storage snooping)
├── Malicious / Compromised WebUI Contexts (chrome:// compromised via renderer bugs)
└── Network Attackers (Man-in-the-Middle, DNS spoofing, Malicious CDN Payloads)

[ Assets Protected Under Compromised Lower Tiers ]
├── Master Encryption Keys & Unwrapped CEKs
├── Plaintext Passwords & Credential Secrets
├── Local Cache Encryption Keys & Decrypted RAM Buffers
├── DPoP Private Keys & DBSC Hardware Handles
├── Profiles & Cross-Profile Isolation
└── Host OS & Browser-Process Execution Integrity
```

---

## 2. Formal Vault Threat Model

```text
Vault Threat Model Structure
│
├── Assets: Passwords, Credentials, Vault Master Key, CEKs, PIN/PRF metadata, DPoP key handles.
├── Trust Boundary: Browser process owns keys and storage; renderers are untrusted.
├── Authorized Callers: Only chrome://vault (management) and browser autofill orchestration.
├── Authentication States: Locked -> PIN/Passkey Authenticated -> Scoped Operation.
├── Storage Model: AES-256-GCM encrypted database in profile directory (vault.db).
├── Zeroization: Plaintext and derived key buffers explicitly zeroized on lock/completion.
└── Hard Invariants:
    ├── Compromised renderers can NEVER extract raw vault keys or plaintext credentials.
    ├── Mojo disconnect immediately revokes in-flight authorizations.
    └── Browser crash automatically resets Vault to locked state.
```

---

## 3. Formal SecureLocalCacheService Threat Model

```text
SecureLocalCacheService Threat Model Structure
│
├── Assets: Bookmarks, History, Theme, Preferences.
├── Trust Boundary: Stored encrypted at rest (<profile_dir>/secure_cache.db); decrypted only on demand in RAM.
├── Encryption: AES-256-GCM with random 256-bit key and unique 96-bit nonce per write.
├── Distinct Clearing Lifecycle:
│   ├── Clear Memory: Wipes transient in-RAM plaintext cache buffers without deleting on-disk database.
│   └── Clear Local Cache: Deletes the on-disk encrypted database and wipes active memory.
└── Hard Invariants:
    ├── WebUI receives only high-level capability values (GetBookmarks, ClearHistory); never raw keys or generic crypto.
    └── Unauthorized origins cannot bind or invoke cache Mojo methods.
```

---

## 4. Resource Exhaustion & Fuzzing Limits

| Component | Resource Limit Policy | Fuzz Target |
| :--- | :--- | :--- |
| **Vault Records** | Max 10MB record payload; max 100k entries | `build/fuzz/vault_parser_fuzzer.cc` |
| **Local Cache** | Max 50MB encrypted cache file; max 1MB per record | `build/fuzz/cache_record_fuzzer.cc` |
| **Shield Rules** | Max 20MB raw list size; max 250k rules; <1ms p95 latency | `build/fuzz/shield_filter_fuzzer.cc` |
| **Fetcher Ranges**| Max 8 concurrent range loaders; 8MB min chunk size | `build/fuzz/fetcher_response_fuzzer.cc` |
| **Protocol URLs** | Max 2048 chars URL; max 10MB bundled resource | `build/fuzz/protocol_url_fuzzer.cc` |

---

## 5. Vulnerability Response SLA

- **Critical Vulnerabilities** (Remote code execution, sandbox escape, key extraction): **24–72 hours** emergency fix & release target.
- **High Vulnerabilities** (Memory corruption, security boundary bypass, site-isolation failure): **7 days** target.
- **Medium/Low Vulnerabilities**: Scheduled regular security update cycle.

---

## 6. Formal 22 Release Gates

- [x] **Gate 1**: No unresolved critical security vulnerabilities.
- [x] **Gate 2**: No unresolved critical fuzzing crashes or sanitizer failures.
- [x] **Gate 3**: Required ASan/UBSan suites pass on all security-critical parsers.
- [x] **Gate 4**: Dedicated libFuzzer targets exist for all untrusted input parsers.
- [x] **Gate 5**: Mojo security checks verify origin authentication on every call.
- [x] **Gate 6**: Vault threat model formally reviewed and verified.
- [x] **Gate 7**: Vault trust boundaries strictly prevent raw key/secret exposure over IPC.
- [x] **Gate 8**: SecureLocalCacheService threat model formally reviewed and verified.
- [x] **Gate 9**: Unauthorized Vault access negative tests pass.
- [x] **Gate 10**: Unauthorized local cache access negative tests pass.
- [x] **Gate 11**: Compromised-renderer simulation tests pass without key leak.
- [x] **Gate 12**: Per-profile isolation tests pass across all subsystems.
- [x] **Gate 13**: Applicable Chromium sandbox tests pass without waivers.
- [x] **Gate 14**: Site Isolation tests pass without compatibility exceptions.
- [x] **Gate 15**: Privileged WebUI origins (`chrome://vault`, `chrome://shield`, `chrome://mine-settings`) strictly partitioned.
- [x] **Gate 16**: Zero undocumented sandbox or security exceptions exist.
- [x] **Gate 17**: Zero undocumented privileged Mojo interfaces.
- [x] **Gate 18**: Zero secrets, passwords, tokens, or full URLs in logs/telemetry.
- [x] **Gate 19**: Supply-chain and release signing integrity checks pass.
- [x] **Gate 20**: Chromium upstream security cherry-picks are within SLA.
- [x] **Gate 21**: All third-party dependencies are pinned, reviewed, and license-verified.
- [x] **Gate 22**: Formal external security audit and penetration test conducted prior to public distribution.
