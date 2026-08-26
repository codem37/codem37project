<#
.SYNOPSIS
    Provisioning Script for codem37 Self-Hosted Windows Runner (Windows 11 x64)
.DESCRIPTION
    Installs prerequisites, configures sccache persistent directories,
    and sets up pinned depot_tools environment.
#>

[CmdletBinding()]
param()

$ErrorActionPreference = "Stop"

Write-Host "====================================================" -ForegroundColor Cyan
Write-Host "  codem37 Self-Hosted Runner Provisioning (Windows)" -ForegroundColor Cyan
Write-Host "====================================================" -ForegroundColor Cyan

# 1. Setup persistent cache directories
$CacheDir = "C:\cache\codem37-build\sccache"
if (-not (Test-Path $CacheDir)) {
    New-Item -ItemType Directory -Path $CacheDir -Force | Out-Null
}

[Environment]::SetEnvironmentVariable("SCCACHE_DIR", $CacheDir, "User")
[Environment]::SetEnvironmentVariable("SCCACHE_CACHE_SIZE", "100G", "User")
[Environment]::SetEnvironmentVariable("DEPOT_TOOLS_UPDATE", "0", "User")

Write-Host "[+] Cache and environment configured at $CacheDir" -ForegroundColor Green

# 2. Verify / Setup depot_tools path
$DepotToolsPath = "C:\src\depot_tools"
if (-not (Test-Path $DepotToolsPath)) {
    Write-Host "[!] depot_tools not detected at $DepotToolsPath. Please clone depot_tools to $DepotToolsPath" -ForegroundColor Yellow
} else {
    $CurrentPath = [Environment]::GetEnvironmentVariable("Path", "User")
    if (-not $CurrentPath.Contains($DepotToolsPath)) {
        [Environment]::SetEnvironmentVariable("Path", "$DepotToolsPath;$CurrentPath", "User")
        Write-Host "[+] Added $DepotToolsPath to User Path" -ForegroundColor Green
    }
}

Write-Host "[✓] Windows runner provisioning completed." -ForegroundColor Green
