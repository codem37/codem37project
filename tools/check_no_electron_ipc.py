#!/usr/bin/env python3
"""
codem37 Electron & Legacy IPC Elimination Verifier
Scans codebase to verify complete removal of Electron/Node IPC bridges,
ensuring pure native Chromium architecture.
"""

import os
import re
import sys
from pathlib import Path

FORBIDDEN_PATTERNS = [
    (r"\bipcRenderer\b", "Legacy Electron ipcRenderer invocation"),
    (r"\bipcMain\b", "Legacy Electron ipcMain handler"),
    (r"\bcontextBridge\b", "Legacy Electron contextBridge export"),
    (r"\brequire\s*\(\s*['\"]electron['\"]\s*\)", "Electron runtime dependency import"),
    (r"\bfrom\s+['\"]electron['\"]", "Electron ES6 import"),
    (r"\bregisterSchemesAsPrivileged\b", "Electron scheme registration"),
    (r"\bwill-download\b", "Legacy Electron will-download event"),
]

EXCLUDED_EXTENSIONS = [".md", ".json", ".py", ".html", ".bak"]

def check_file(file_path: Path):
    violations = []
    if file_path.suffix in EXCLUDED_EXTENSIONS:
        return violations

    with open(file_path, "r", encoding="utf-8", errors="ignore") as f:
        lines = f.readlines()

    for idx, line in enumerate(lines, 1):
        for pattern, desc in FORBIDDEN_PATTERNS:
            if re.search(pattern, line):
                violations.append(f"Line {idx}: {desc} ('{line.strip()}')")

    return violations

def main():
    root_dir = Path(__file__).resolve().parent.parent
    src_dir = root_dir / "src"

    all_violations = {}

    print("=" * 60)
    print("  codem37 Electron & Legacy IPC Elimination Audit")
    print(f"  Scanning: {src_dir}")
    print("=" * 60)

    for root, _, files in os.walk(src_dir):
        for file in files:
            p = Path(root) / file
            v = check_file(p)
            if v:
                all_violations[str(p.relative_to(root_dir))] = v

    if all_violations:
        print("\n[-] Lingering Electron/Node IPC patterns found:")
        for file, errs in all_violations.items():
            print(f"\n  File: {file}")
            for err in errs:
                print(f"    - {err}")
        print("\n[-] CI Electron Elimination Check FAILED.")
        sys.exit(1)

    print("\n[✓] Clean native architecture: 0 Electron/Node IPC bridges found in src/.")

if __name__ == "__main__":
    main()
