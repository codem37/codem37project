<#
.SYNOPSIS
    codem37 All-in-One Automated Bootstrap, Build, and Launch Script for PowerShell.
#>

$ErrorActionPreference = "Stop"
$ScriptDir = Split-Path -Parent $MyInvocation.MyCommand.Definition
$RootDir = Split-Path -Parent $ScriptDir

Set-Location $RootDir

Write-Host "============================================================" -ForegroundColor Cyan
Write-Host "  codem37 Automated Bootstrap & Run Driver" -ForegroundColor Cyan
Write-Host "============================================================" -ForegroundColor Cyan

# 1. Run Python build script with auto-bootstrap and auto-launch
python "$RootDir\build\scripts\build.py" --config component --target codem37 --launch
