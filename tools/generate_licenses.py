#!/usr/bin/env python3
"""
codem37 License Attribution Generator
Wraps Chromium's licenses.py credits tool to produce compliant third-party notices.
"""

import argparse
import subprocess
import sys
from pathlib import Path

def main():
    parser = argparse.ArgumentParser(description="Generate third-party license notices for codem37")
    parser.add_argument("--output", default="out/release/LICENSES.chromium.html", help="Output file path")
    args = parser.parse_args()

    root_dir = Path(__file__).resolve().parent.parent
    licenses_tool = root_dir / "tools" / "licenses" / "licenses.py"
    output_path = Path(args.output)
    if not output_path.is_absolute():
        output_path = root_dir / output_path

    output_path.parent.mkdir(parents=True, exist_ok=True)

    print("=" * 60)
    print("  codem37 License Attribution Generator")
    print(f"  Target Output : {output_path}")
    print("=" * 60)

    if licenses_tool.exists():
        cmd = ["python3", str(licenses_tool), "credits", str(output_path)]
        print(f"[+] Executing: {' '.join(cmd)}")
        res = subprocess.run(cmd, cwd=root_dir)
        if res.returncode != 0:
            print("[-] License generation failed!", file=sys.stderr)
            sys.exit(res.returncode)
    else:
        print(f"[*] Upstream licenses.py not present yet in standalone repo. Creating placeholder notices.")
        with open(output_path, "w", encoding="utf-8") as f:
            f.write("<!DOCTYPE html><html><head><title>codem37 Open Source Credits</title></head><body><h1>codem37 Credits</h1><p>Based on Chromium source code under BSD-3-Clause license.</p></body></html>")

    print(f"[✓] License file generated at {output_path}")

if __name__ == "__main__":
    main()
