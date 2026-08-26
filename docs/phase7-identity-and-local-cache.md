# codem37 — Phase 7: Identity, FedCM/SAA, & Secure Local Data Cache Specification

> **Governing Identity & Cache Architecture:**
> 
> `chrome://mine-*` is a privileged interface, not a trusted security boundary. Bookmarks, history, theme, and browser preferences are owned by a browser-process `SecureLocalCacheService`; they are encrypted at rest with AES-256-GCM and decrypted only when needed in browser-process memory. `VaultService` remains exclusively responsible for credentials, passwords, and other high-value secrets. WebUI and renderers never receive cache encryption keys or generic encryption/decryption primitives. The application provides explicit profile-scoped controls to clear decrypted runtime data (`Clear Memory`) and to delete the encrypted local cache (`Clear Local Cache`).
> 
> Security-sensitive state and the cache encryption/decryption operations remain browser-process-owned. Ordinary browser data such as bookmarks, history, theme, and preferences is also managed by the browser process through `SecureLocalCacheService`, with encrypted-at-rest storage and controlled renderer access.

---

## 1. Browser Process Component Separation

```text
Browser Process
│
├── VaultService
│   ├── Passwords
│   ├── Credentials
│   ├── Vault keys
│   ├── PIN / Passkey / PRF
│   └── Future DPoP / DBSC key handles
│
├── SecureLocalCacheService
│   ├── Bookmarks
│   ├── History
│   ├── Theme
│   └── Browser preferences
│
└── IdentityService
    ├── OAuth / OIDC (Web Compatibility)
    ├── Future DPoP (RFC 9449 P-256/ES256)
    └── Future DBSC
```

---

## 2. Local-Cache Lifecycle & Data Flow

```text
Encrypted local storage (<profile_dir>/secure_cache.db)
        │
        │ AES-256-GCM (random 256-bit key, unique 96-bit nonce per write)
        ▼
SecureLocalCacheService
        │
        │ decrypt only when required
        ▼
Browser-process runtime memory
        │
        ├── Bookmark data
        ├── History data
        ├── Theme state
        └── Preference state
        │
        ▼
WebUI receives only required values/results
```

---

## 3. The Two Distinct Clearing Operations

The application provides two separate, non-confusable lifecycle operations:

1. **Clear Memory (`ClearMemory()`)**:
   - Clears decrypted runtime cache state and releases/zeroizes browser-owned plaintext buffers and relevant temporary cryptographic material where the implementation allows deterministic clearing.
   - **Does NOT delete the encrypted local database.**
   - Subsequent requests reload and decrypt the encrypted local cache on demand.

2. **Clear Local Cache (`ClearAllCache()`)**:
   - Deletes the encrypted bookmark/history/theme/settings database file for the current profile.
   - Clears its active decrypted memory simultaneously.

---

## 4. WebUI Management (`chrome://mine-settings`)

`chrome://mine-settings` exposes the control panel under **Privacy & Data → Secure Local Cache**:

```text
Secure Local Data (AES-256-GCM Encrypted)
----------------------------------------
Bookmarks       [ Encrypted on Disk ]
History         [ Encrypted on Disk ]
Preferences     [ Encrypted on Disk ]
Theme           [ Encrypted on Disk ]

Active Runtime Memory State: [ Loaded / Cleared ]

[ Clear Memory ]      [ Clear History ]      [ Clear All Local Cache ]
```

---

## 5. Standard Web Identity & Token Custody

- **FedCM & Storage Access API (SAA)**: Preserved from upstream Chromium; native consent prompts and origin checks are never bypassed.
- **DPoP (RFC 9449)**: Asymmetric P-256 (ES256) keys generated in `VaultService`; DPoP proofs attached directly by browser network authentication layer; private keys are never exported across Mojo to WebUI.
- **DBSC (Device Bound Session Credentials)**: Reserved for future hardware-backed session binding; browser process acts as sole cryptographic mediator.
