#!/usr/bin/env python3
"""
codem37 Build Driver Script
Generates GN configuration from versioned presets and invokes Ninja.
"""

import argparse
import os
import shutil
import subprocess
import sys
from pathlib import Path

VALID_CONFIGS = ["debug", "release", "component", "official", "asan", "ubsan"]

def main():
    parser = argparse.ArgumentParser(description="codem37 Build Driver")
    parser.add_argument(
        "--config",
        choices=VALID_CONFIGS,
        default="debug",
        help="Build configuration preset (default: debug)",
    )
    parser.add_argument(
        "--target",
        default="codem37",
        help="Ninja build target (default: codem37)",
    )
    parser.add_argument(
        "--out-dir",
        default=None,
        help="Output directory (default: out/<config>)",
    )
    parser.add_argument(
        "--clean",
        action="store_true",
        help="Clean output directory before building",
    )
    parser.add_argument(
        "--offline",
        action="store_true",
        help="Enforce offline compile constraints",
    )
    parser.add_argument(
        "--gn-only",
        action="store_true",
        help="Generate GN configuration only, do not invoke Ninja",
    )

    args = parser.parse_args()

    root_dir = Path(__file__).resolve().parent.parent.parent
    out_dir = Path(args.out_dir) if args.out_dir else root_dir / "out" / args.config
    preset_file = root_dir / "build" / "gn" / args.config / "args.gn"

    if not preset_file.is_file():
        print(f"[-] Error: Preset file not found: {preset_file}", file=sys.stderr)
        sys.exit(1)

    print("=" * 60)
    print(f"  codem37 Build System")
    print(f"  Configuration : {args.config}")
    print(f"  Target        : {args.target}")
    print(f"  Output Dir    : {out_dir}")
    print(f"  Preset File   : {preset_file}")
    print("=" * 60)

    # Clean if requested
    if args.clean and out_dir.exists():
        print(f"[+] Cleaning output directory: {out_dir}")
        shutil.rmtree(out_dir)

    out_dir.mkdir(parents=True, exist_ok=True)

    # Copy / Write args.gn to output directory
    dest_args = out_dir / "args.gn"
    shutil.copy2(preset_file, dest_args)
    print(f"[+] Installed GN args to {dest_args}")

    # Set pinned environment
    env = os.environ.copy()
    env["DEPOT_TOOLS_UPDATE"] = "0"
    if args.offline:
        env["CODEM37_BUILD_OFFLINE"] = "1"

    # 1. Run GN gen
    gn_cmd = ["gn", "gen", str(out_dir)]
    print(f"[+] Executing: {' '.join(gn_cmd)}")
    result = subprocess.run(gn_cmd, cwd=root_dir, env=env)
    if result.returncode != 0:
        print("[-] GN generation failed!", file=sys.stderr)
        sys.exit(result.returncode)

    if args.gn_only:
        print("[✓] GN generation completed successfully.")
        return

    # 2. Run Ninja / autoninja
    ninja_exe = "autoninja" if shutil.which("autoninja") else "ninja"
    ninja_cmd = [ninja_exe, "-C", str(out_dir), args.target]
    print(f"[+] Executing: {' '.join(ninja_cmd)}")
    result = subprocess.run(ninja_cmd, cwd=root_dir, env=env)
    if result.returncode != 0:
        print("[-] Ninja compilation failed!", file=sys.stderr)
        sys.exit(result.returncode)

    print("=" * 60)
    print(f"  [✓] Build completed successfully: {out_dir / args.target}")
    print("=" * 60)

if __name__ == "__main__":
    main()
