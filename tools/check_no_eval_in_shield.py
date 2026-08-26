#!/usr/bin/env python3
"""
codem37 Shield No-Eval & Data-Driven Scriptlet Verifier
Scans src/mine/shield/ to ensure zero eval(), new Function(), or arbitrary
remote script execution paths exist.
"""

import os
import re
import sys
from pathlib import Path

FORBIDDEN_PATTERNS = [
    (r"\beval\s*\(", "Forbidden dynamic JavaScript eval() invocation"),
    (r"\bnew\s+Function\s*\(", "Forbidden new Function() dynamic code generator"),
    (r"\bExecuteScriptAsync\s*\(", "Unbounded main-world script injection"),
    (r"\bsetTimeout\s*\(\s*['\"]", "Forbidden string-based setTimeout evaluation"),
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
    shield_dir = root_dir / "src" / "mine" / "shield"

    all_violations = {}

    print("=" * 60)
    print("  codem37 Shield Safe Scriptlet & No-Eval Audit")
    print(f"  Scanning: {shield_dir}")
    print("=" * 60)

    for root, _, files in os.walk(shield_dir):
        for file in files:
            p = Path(root) / file
            v = check_file(p)
            if v:
                all_violations[str(p.relative_to(root_dir))] = v

    if all_violations:
        print("\n[-] Dynamic evaluation or dangerous execution patterns found in shield:")
        for file, errs in all_violations.items():
            print(f"\n  File: {file}")
            for err in errs:
                print(f"    - {err}")
        print("\n[-] CI Shield Security Check FAILED.")
        sys.exit(1)

    print("\n[✓] Shield security verified: 0 dynamic eval/arbitrary script execution patterns found.")

if __name__ == "__main__":
    main()
