#!/usr/bin/env python3
"""
codem37 Build & Launch Driver Script
Configures GN arguments from presets, manages depot_tools integration,
executes Ninja compilation with memory-aware concurrency throttling, and launches the browser.
"""

import argparse
import os
import shutil
import subprocess
import sys
from pathlib import Path

VALID_CONFIGS = ["debug", "release", "component", "official", "asan", "ubsan"]

def ensure_environment(root_dir: Path) -> Path:
    """Prepares depot_tools in PATH and sets Chromium toolchain environment variables."""
    depot_tools = root_dir / "third_party" / "depot_tools"
    if not depot_tools.exists():
        c_src_dt = Path("C:/src/depot_tools")
        if c_src_dt.exists():
            depot_tools = c_src_dt

    os.environ["PATH"] = str(depot_tools) + os.pathsep + os.environ.get("PATH", "")
    os.environ["DEPOT_TOOLS_WIN_TOOLCHAIN"] = "0"
    os.environ["DEPOT_TOOLS_UPDATE"] = "0"
    return depot_tools

def find_gn(src_dir: Path, root_dir: Path) -> str:
    # 1. Canonical Chromium buildtools
    bt_gn = src_dir / "buildtools" / "win" / "gn.exe"
    if bt_gn.exists():
        return str(bt_gn)
    # 2. depot_tools / system gn
    dt_gn = root_dir / "third_party" / "depot_tools" / ("gn.bat" if sys.platform == "win32" else "gn")
    if dt_gn.exists():
        return str(dt_gn)
    return shutil.which("gn") or "gn"

def find_ninja(root_dir: Path) -> str:
    autoninja = root_dir / "third_party" / "depot_tools" / ("autoninja.bat" if sys.platform == "win32" else "autoninja")
    if autoninja.exists():
        return str(autoninja)
    ninja_bin = root_dir / "third_party" / "ninja" / ("ninja.exe" if sys.platform == "win32" else "ninja")
    if ninja_bin.exists():
        return str(ninja_bin)
    return shutil.which("ninja") or "ninja"

def main():
    parser = argparse.ArgumentParser(description="codem37 Build & Launch Driver")
    parser.add_argument(
        "--config",
        choices=VALID_CONFIGS,
        default="component",
        help="Build configuration preset (default: component)",
    )
    parser.add_argument(
        "--target",
        default="chrome",
        help="Ninja build target (default: chrome)",
    )
    parser.add_argument(
        "--out-dir",
        default=None,
        help="Output directory (default: src/out/<config> or out/<config>)",
    )
    parser.add_argument(
        "--jobs", "-j",
        type=int,
        default=6,
        help="Ninja concurrent jobs (default: 6 for 8GB RAM profile)",
    )
    parser.add_argument(
        "--clean",
        action="store_true",
        help="Clean output directory before building",
    )
    parser.add_argument(
        "--gn-only",
        action="store_true",
        help="Generate GN configuration only, do not invoke Ninja",
    )
    parser.add_argument(
        "--launch",
        action="store_true",
        help="Automatically launch the browser after successful build",
    )

    args = parser.parse_args()

    root_dir = Path(__file__).resolve().parent.parent.parent
    src_dir = root_dir / "src"
    
    # Check if real Chromium source tree exists
    is_real_chromium = (src_dir / ".gn").exists()
    work_dir = src_dir if is_real_chromium else root_dir
    
    out_dir = Path(args.out_dir) if args.out_dir else (src_dir / "out" / args.config if is_real_chromium else root_dir / "out" / args.config)
    preset_file = root_dir / "build" / "gn" / args.config / "args.gn"

    ensure_environment(root_dir)

    print("=" * 60)
    print(f"  codem37 Automated Build System")
    print(f"  Build Root    : {work_dir}")
    print(f"  Configuration : {args.config}")
    print(f"  Target        : {args.target}")
    print(f"  Output Dir    : {out_dir}")
    print(f"  Preset File   : {preset_file}")
    print(f"  Parallel Jobs : -j {args.jobs} (8GB RAM Protected)")
    print("=" * 60)

    if not preset_file.is_file():
        print(f"[-] Error: Preset file not found: {preset_file}", file=sys.stderr)
        sys.exit(1)

    if args.clean and out_dir.exists():
        print(f"[+] Cleaning output directory: {out_dir}")
        shutil.rmtree(out_dir)

    out_dir.mkdir(parents=True, exist_ok=True)

    dest_args = out_dir / "args.gn"
    shutil.copy2(preset_file, dest_args)
    print(f"[+] Installed GN args to {dest_args}")

    env = os.environ.copy()
    env["DEPOT_TOOLS_WIN_TOOLCHAIN"] = "0"
    env["DEPOT_TOOLS_UPDATE"] = "0"

    # 1. Run GN gen
    gn_cmd = [find_gn(src_dir, root_dir), "gen", str(out_dir)]
    print(f"[+] Executing: {' '.join(gn_cmd)}")
    result = subprocess.run(gn_cmd, cwd=str(work_dir), env=env)
    if result.returncode != 0:
        print("[-] GN generation failed!", file=sys.stderr)
        sys.exit(result.returncode)

    if args.gn_only:
        print("[SUCCESS] GN generation completed successfully.")
        return

    # 2. Run Ninja with memory-safe concurrency
    ninja_cmd = [find_ninja(root_dir), "-C", str(out_dir), f"-j{args.jobs}", args.target]
    print(f"[+] Executing: {' '.join(ninja_cmd)}")
    result = subprocess.run(ninja_cmd, cwd=str(work_dir), env=env)
    
    if result.returncode != 0:
        print(f"\n[-] Ninja build failed (exit code {result.returncode}).", file=sys.stderr)
        print("[-] If the failure was due to memory exhaustion (OOM), retrying with link-safe concurrency: -j 2 ...")
        retry_cmd = [find_ninja(root_dir), "-C", str(out_dir), "-j2", args.target]
        result = subprocess.run(retry_cmd, cwd=str(work_dir), env=env)
        if result.returncode != 0:
            print("[-] Retried build also failed.", file=sys.stderr)
            sys.exit(result.returncode)

    print("=" * 60)
    print(f"  [SUCCESS] Build completed successfully: {out_dir / args.target}")
    print("=" * 60)

    # 3. Launch if requested
    if args.launch:
        exe_ext = ".exe" if sys.platform == "win32" else ""
        browser_exe = out_dir / f"{args.target}{exe_ext}"
        if not browser_exe.exists():
            browser_exe = out_dir / f"codem37{exe_ext}"

        if browser_exe.exists():
            print(f"\n[+] Launching browser: {browser_exe} ...")
            subprocess.Popen([str(browser_exe)])
        else:
            print(f"[-] Executable not found at {browser_exe}", file=sys.stderr)

if __name__ == "__main__":
    main()
