@echo off
REM ============================================================
REM  codem37 All-in-One Automated Build & Launch Script
REM ============================================================

setlocal
cd /d "%~dp0"

echo [codem37] Initializing automated build and launch...
python build\scripts\build.py --config component --target codem37 --launch

if %ERRORLEVEL% NEQ 0 (
    echo.
    echo [-] Build or launch exited with error code %ERRORLEVEL%.
    pause
)
endlocal
