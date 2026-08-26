<#
.SYNOPSIS
    codem37 - True Fork Initialization Script (Windows PowerShell)
.DESCRIPTION
    Initializes git repository, sets up upstream remote, fetches the pinned
    Chromium milestone tag, and creates codem37-main and upstream-tracking branches.
#>

[CmdletBinding()]
param()

$ErrorActionPreference = "Stop"

$ScriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$RootDir = Split-Path -Parent $ScriptDir
$VersionFile = Join-Path $RootDir "CHROMIUM_VERSION"

if (-not (Test-Path $VersionFile)) {
    Write-Error "[-] Error: CHROMIUM_VERSION file not found at $VersionFile"
    exit 1
}

# Parse version parameters from CHROMIUM_VERSION
$VersionConfig = @{}
Get-Content $VersionFile | ForEach-Object {
    $line = $_.Trim()
    if ($line -and -not $line.StartsWith("#")) {
        $parts = $line.Split("=", 2)
        if ($parts.Length -eq 2) {
            $VersionConfig[$parts[0].Trim()] = $parts[1].Trim()
        }
    }
}

$ChromiumTag = $VersionConfig["CHROMIUM_TAG"]
$ChromiumMilestone = $VersionConfig["CHROMIUM_MILESTONE"]
$UpstreamRemoteUrl = $VersionConfig["UPSTREAM_REMOTE_URL"]

Write-Host "====================================================" -ForegroundColor Cyan
Write-Host "  codem37 True Fork Setup (Windows)" -ForegroundColor Cyan
Write-Host "  Target Milestone: M$ChromiumMilestone ($ChromiumTag)" -ForegroundColor Cyan
Write-Host "  Upstream Remote:  $UpstreamRemoteUrl" -ForegroundColor Cyan
Write-Host "====================================================" -ForegroundColor Cyan

Set-Location $RootDir

# 1. Initialize git repository if needed
if (-not (Test-Path ".git")) {
    Write-Host "[+] Initializing local git repository..." -ForegroundColor Green
    git init -b codem37-main
}

# 2. Add upstream remote
$ExistingRemote = git remote | Where-Object { $_ -eq "upstream" }
if ($ExistingRemote) {
    Write-Host "[*] Upstream remote already configured." -ForegroundColor Yellow
} else {
    Write-Host "[+] Adding upstream Chromium remote ($UpstreamRemoteUrl)..." -ForegroundColor Green
    git remote add upstream $UpstreamRemoteUrl
}

# 3. Fetch milestone tag from upstream
Write-Host "[+] Fetching tag $ChromiumTag from upstream..." -ForegroundColor Green
try {
    git fetch upstream "refs/tags/${ChromiumTag}:refs/tags/${ChromiumTag}" --depth=1
} catch {
    Write-Host "[!] Shallow tag fetch failed, attempting standard fetch..." -ForegroundColor Yellow
    git fetch upstream "refs/tags/$ChromiumTag"
}

# 4. Initialize upstream-tracking branch
$TrackingBranch = git branch --list "upstream-tracking"
if (-not $TrackingBranch) {
    Write-Host "[+] Creating upstream-tracking branch at tag $ChromiumTag..." -ForegroundColor Green
    git branch upstream-tracking "tags/$ChromiumTag"
}

# 5. Ensure codem37-main is established
$CurrentBranch = (git branch --show-current).Trim()
if ($CurrentBranch -ne "codem37-main") {
    $MainBranch = git branch --list "codem37-main"
    if ($MainBranch) {
        git checkout codem37-main
    } else {
        Write-Host "[+] Creating codem37-main branch from tags/$ChromiumTag..." -ForegroundColor Green
        git checkout -b codem37-main "tags/$ChromiumTag"
    }
}

# 6. Create custom component directories
$MineDirs = @("vault", "shield", "fetcher", "protocols")
foreach ($dir in $MineDirs) {
    $targetPath = Join-Path $RootDir "src\mine\$dir"
    if (-not (Test-Path $targetPath)) {
        New-Item -ItemType Directory -Path $targetPath -Force | Out-Null
    }
}

Write-Host ""
Write-Host "====================================================" -ForegroundColor Cyan
Write-Host "  [✓] codem37 fork setup complete!" -ForegroundColor Green
Write-Host "  Current branch: $(git branch --show-current)" -ForegroundColor White
Write-Host "  Next steps:" -ForegroundColor White
Write-Host "    1. Run depot_tools gclient sync to populate submodules/dependencies."
Write-Host "    2. Verify initial build using gn args & ninja."
Write-Host "====================================================" -ForegroundColor Cyan
