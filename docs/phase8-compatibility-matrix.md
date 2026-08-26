# codem37 — Phase 8: Compatibility Matrix & Policy Specification

> **Governing Compatibility Principle:**
> **`codem37` maximizes legitimate Chromium and web platform compatibility without pretending to be another browser, bypassing identity-provider restrictions, weakening security boundaries, or redistributing proprietary DRM components without authorization.**

---

## 1. Acceptance Corpus & Compatibility Targets

| Target Area | Acceptance Sites & Platforms | Required Behavior |
| :--- | :--- | :--- |
| **Search & Web** | Google, GitHub, Wikipedia, Major News | Full navigation, service workers, modern HTML5/CSS standards. |
| **Video & Streaming** | YouTube, Netflix | YouTube player scriptlet compatibility; Netflix EME/Widevine playback (subject to licensing). |
| **Authentication** | Google Sign-In, Microsoft Entra ID / OIDC, WebAuthn/Passkeys | Standards-compliant OAuth authorization, popup flows, PKCE, and exact callback routing. |
| **Enterprise** | Enterprise SSO portals, VPN web interfaces | Seamless SAML/OIDC authentication and corporate proxy/TLS support. |
| **Extensions** | Chrome Web Store, Manifest V3 | Standard MV3 extension execution and lifecycle. |

---

## 2. User-Agent & Client Hints Policy

1. **Truthful & Non-Deceptive**:
   - `codem37` uses standard Chromium-compatible User-Agent formatting referencing the exact pinned Chromium milestone in `CHROMIUM_VERSION`.
   - Browser spoofing (pretending to be Safari or Firefox to evade restrictions) is **strictly forbidden**.
2. **Client Hints (`Sec-CH-UA`)**:
   - Aligned directly with upstream Chromium User-Agent Reduction specifications.
   - Exposes truthful brand identifier `codem37` alongside engine compatibility brands (`Chromium`).

---

## 3. Controlled Compatibility Override Subsystem

When a legitimate website compatibility issue arises due to non-standard server-side UA sniffing:
- **Strict Review Required**: Every override entry must specify:
  `id`, `target_domain`, `reason`, `chromium_milestone`, `expiry_date`, and an associated automated regression test.
- **Absolute Prohibitions**: Overrides can **NEVER**:
  - Disable or weaken Site Isolation, process sandboxing, or origin checks.
  - Bypass certificate validation or TLS security policies.
  - Weaken Content Security Policy (CSP) or CORS boundaries.
  - Manipulate TLS fingerprints (JA4 mimicry) for anti-bot evasion.

---

## 4. Widevine DRM & Media Pipeline

- **Vendor Licensing**: Widevine CDM is a separate vendor/legal dependency. `codem37` integrates with the official CDM pipeline without unofficial blob redistribution or reverse-engineering.
- **Graceful Non-DRM Fallback**: If Widevine CDM is unprovisioned, the browser clearly notifies the user:
  > *"Protected content requires DRM support that is not currently available."*
- **Hardware Acceleration**: Preserves upstream DXVA/Media Foundation on Windows and VA-API on Linux for H.264, VP9, and AV1 video decoding.

---

## 5. Privacy-Safe Diagnostics

- **Opt-In Only**: Diagnostic telemetry is strictly opt-in and disabled by default.
- **Sanitized Reports**:
  - Excludes full URLs, query strings, headers, cookies, passwords, and tokens.
  - Captures only sanitized error codes, HTTP status, TLS errors, and CDM availability.
