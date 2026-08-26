@echo off
REM ============================================================
REM  codem37 All-in-One Automated Build & Launch Script
REM ============================================================

setlocal enabledelayedexpansion
cd /d "%~dp0"

REM Find Python executable
set "PYTHON_EXE=python"
if exist "%LOCALAPPDATA%\Programs\Python\Python312\python.exe" (
    set "PYTHON_EXE=%LOCALAPPDATA%\Programs\Python\Python312\python.exe"
) else if exist "%ProgramFiles%\Python312\python.exe" (
    set "PYTHON_EXE=%ProgramFiles%\Python312\python.exe"
) else if exist "%~dp0third_party\depot_tools\bootstrap-2@3_11_8_chromium_35_bin\python3\bin\python3.exe" (
    set "PYTHON_EXE=%~dp0third_party\depot_tools\bootstrap-2@3_11_8_chromium_35_bin\python3\bin\python3.exe"
)

echo [codem37] Initializing automated build and launch using: !PYTHON_EXE!
"!PYTHON_EXE!" build\scripts\build.py --config component --target codem37 --launch

if %ERRORLEVEL% NEQ 0 (
    echo.
    echo [-] Build or launch exited with error code %ERRORLEVEL%.
    pause
)
endlocal
