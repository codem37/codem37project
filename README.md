# codem37

A privacy-focused browser built as a true fork of Chromium for Windows 10/11 and Linux x64.

## Quick Links

- [Phase 0 Architectural Decisions](file:///c:/Users/manoh/codem37/PHASE_0_DECISIONS.md)
- [Contributing & AI Agent Guardrails](file:///c:/Users/manoh/codem37/CONTRIBUTING.md)
- [Chromium Milestone Pinning](file:///c:/Users/manoh/codem37/CHROMIUM_VERSION)

## Repository Layout

- `src/`: Chromium source tree with direct in-tree modifications (branding, preferences, UI views).
- `src/mine/`: Net-new, self-contained codem37 components:
  - `src/mine/vault/`: Hardware-bound vault & session credential manager.
  - `src/mine/shield/`: Privacy enforcement & tracking prevention.
  - `src/mine/fetcher/`: Isolated network fetcher services.
  - `src/mine/protocols/`: Custom internal protocol handlers.
- `scripts/`: Fork setup and build management utilities.

## Initializing the Fork

### Windows (PowerShell)
```powershell
.\scripts\setup_fork.ps1
```

### Linux / Bash
```bash
./scripts/setup_fork.sh
```
