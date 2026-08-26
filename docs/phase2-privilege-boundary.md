# codem37 — Phase 2: Process Model & Site Isolation (Privilege Boundary Specification)

> **Governing Axiom:**
> **WebUI is a privileged interface, not a privileged trust zone.** `chrome://mine/*` renderers are treated as potentially compromised, same as any other renderer. Sensitive state (raw keys, unwrapped tokens, plaintext credentials) lives solely in the browser process. Renderers — including your own UI — only ever request narrowly scoped, authorization-checked operations over Mojo.

---

## 1. Threat Model & Asset Classification

### Assets & Retention Boundaries

| Asset Class | Storage Location | Exposed to WebUI? | Exposed to Renderer? |
| :--- | :--- | :---: | :---: |
| **Vault Master Key (VMK)** | Browser Process / Hardware Token | 🚫 Never | 🚫 Never |
| **Content Encryption Keys (CEKs)** | Browser Process Memory (Encrypted on Disk) | 🚫 Never | 🚫 Never |
| **Bulk Plaintext Passwords** | Browser Process Memory | 🚫 Never | 🚫 Never |
| **Autofilled Credential (Single Entry)** | Browser Process -> Target Web Form | 🚫 Never | ✅ Verified Target Origin Only |
| **Vault Metadata (Site, Username)** | Browser Process -> `chrome://vault` | ✅ Over Mojo | 🚫 Never to Web |
| **Shield Filter Rules & Toggles** | Browser Process -> `chrome://shield` | ✅ Over Mojo | 🚫 Never to Web |

---

## 2. Operation Privilege Matrix

| Operation | Renderer Allowed? | Browser Process Only? |
| :--- | :---: | :---: |
| Read non-sensitive UI state (toggle labels, profile names) | ✅ | Source of truth: ✅ |
| Read vault entry metadata (site name, username — **not** password) | ✅ (via Mojo call result) | Source of truth: ✅ |
| Access or hold raw vault key material | 🚫 No | ✅ |
| Perform cryptographic operation (unlock, decrypt-for-autofill) | 🚫 No (Request only) | ✅ |
| Read shield filter-list state | ✅ (via Mojo call result) | Source of truth: ✅ |
| Modify shield allow/block lists | 🚫 No (Request only) | ✅ |
| Write protected profile/vault data | 🚫 No | ✅ |
| Direct file-system access (vault DB, filter cache) | 🚫 No | ✅ |
| Network access for filter-list updates | 🚫 No | ✅ |
| Modify persistent preferences | Via `PrefService` Mojo | ✅ (Enforced by `PrefService`) |

---

## 3. WebUI Architecture & Origin Scoping

### Origins & Controllers

1. **`chrome://vault`**: High-privilege credential management UI.
   - Bound exclusively to `mojom::VaultService`.
   - Factory: `VaultWebUIControllerFactory` creating `VaultWebUIController`.
2. **`chrome://shield`**: Medium-privilege content-blocking dashboard.
   - Bound exclusively to `mojom::ShieldService`.
   - Factory: `ShieldWebUIControllerFactory` creating `ShieldWebUIController`.
3. **`chrome://mine-settings`**: Low-privilege browser preferences UI.
   - Interacts strictly with standard Chromium `PrefService`.
   - Factory: `MineSettingsWebUIControllerFactory` creating `MineSettingsWebUIController`.

### Security Rules
- **No Umbrella Host**: Distinct origins per feature to structurally limit blast radius.
- **Strict CSP**: `default-src 'self'; script-src 'self'; object-src 'none'; frame-ancestors 'none';`. No inline scripts or remote resources.
- **No Embedding / No Iframes**: External or third-party web content is strictly forbidden inside privileged WebUI origins.
- **404 Fail-Closed**: Unregistered sub-paths resolve to 404 error pages, never elevated fallbacks.

---

## 4. Multi-Profile KeyedService Architecture

- Every service (`VaultServiceImpl`, `ShieldServiceImpl`) is an instance of `KeyedService` managed via `BrowserContextKeyedServiceFactory`.
- **Instance-per-Profile**: Services are bound to a specific `content::BrowserContext` at construction time.
- **No Cross-Profile Parameterization**: Requests are implicitly scoped to the bound context. Profile A cannot query Profile B's data even with a forged profile identifier.
- **Clean Teardown**: Upon profile destruction, Mojo pipes disconnect and in-flight callbacks drop cleanly using `base::WeakPtr`.

---

## 5. Non-Negotiable Guardrail (§154 & §Q)

> **"Never weaken isolation or sandboxing to make a feature work; redesign the feature to fit inside the existing boundary."**

### Prohibited Operations for Coding Agents & Developers
1. Modifying Chromium's Full Site-per-Process isolation or OOPIF policies.
2. Altering `//sandbox/*` configurations or platform sandbox profiles.
3. Modifying `//content/browser/child_process_security_policy*` without explicit developer sign-off.
4. Adding new custom renderer process privilege tiers.
5. Exposing new Mojo interfaces to renderers without explicit design review.
6. Permitting raw cryptographic keys or bulk plaintext to cross Mojo pipes.
