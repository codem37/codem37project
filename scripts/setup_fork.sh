#!/usr/bin/env bash
# codem37 - True Fork Initialization Script (Linux / macOS / Bash)
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ROOT_DIR="$(cd "${SCRIPT_DIR}/.." && pwd)"

VERSION_FILE="${ROOT_DIR}/CHROMIUM_VERSION"

if [ ! -f "${VERSION_FILE}" ]; then
  echo "[-] Error: CHROMIUM_VERSION file not found at ${VERSION_FILE}" >&2
  exit 1
fi

# Parse version parameters
CHROMIUM_TAG=$(grep '^CHROMIUM_TAG=' "${VERSION_FILE}" | cut -d '=' -f2 | tr -d ' \r\n')
CHROMIUM_MILESTONE=$(grep '^CHROMIUM_MILESTONE=' "${VERSION_FILE}" | cut -d '=' -f2 | tr -d ' \r\n')
UPSTREAM_REMOTE_URL=$(grep '^UPSTREAM_REMOTE_URL=' "${VERSION_FILE}" | cut -d '=' -f2 | tr -d ' \r\n')

echo "===================================================="
echo "  codem37 True Fork Setup"
echo "  Target Milestone: M${CHROMIUM_MILESTONE} (${CHROMIUM_TAG})"
echo "  Upstream Remote:  ${UPSTREAM_REMOTE_URL}"
echo "===================================================="

cd "${ROOT_DIR}"

# 1. Initialize git repository if not already initialized
if [ ! -d ".git" ]; then
  echo "[+] Initializing local git repository..."
  git init -b codem37-main
fi

# 2. Add upstream remote
if git remote get-url upstream >/dev/null 2>&1; then
  echo "[*] Upstream remote already configured:"
  git remote -v | grep upstream
else
  echo "[+] Adding upstream Chromium remote (${UPSTREAM_REMOTE_URL})..."
  git remote add upstream "${UPSTREAM_REMOTE_URL}"
fi

# 3. Fetch shallow/full milestone tag from upstream
echo "[+] Fetching tag ${CHROMIUM_TAG} from upstream..."
git fetch upstream "refs/tags/${CHROMIUM_TAG}:refs/tags/${CHROMIUM_TAG}" --depth=1 || {
  echo "[!] Shallow tag fetch failed, attempting standard fetch..."
  git fetch upstream "refs/tags/${CHROMIUM_TAG}"
}

# 4. Initialize upstream-tracking branch
if ! git show-ref --verify --quiet refs/heads/upstream-tracking; then
  echo "[+] Creating upstream-tracking branch at tag ${CHROMIUM_TAG}..."
  git branch upstream-tracking "tags/${CHROMIUM_TAG}"
fi

# 5. Ensure codem37-main is established
CURRENT_BRANCH=$(git branch --show-current || echo "")
if [ "${CURRENT_BRANCH}" != "codem37-main" ]; then
  if git show-ref --verify --quiet refs/heads/codem37-main; then
    git checkout codem37-main
  else
    echo "[+] Creating codem37-main branch from tags/${CHROMIUM_TAG}..."
    git checkout -b codem37-main "tags/${CHROMIUM_TAG}"
  fi
fi

# 6. Create custom component directories
mkdir -p "${ROOT_DIR}/src/mine/vault"
mkdir -p "${ROOT_DIR}/src/mine/shield"
mkdir -p "${ROOT_DIR}/src/mine/fetcher"
mkdir -p "${ROOT_DIR}/src/mine/protocols"

echo ""
echo "===================================================="
echo "  [✓] codem37 fork setup complete!"
echo "  Current branch: $(git branch --show-current)"
echo "  Next steps:"
echo "    1. Run depot_tools gclient sync to populate submodules/dependencies."
echo "    2. Verify initial build using gn args & ninja."
echo "===================================================="
