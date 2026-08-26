#!/usr/bin/env python3
"""
codem37 Upstream Rebase Helper
Assists with 8-week Chromium milestone merges and in-tree conflict auditing.
"""

import argparse
import subprocess
import sys
from pathlib import Path

def run_cmd(cmd, cwd=None):
    print(f"[+] Executing: {' '.join(cmd)}")
    res = subprocess.run(cmd, cwd=cwd, text=True, capture_output=True)
    if res.returncode != 0:
        print(f"[-] Error: {res.stderr}", file=sys.stderr)
    return res

def main():
    parser = argparse.ArgumentParser(description="codem37 Upstream Rebase Helper")
    parser.add_argument("--fetch-milestone", help="Milestone tag to fetch from upstream (e.g. 136.0.7000.50)")
    parser.add_argument("--audit-conflicts", action="store_true", help="Audit local modifications in upstream files")
    args = parser.parse_args()

    root_dir = Path(__file__).resolve().parent.parent

    if args.fetch_milestone:
        print(f"[+] Fetching milestone tag {args.fetch_milestone} from upstream...")
        run_cmd(["git", "fetch", "upstream", f"refs/tags/{args.fetch_milestone}"], cwd=root_dir)
        print(f"[+] Updating upstream-tracking branch...")
        run_cmd(["git", "branch", "-f", "upstream-tracking", f"tags/{args.fetch_milestone}"], cwd=root_dir)
        print(f"[✓] Ready to merge into codem37-main via rebase branch: git checkout -b rebase/M-{args.fetch_milestone} codem37-main")

    if args.audit_conflicts:
        print("[+] Auditing codem37-specific commits on modified upstream files...")
        res = run_cmd(["git", "log", "--grep=\\[codem37\\]", "--oneline"], cwd=root_dir)
        print(res.stdout)

if __name__ == "__main__":
    main()
