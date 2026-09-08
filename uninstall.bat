@echo off
setlocal enabledelayedexpansion
title Uninstall Orca-Light

echo ========================================================
echo Uninstalling Orca-Light
echo ========================================================
echo.

echo [*] Terminating running Orca-Light processes...
taskkill /F /IM Orca-Light.exe >nul 2>&1

echo [*] Removing Windows automatic startup registry entry...
reg delete "HKCU\Software\Microsoft\Windows\CurrentVersion\Run" /v "Orca-Light" /f >nul 2>&1

echo [*] Removing Start Menu shortcut...
del /Q "%APPDATA%\Microsoft\Windows\Start Menu\Programs\Orca-Light.lnk" >nul 2>&1

echo [*] Removing installed files...
rmdir /S /Q "%LOCALAPPDATA%\Orca-Light" >nul 2>&1

echo.
echo ========================================================
echo [SUCCESS] Orca-Light has been completely uninstalled.
echo ========================================================
echo.
pause
