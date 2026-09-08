@echo off
title Orca-Light Portable
cd /d "%~dp0"

echo [*] Closing any old instances...
taskkill /F /IM Orca-Light.exe >nul 2>&1
timeout /t 1 /nobreak >nul 2>&1

echo [*] Starting Orca-Light...
REM Strip Mark of the Web to prevent Windows SmartScreen silent blocks
(echo. > "%~dp0Orca-Light.exe:Zone.Identifier") 2>nul
del "%~dp0Orca-Light.exe:Zone.Identifier" >nul 2>&1

start "" "%~dp0Orca-Light.exe"

timeout /t 2 /nobreak >nul 2>&1

tasklist /FI "IMAGENAME eq Orca-Light.exe" 2>nul | find /i "Orca-Light.exe" >nul
if %ERRORLEVEL% equ 0 (
    echo.
    echo ========================================================
    echo [SUCCESS] Orca-Light is running!
    echo ========================================================
    echo.
    echo Primary Hotkey  : Windows Key + Space
    echo Fallback Hotkey : Ctrl + Space
    echo.
    echo Press [Windows Key + Space] or [Ctrl + Space] to toggle Orca-Light.
    echo Type "> exit" in Orca-Light to quit.
    echo.
) else (
    echo.
    echo [ERROR] Orca-Light did not stay running.
    echo Checking diagnostic log:
    if exist "%LOCALAPPDATA%\Orca-Light\orca-light.log" (
        echo ----------------------------------------------------
        type "%LOCALAPPDATA%\Orca-Light\orca-light.log"
        echo ----------------------------------------------------
    ) else (
        echo No log file generated yet.
    )
    echo.
)

pause
