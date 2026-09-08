@echo off
setlocal enabledelayedexpansion

echo ========================================================
echo Orca-Light - Native Windows Launcher Build Script
echo ========================================================
echo.

where cmake >nul 2>nul
if %ERRORLEVEL% equ 0 (
    echo [*] CMake detected. Configuring build with CMake...
    if not exist build (
        mkdir build
    )
    cd build
    cmake -G "Visual Studio 17 2022" -A x64 ..
    if %ERRORLEVEL% neq 0 (
        echo [!] CMake configuration failed. Trying default generator...
        cmake ..
    )
    echo [*] Building Release executable...
    cmake --build . --config Release
    if %ERRORLEVEL% equ 0 (
        echo.
        echo [SUCCESS] Build completed!
        echo Executable location: build\bin\Release\Orca-Light.exe or build\Release\Orca-Light.exe
    ) else (
        echo.
        echo [FAIL] Build encountered errors.
    )
    cd ..
    exit /b %ERRORLEVEL%
)

where cl >nul 2>nul
if %ERRORLEVEL% neq 0 (
    echo [*] Attempting to locate Visual Studio vcvars64.bat...
    set "VS_PATH="
    if exist "%ProgramFiles%\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat" (
        set "VS_PATH=%ProgramFiles%\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat"
    ) else if exist "%ProgramFiles%\Microsoft Visual Studio\2022\Professional\VC\Auxiliary\Build\vcvars64.bat" (
        set "VS_PATH=%ProgramFiles%\Microsoft Visual Studio\2022\Professional\VC\Auxiliary\Build\vcvars64.bat"
    ) else if exist "%ProgramFiles%\Microsoft Visual Studio\2022\Enterprise\VC\Auxiliary\Build\vcvars64.bat" (
        set "VS_PATH=%ProgramFiles%\Microsoft Visual Studio\2022\Enterprise\VC\Auxiliary\Build\vcvars64.bat"
    ) else if exist "%ProgramFiles(x86)%\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvars64.bat" (
        set "VS_PATH=%ProgramFiles(x86)%\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvars64.bat"
    )

    if defined VS_PATH (
        echo [*] Initializing MSVC environment from: !VS_PATH!
        call "!VS_PATH!"
    ) else (
        echo [!] Neither CMake nor cl.exe found in PATH.
        echo [!] Please open 'x64 Native Tools Command Prompt for VS 2022' and run build.bat.
        exit /b 1
    )
)

echo [*] Compiling Windows resource script...
if not exist build mkdir build
rc.exe /fo build\resource.res res\resource.rc

echo [*] Compiling Orca-Light native executable (C++20, Release /O2)...
cl.exe /nologo /std:c++20 /O2 /Oi /Ot /Gy /MD /EHsc /W4 ^
    /DUNICODE /D_UNICODE /DWIN32_LEAN_AND_MEAN /DNOMINMAX /DNDEBUG ^
    /Isrc /Ires ^
    src\main.cpp ^
    src\app.cpp ^
    src\calculator\calc_engine.cpp ^
    src\clipboard\clipboard_history.cpp ^
    src\config\config_manager.cpp ^
    src\indexer\app_indexer.cpp ^
    src\indexer\file_indexer.cpp ^
    src\search\fuzzy_matcher.cpp ^
    src\search\search_engine.cpp ^
    src\system\system_commands.cpp ^
    src\ui\renderer.cpp ^
    src\ui\window.cpp ^
    src\utils\icon_loader.cpp ^
    src\utils\shell_utils.cpp ^
    src\utils\string_utils.cpp ^
    build\resource.res ^
    /link /OUT:build\Orca-Light.exe /SUBSYSTEM:WINDOWS /LTCG /OPT:REF /OPT:ICF ^
    d2d1.lib dwrite.lib windowscodecs.lib dxgi.lib shlwapi.lib shell32.lib ole32.lib oleaut32.lib advapi32.lib user32.lib gdi32.lib propsys.lib uuid.lib

if %ERRORLEVEL% equ 0 (
    echo.
    echo ========================================================
    echo [SUCCESS] Orca-Light built successfully!
    echo Binary: build\Orca-Light.exe
    echo ========================================================
) else (
    echo.
    echo [FAIL] Compilation failed.
)

exit /b %ERRORLEVEL%
