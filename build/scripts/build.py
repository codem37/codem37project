#!/usr/bin/env python3
"""
codem37 Build & Launch Driver Script
Automatically bootstraps build tools (depot_tools, gn, ninja via CIPD) if missing,
generates GN configuration from versioned presets, compiles, and optionally launches the browser.
"""

import argparse
import os
import shutil
import subprocess
import sys
import urllib.request
import zipfile
from pathlib import Path

VALID_CONFIGS = ["debug", "release", "component", "official", "asan", "ubsan"]
DEPOT_TOOLS_URL = "https://storage.googleapis.com/chrome-infra/depot_tools.zip"

def ensure_depot_tools(root_dir: Path) -> None:
    """Auto-detects or downloads and configures depot_tools, gn, and ninja if missing."""
    local_dt = root_dir / "third_party" / "depot_tools"
    if not local_dt.exists():
        c_src_dt = Path("C:/src/depot_tools")
        if c_src_dt.exists():
            local_dt = c_src_dt

    if not local_dt.exists():
        print("[+] Build tools not found. Automatically setting up depot_tools...")
        local_dt.mkdir(parents=True, exist_ok=True)
        zip_path = local_dt / "depot_tools.zip"

        print(f"[+] Downloading depot_tools from {DEPOT_TOOLS_URL} ...")
        try:
            urllib.request.urlretrieve(DEPOT_TOOLS_URL, zip_path)
            print("[+] Extracting depot_tools...")
            with zipfile.ZipFile(zip_path, "r") as zip_ref:
                zip_ref.extractall(local_dt)
            if zip_path.exists():
                zip_path.unlink()
            print(f"[✓] depot_tools installed to {local_dt}")
        except Exception as e:
            print(f"[-] Automated download failed: {e}. Please ensure internet connectivity.", file=sys.stderr)

    # 1. Add tools paths to process PATH
    gn_dir = root_dir / "third_party" / "gn"
    ninja_dir = root_dir / "third_party" / "ninja"
    llvm_bin = Path("C:/Program Files/LLVM/bin")
    llvm_bin86 = Path("C:/Program Files (x86)/LLVM/bin")

    extra_paths = [str(gn_dir), str(ninja_dir), str(local_dt)]
    if llvm_bin.exists():
        extra_paths.append(str(llvm_bin))
    if llvm_bin86.exists():
        extra_paths.append(str(llvm_bin86))

    os.environ["PATH"] = os.pathsep.join(extra_paths) + os.pathsep + os.environ.get("PATH", "")
    os.environ["DEPOT_TOOLS_WIN_TOOLCHAIN"] = "0"
    os.environ["DEPOT_TOOLS_UPDATE"] = "0"

    # 2. Check and install standalone gn and ninja binaries via CIPD if missing
    cipd_exe = local_dt / ("cipd.bat" if sys.platform == "win32" else "cipd")
    gn_exe = gn_dir / ("gn.exe" if sys.platform == "win32" else "gn")
    ninja_exe = ninja_dir / ("ninja.exe" if sys.platform == "win32" else "ninja")

    if cipd_exe.exists():
        if not gn_exe.exists():
            print("[+] Fetching standalone GN binary via CIPD...")
            pkg = "gn/gn/windows-amd64" if sys.platform == "win32" else "gn/gn/linux-amd64"
            subprocess.run([str(cipd_exe), "install", pkg, "latest", "-root", str(gn_dir)], shell=(sys.platform == "win32"))

        if not ninja_exe.exists():
            print("[+] Fetching standalone Ninja binary via CIPD...")
            pkg = "infra/3pp/tools/ninja/windows-amd64" if sys.platform == "win32" else "infra/3pp/tools/ninja/linux-amd64"
            subprocess.run([str(cipd_exe), "install", pkg, "latest", "-root", str(ninja_dir)], shell=(sys.platform == "win32"))

def find_gn(root_dir: Path) -> str:
    gn_bin = root_dir / "third_party" / "gn" / ("gn.exe" if sys.platform == "win32" else "gn")
    if gn_bin.exists():
        return str(gn_bin)
    return shutil.which("gn") or "gn"

def find_ninja(root_dir: Path) -> str:
    ninja_bin = root_dir / "third_party" / "ninja" / ("ninja.exe" if sys.platform == "win32" else "ninja")
    if ninja_bin.exists():
        return str(ninja_bin)
    return shutil.which("autoninja") or shutil.which("ninja") or "ninja"

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
    parser.add_argument(
        "--launch",
        action="store_true",
        help="Automatically launch the browser after successful build",
    )

    args = parser.parse_args()

    root_dir = Path(__file__).resolve().parent.parent.parent
    out_dir = Path(args.out_dir) if args.out_dir else root_dir / "out" / args.config
    preset_file = root_dir / "build" / "gn" / args.config / "args.gn"

    if not preset_file.is_file():
        print(f"[-] Error: Preset file not found: {preset_file}", file=sys.stderr)
        sys.exit(1)

    # Auto-bootstrap build tools
    ensure_depot_tools(root_dir)

    print("=" * 60)
    print(f"  codem37 Automated Build System")
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
    gn_cmd = [find_gn(root_dir), "gen", str(out_dir)]
    print(f"[+] Executing: {' '.join(gn_cmd)}")
    result = subprocess.run(gn_cmd, cwd=root_dir, env=env)
    if result.returncode != 0:
        print("[-] GN generation failed!", file=sys.stderr)
        sys.exit(result.returncode)

    if args.gn_only:
        print("[✓] GN generation completed successfully.")
        return

    # 2. Run Ninja / autoninja
    ninja_cmd = [find_ninja(root_dir), "-C", str(out_dir), args.target]
    print(f"[+] Executing: {' '.join(ninja_cmd)}")
    result = subprocess.run(ninja_cmd, cwd=root_dir, env=env)
    if result.returncode != 0:
        print("[-] Ninja compilation failed!", file=sys.stderr)
        sys.exit(result.returncode)

    print("=" * 60)
    print(f"  [✓] Build completed successfully: {out_dir / args.target}")
    print("=" * 60)

    # 3. Launch if requested
    if args.launch:
        exe_ext = ".exe" if sys.platform == "win32" else ""
        browser_exe = out_dir / f"{args.target}{exe_ext}"
        if not browser_exe.exists():
            browser_exe = out_dir / f"chrome{exe_ext}"

        if browser_exe.exists():
            print(f"\n[+] Launching browser: {browser_exe} ...")
            subprocess.Popen([str(browser_exe)])
        else:
            print(f"[-] Executable not found at {browser_exe}", file=sys.stderr)

if __name__ == "__main__":
    main()
