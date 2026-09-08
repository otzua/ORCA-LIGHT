@echo off
setlocal enabledelayedexpansion
title Orca-Light Setup

echo ========================================================
echo Orca-Light - Ultra-lightweight Windows Launcher Setup
echo ========================================================
echo.

set "INSTALL_DIR=%LOCALAPPDATA%\Orca-Light"

echo [*] Closing any running Orca-Light processes...
taskkill /F /IM Orca-Light.exe >nul 2>&1
timeout /t 1 /nobreak >nul 2>&1

echo [*] Target directory: %INSTALL_DIR%
if not exist "%INSTALL_DIR%" (
    mkdir "%INSTALL_DIR%"
)

if not exist "%~dp0Orca-Light.exe" (
    echo.
    echo [ERROR] Orca-Light.exe was not found in this folder.
    echo.
    echo Did you double-click install.bat from inside the ZIP file?
    echo You MUST extract the ZIP first:
    echo  1. Right-click Orca-Light-Windows-x64.zip
    echo  2. Click "Extract All..."
    echo  3. Open the extracted folder and run install.bat
    echo.
    pause
    exit /b 1
)

echo [*] Installing binary files...
REM Strip Mark of the Web to prevent Windows SmartScreen silent blocks
(echo. > "%~dp0Orca-Light.exe:Zone.Identifier") 2>nul
del "%~dp0Orca-Light.exe:Zone.Identifier" >nul 2>&1

copy /Y "%~dp0Orca-Light.exe" "%INSTALL_DIR%\Orca-Light.exe" >nul
if %ERRORLEVEL% neq 0 (
    echo [ERROR] Failed to copy Orca-Light.exe. Make sure it is not locked by another process.
    pause
    exit /b 1
)

if exist "%~dp0config.ini" (
    if not exist "%INSTALL_DIR%\config.ini" (
        copy /Y "%~dp0config.ini" "%INSTALL_DIR%\config.ini" >nul
    )
)

echo [*] Creating Start Menu shortcut...
set "VBS_PATH=%TEMP%\create_orca-light_shortcut.vbs"
(
echo Set oWS = WScript.CreateObject("WScript.Shell"^)
echo sLinkFile = "%APPDATA%\Microsoft\Windows\Start Menu\Programs\Orca-Light.lnk"
echo Set oLink = oWS.CreateShortcut(sLinkFile^)
echo oLink.TargetPath = "%INSTALL_DIR%\Orca-Light.exe"
echo oLink.WorkingDirectory = "%INSTALL_DIR%"
echo oLink.Description = "Orca-Light Launcher"
echo oLink.Save
) > "%VBS_PATH%"
cscript //nologo "%VBS_PATH%" >nul 2>&1
del "%VBS_PATH%" >nul 2>&1

echo [*] Registering automatic Windows startup...
reg add "HKCU\Software\Microsoft\Windows\CurrentVersion\Run" /v "Orca-Light" /t REG_SZ /d "\"%INSTALL_DIR%\Orca-Light.exe\"" /f >nul

echo [*] Launching Orca-Light...
start "" "%INSTALL_DIR%\Orca-Light.exe"

REM Wait a moment to verify startup
timeout /t 2 /nobreak >nul 2>&1

tasklist /FI "IMAGENAME eq Orca-Light.exe" 2>nul | find /i "Orca-Light.exe" >nul
if %ERRORLEVEL% equ 0 (
    echo.
    echo ========================================================
    echo [SUCCESS] Orca-Light is installed and currently running!
    echo ========================================================
    echo.
    echo Primary Hotkey  : Windows Key + Space
    echo Fallback Hotkey : Ctrl + Space
    echo Install Path    : %INSTALL_DIR%
    echo Start Menu      : Added
    echo Autostart       : Enabled
    echo.
    echo Press [Windows Key + Space] or [Ctrl + Space] to open!
    echo.
) else (
    echo.
    echo [NOTE] Orca-Light was started. If the search bar didn't show,
    echo press Windows Key + Space or Ctrl + Space to bring it up.
    echo Check log if needed: %LOCALAPPDATA%\Orca-Light\orca-light.log
    echo.
)

pause
