# codem37 — Phase 7: Identity, FedCM/SAA, & Secure Local Data Cache Specification

> **Governing Identity & Cache Principles:**
> 1. **Local Encrypted Cache $\neq$ Cloud Sync**: There is no first-party codem37 account or cloud sync in v1. All browser state (Bookmarks, History, Theme, Preferences) is stored in a local AES-256-GCM encrypted cache and decrypted only transiently in browser-process memory.
> 2. **`VaultService` vs. `SecureLocalCacheService` Separation**: Sensitive credentials (passwords, encryption keys, DPoP key handles) remain strictly owned by `VaultService`. Ordinary browser data is managed by `SecureLocalCacheService`.
> 3. **Capability-Oriented Mojo**: WebUI renderers invoke high-level capability methods (`GetBookmarks`, `ClearHistory`, `ClearMemory`, `ClearAllCache`). Generic `encrypt(bytes)` or `decrypt(bytes)` methods and raw cryptographic keys are strictly prohibited from crossing IPC.
> 4. **Standard Web Identity Unmodified**: Standard Chromium FedCM (Federated Credential Management) and Storage Access API (SAA) permission prompts, consent flows, and origin checks are inherited directly from upstream Chromium and never bypassed.

---

## 1. Local Encrypted Data Architecture

```text
Browser Profile Directory (<profile_dir>/)
│
├── Secure Local Cache (secure_cache.db)
│   ├── Bookmarks (AES-256-GCM Ciphertext)
│   ├── History   (AES-256-GCM Ciphertext)
│   ├── Theme     (AES-256-GCM Ciphertext)
│   └── Settings  (AES-256-GCM Ciphertext)
│
├── Password Vault (vault.db)
│   ├── Passwords & Credentials
│   ├── Credential Metadata
│   └── Master Symmetric Key (Wrapped)
│
└── Identity Security Storage
    ├── DPoP Key Handles (P-256 / ES256)
    └── Future DBSC Platform Handles
```

### Encryption Parameters
- **Cipher**: **AES-256-GCM** authenticated encryption with 128-bit authentication tag.
- **Key Generation**: 256-bit cryptographic random key per profile.
- **Nonce/IV**: 96-bit unique cryptographically random IV generated for every individual record/blob write (never reused).
- **Tamper Protection**: Any modification to ciphertext or authentication tag immediately fails verification and rejects the record.

---

## 2. Transient Memory & Zeroization Semantics

- **Decrypted on Demand**: Encrypted records are read from disk and decrypted into transient browser-process memory only when requested.
- **Deterministic Memory Clearing**:
  - `ClearMemory()`: Explicitly zeroizes and frees all transient in-RAM plaintext cache buffers and releases active decryption keys from memory.
  - `ClearAllCache()`: Deletes the on-disk encrypted records and wipes the in-memory cache completely.
- **Acceptance Boundary**: The browser deterministically zeroizes its owned memory buffers using platform-appropriate secure memory primitives (`SecureZeroMemory` / `OPENSSL_cleanse` / `zeroize`).

---

## 3. WebUI Management (`chrome://mine-settings`)

The `chrome://mine-settings` page exposes a dedicated **Privacy & Data → Secure Local Cache** control panel:

```text
Secure Local Data (AES-256-GCM Encrypted)
----------------------------------------
Bookmarks       [ Encrypted on Disk ]
History         [ Encrypted on Disk ]
Preferences     [ Encrypted on Disk ]
Theme           [ Encrypted on Disk ]

Active Memory State: [ Loaded / Cleared ]

[ Clear Memory ]  [ Clear History ]  [ Clear All Local Cache ]
```

---

## 4. DPoP & DBSC Key Custody

- **DPoP (RFC 9449)**: Asymmetric P-256 (ES256) keys generated and stored in the browser-process vault.
- **Proof Generation**: The browser authentication service signs DPoP HTTP request proofs (`DPoP` header with `htu`, `htm`, `jti`, `iat`) internally; WebUI renderers never see or hold private keys.
- **DBSC (Device Bound Session Credentials)**: Reserved for future hardware-backed token binding; browser process acts as sole cryptographic mediator.
