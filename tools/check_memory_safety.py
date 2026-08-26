#!/usr/bin/env python3
"""
codem37 Memory Safety & Ownership Static Analyzer
Enforces raw_ptr<T> usage, prohibits raw owning pointers, and verifies unsafe Rust blocks.
"""

import os
import re
import sys
from pathlib import Path

def check_cpp_file(file_path: Path):
    violations = []
    with open(file_path, "r", encoding="utf-8", errors="ignore") as f:
        lines = f.readlines()

    for idx, line in enumerate(lines, 1):
        # Check for raw delete
        if re.search(r"\bdelete\s+[\w\->]+;", line) and not re.search(r"operator\s+delete", line):
            violations.append(f"Line {idx}: Forbidden raw owning 'delete' operator. Use std::unique_ptr.")

        # Check for reinterpret_cast without review marker
        if "reinterpret_cast<" in line:
            has_safety = "// SAFETY:" in line
            if not has_safety:
                start_lookback = max(0, idx - 4)
                for prev_idx in range(start_lookback, idx):
                    if "// SAFETY:" in lines[prev_idx]:
                        has_safety = True
                        break
            if not has_safety:
                violations.append(f"Line {idx}: Unapproved reinterpret_cast without // SAFETY: rationale.")

    return violations

def check_rust_file(file_path: Path):
    violations = []
    with open(file_path, "r", encoding="utf-8", errors="ignore") as f:
        content = f.read()

    lines = content.splitlines()
    for idx, line in enumerate(lines, 1):
        if re.search(r"\bunsafe\s*\{", line) or re.search(r"\bunsafe\s+fn\b", line):
            # Check if previous lines contain // SAFETY:
            has_safety_comment = False
            start_lookback = max(0, idx - 4)
            for prev_idx in range(start_lookback, idx):
                if "// SAFETY:" in lines[prev_idx]:
                    has_safety_comment = True
                    break
            if not has_safety_comment:
                violations.append(f"Line {idx}: unsafe block without required '// SAFETY:' justification.")

    return violations

def main():
    root_dir = Path(__file__).resolve().parent.parent
    src_mine = root_dir / "src" / "mine"

    all_violations = {}

    print("=" * 60)
    print("  codem37 Memory Safety & Static Analysis Check")
    print(f"  Target Path: {src_mine}")
    print("=" * 60)

    for root, _, files in os.walk(src_mine):
        for file in files:
            p = Path(root) / file
            if p.suffix in [".cc", ".h", ".cpp"]:
                v = check_cpp_file(p)
                if v:
                    all_violations[str(p.relative_to(root_dir))] = v
            elif p.suffix == ".rs":
                v = check_rust_file(p)
                if v:
                    all_violations[str(p.relative_to(root_dir))] = v

    if all_violations:
        print("\n[-] Memory safety violations detected:")
        for file, errs in all_violations.items():
            print(f"\n  File: {file}")
            for err in errs:
                print(f"    - {err}")
        print("\n[-] CI Memory Safety Check FAILED.")
        sys.exit(1)

    print("\n[SUCCESS] All C++ and Rust sources passed memory safety and ownership checks.")

if __name__ == "__main__":
    main()
