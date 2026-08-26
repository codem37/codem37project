#!/usr/bin/env python3
"""
codem37 Truthful Compatibility & Anti-Evasion Verifier
Scans src/mine/ to ensure zero deceptive UA spoofing, JA4 evasion hacks,
or security-disabling compatibility shortcuts exist.
"""

import os
import re
import sys
from pathlib import Path

FORBIDDEN_PATTERNS = [
    (r"\bJA4\b", "Forbidden JA4 fingerprint evasion hack"),
    (r"\bTLS_MIMICRY\b", "Forbidden TLS mimicry evasion"),
    (r"\bDisableSiteIsolation\b", "Forbidden compatibility exception disabling Site Isolation"),
    (r"\bDisableSandbox\b", "Forbidden compatibility exception disabling sandbox"),
    (r"\bBypassCertValidation\b", "Forbidden certificate validation bypass"),
    (r"Mozilla/5\.0.*Firefox/", "Deceptive Firefox User-Agent spoofing"),
    (r"Version/\d+\.\d+.*Safari/", "Deceptive Safari User-Agent spoofing"),
]

EXCLUDED_EXTENSIONS = [".md", ".json", ".py", ".bak"]

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
    src_mine = root_dir / "src" / "mine"

    all_violations = {}

    print("=" * 60)
    print("  codem37 Truthful Compatibility & Anti-Evasion Audit")
    print(f"  Scanning: {src_mine}")
    print("=" * 60)

    for root, _, files in os.walk(src_mine):
        for file in files:
            p = Path(root) / file
            v = check_file(p)
            if v:
                all_violations[str(p.relative_to(root_dir))] = v

    if all_violations:
        print("\n[-] Forbidden evasion, spoofing, or security bypass found:")
        for file, errs in all_violations.items():
            print(f"\n  File: {file}")
            for err in errs:
                print(f"    - {err}")
        print("\n[-] CI Compatibility & Anti-Evasion Check FAILED.")
        sys.exit(1)

    print("\n[SUCCESS] Compatibility security verified: 0 deceptive spoofing or security-bypass patterns found.")

if __name__ == "__main__":
    main()
