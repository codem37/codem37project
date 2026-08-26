#!/usr/bin/env python3
"""
codem37 Secure Local Cache Mojo Boundary Verifier
Scans src/mine/cache/ to ensure no generic encrypt/decrypt methods or raw keys
are exposed over Mojo to WebUI.
"""

import os
import re
import sys
from pathlib import Path

FORBIDDEN_MOJO_PATTERNS = [
    (r"\bEncrypt\s*\(", "Forbidden generic Encrypt method on Mojo interface"),
    (r"\bDecrypt\s*\(", "Forbidden generic Decrypt method on Mojo interface"),
    (r"\bGetRawKey\s*\(", "Forbidden raw encryption key exposure"),
    (r"\bExportKey\s*\(", "Forbidden key export operation"),
    (r"\bGetToken\s*\(", "Forbidden token exposure to renderer"),
]

def check_mojom_file(file_path: Path):
    violations = []
    with open(file_path, "r", encoding="utf-8", errors="ignore") as f:
        lines = f.readlines()

    for idx, line in enumerate(lines, 1):
        for pattern, desc in FORBIDDEN_MOJO_PATTERNS:
            if re.search(pattern, line):
                violations.append(f"Line {idx}: {desc} ('{line.strip()}')")

    return violations

def main():
    root_dir = Path(__file__).resolve().parent.parent
    cache_mojom_dir = root_dir / "src" / "mine" / "cache" / "mojom"

    all_violations = {}

    print("=" * 60)
    print("  codem37 Secure Local Cache Mojo Boundary Audit")
    print(f"  Scanning: {cache_mojom_dir}")
    print("=" * 60)

    if cache_mojom_dir.exists():
        for root, _, files in os.walk(cache_mojom_dir):
            for file in files:
                if file.endswith(".mojom"):
                    p = Path(root) / file
                    v = check_mojom_file(p)
                    if v:
                        all_violations[str(p.relative_to(root_dir))] = v

    if all_violations:
        print("\n[-] Forbidden generic crypto or key exposure found in Mojo interface:")
        for file, errs in all_violations.items():
            print(f"\n  File: {file}")
            for err in errs:
                print(f"    - {err}")
        print("\n[-] CI Secure Cache Boundary Check FAILED.")
        sys.exit(1)

    print("\n[SUCCESS] Secure local cache capability boundary verified: 0 generic crypto/key leaks found.")

if __name__ == "__main__":
    main()
