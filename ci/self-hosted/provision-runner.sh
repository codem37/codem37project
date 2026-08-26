#!/usr/bin/env bash
# Provisioning Script for codem37 Self-Hosted Linux Runner (Ubuntu 22.04 LTS x64)
set -euo pipefail

echo "===================================================="
echo "  codem37 Self-Hosted Runner Provisioning (Linux)"
echo "===================================================="

# 1. Update OS packages & install core dependencies
sudo apt-get update
sudo apt-get install -y \
  build-essential \
  clang \
  cmake \
  curl \
  git \
  git-lfs \
  libglib2.0-dev \
  libgtk-3-dev \
  libnss3-dev \
  ninja-build \
  python3 \
  python3-pip \
  sccache \
  xvfb

# 2. Setup persistent cache directories
mkdir -p "$HOME/.cache/codem37-build/sccache"
export SCCACHE_DIR="$HOME/.cache/codem37-build/sccache"
export SCCACHE_CACHE_SIZE="100G"

# 3. Clone and pin depot_tools
if [ ! -d "$HOME/depot_tools" ]; then
  git clone https://chromium.googlesource.com/chromium/tools/depot_tools.git "$HOME/depot_tools"
fi

# Add depot_tools to PATH in bashrc
if ! grep -q 'depot_tools' "$HOME/.bashrc"; then
  echo 'export PATH="$HOME/depot_tools:$PATH"' >> "$HOME/.bashrc"
  echo 'export DEPOT_TOOLS_UPDATE=0' >> "$HOME/.bashrc"
fi

echo "[✓] Linux runner provisioning complete."
