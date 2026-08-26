# codem37 — Build Scripts & Tooling Environment

This directory contains drivers and automation wrappers for building, benchmarking, and measuring **codem37**.

---

## Required Environment Variables

When running local or CI builds, configure the following environment variables:

| Variable | Recommended Value | Purpose |
| :--- | :--- | :--- |
| `DEPOT_TOOLS_UPDATE` | `0` | Disables automated updating of depot_tools to maintain version pinning. |
| `DEPOT_TOOLS_PATH` | `/path/to/depot_tools` (or added to `PATH`) | Location of pinned depot_tools checkout. |
| `SCCACHE_DIR` | `~/.cache/codem37-build/sccache` (Linux) / `C:\cache\codem37-build\sccache` (Windows) | Persistent directory for compilation cache. |
| `SCCACHE_CACHE_SIZE` | `100G` | Maximum size of compilation cache. |
| `GN_ARGS_DIR` | `//build/gn` | Relative path to versioned GN arg presets. |

---

## Build Commands

### Cross-Platform Python Driver
```bash
# Debug Build
python3 build/scripts/build.py --config debug --target codem37

# Release Build
python3 build/scripts/build.py --config release --target codem37

# Component Build (Fast Linking)
python3 build/scripts/build.py --config component --target codem37
```
