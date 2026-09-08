<div align="center">
  <img src="res/icon.ico" width="128" height="128" alt="Spotlight Logo">
  <h1>Spotlight</h1>
  <p><strong>Ultra-lightweight, native Win32, brutalist application launcher and productivity spotlight for Windows.</strong></p>

  <p>
    <a href="https://opensource.org/licenses/MIT"><img src="https://img.shields.io/badge/License-MIT-blue.svg" alt="License: MIT"></a>
    <a href="https://isocpp.org/"><img src="https://img.shields.io/badge/Language-C%2B%2B20-00599C.svg" alt="C++20"></a>
    <a href="https://www.microsoft.com/windows"><img src="https://img.shields.io/badge/Platform-Windows%2010%20%7C%2011%20(x64)-0078D6.svg" alt="Platform: Windows 10/11 x64"></a>
    <img src="https://img.shields.io/badge/Architecture-x86__64-informational.svg" alt="Architecture: x86_64">
    <img src="https://img.shields.io/badge/Graphics-Direct2D%20%2F%20DirectWrite-success.svg" alt="Graphics: Direct2D / DirectWrite">
    <img src="https://img.shields.io/badge/Runtime-Zero%20Dependencies-brightgreen.svg" alt="Runtime: Zero Dependencies">
    <img src="https://img.shields.io/badge/RAM-~12.4%20MB-orange.svg" alt="RAM: ~12.4 MB">
  </p>
</div>

<br/>

## Table of Contents

- [Executive Overview](#executive-overview)
- [Design Principles](#design-principles)
- [Core Subsystems & Architecture](#core-subsystems--architecture)
- [Feature Matrix](#feature-matrix)
- [Keyboard Navigation Reference](#keyboard-navigation-reference)
- [Performance Benchmarks](#performance-benchmarks)
- [Installation & Deployment](#installation--deployment)
- [Configuration Reference](#configuration-reference)
- [Diagnostics & Logging](#diagnostics--logging)
- [Compilation & Building from Source](#compilation--building-from-source)
- [Repository Structure & Codebase Index](#repository-structure--codebase-index)
- [Security & Privilege Model](#security--privilege-model)
- [Documentation Index](#documentation-index)
- [Contributing](#contributing)
- [License & Attributions](#license--attributions)

---

## Executive Overview

Orca Light is a bare-metal, keyboard-driven productivity launcher engineered from the ground up for Windows 10 and Windows 11. Conceived as a high-performance, distraction-free alternative to bloated application launchers, Orca Light completely eliminates the overhead of WebViews, Chromium wrappers, Electron runtimes, .NET runtimes, and intermediate interpretation layers.

Compiled into a standalone, statically linked executable weighing approximately 1.6 megabytes, Orca Light interfaces directly with native Win32 system APIs, leverages hardware-accelerated Direct2D and DirectWrite for sub-millisecond glyph rendering, and executes asynchronous search and background indexing on dedicated worker threads throttled to low system priorities.

Whether launching installed programs, querying deep directory paths, evaluating mathematical formulas, tracking clipboard history, executing system commands, or converting live international currency exchange rates, Orca Light delivers immediate responsiveness with a resting memory footprint of approximately 12.4 megabytes.

---

## Design Principles

Orca Light is constructed under five non-negotiable systems engineering principles:

1. **Zero External Runtime Overhead**
   The application requires no runtime installations, no .NET Framework / .NET Core runtimes, no Visual C++ Redistributable prerequisites (via static CRT linkage), and no embedded browser engines.

2. **Strict Single-Binary Portability**
   All core dependencies, window manifests, DPI awareness policies, and graphics assets are compiled directly into the binary or accessed via standard Windows DLLs present in every Windows installation since Windows 7 SP1.

3. **Hardware-Accelerated Brutalist Aesthetics**
   The user interface is engineered for clarity, speed, and contrast. By combining Direct2D 1.1 render targets with Desktop Window Manager (DWM) alpha-compositing, Orca Light achieves fluid 60/120 FPS rendering without consuming GPU power.

4. **Kernel-Friendly Resource Management**
   Background indexing runs at `THREAD_PRIORITY_LOWEST`, incorporates startup grace periods (2,000ms delay), yields CPU slices through cooperative micro-sleeps, and avoids continuous disk thrashing.

5. **Complete Local Privacy**
   Orca Light features zero telemetry, zero analytics, zero crash-reporting outbound beacons, and zero cloud accounts. Search queries never leave your local machine.

---

## Core Subsystems & Architecture

Orca Light operates as a modular, decoupled set of subsystems communicating through thread-safe queues and Win32 message queues.

```
+-----------------------------------------------------------------------------+
|                                ORCA LIGHT                                   |
+-----------------------------------------------------------------------------+
                                       |
                +----------------------+----------------------+
                |                                             |
   [Low-Level Keyboard Hook]                      [Win32 Message Loop]
  WH_KEYBOARD_LL / RegisterHotKey                 GetMessageW / DispatchMessageW
                |                                             |
                v                                             v
     +--------------------+                       +-----------------------+
     | Hotkey Coordinator |---------------------->| MainWindow Controller |
     +--------------------+                       +-----------------------+
                                                              |
                 +--------------------------------------------+
                 |
                 +--> [Direct2D Render Pipeline]
                 |    ID2D1HwndRenderTarget + IDWriteFactory (ClearType / Alpha Blended)
                 |
                 +--> [Query Dispatcher]
                      |
                      +---> [App & Path Indexer] (Cached Start Menu / Path Executables)
                      +---> [Asynchronous File Indexer] (Background DFS / Memory Mapped)
                      +---> [Fuzzy Matcher] (Bitap / Modified Smith-Waterman Heuristic)
                      +---> [Math Calculation Engine] (Shunting-Yard Infix Evaluator)
                      +---> [Live Currency Converter] (WinInet Async Worker Thread)
                      +---> [Clipboard History Ring] (WM_CLIPBOARDUPDATE Monitor)
                      +---> [System Command Executor] (ShellExecuteExW / Advapi32)
```

For comprehensive deep dives into internal memory layouts, message pumps, and thread synchronization, refer to [ARCHITECTURE.md](ARCHITECTURE.md) and [docs/ARCHITECTURE_INTERNALS.md](docs/ARCHITECTURE_INTERNALS.md).

---

## Feature Matrix

### 1. Instant Application Launching
Scans and indexes standard Windows Start Menu locations (both Common and User profiles), registered system App Paths in the Windows Registry (`HKLM` and `HKCU`), Control Panel applets, and standard administrative utilities.

### 2. Deep File Indexing & Fuzzy Matching
Traverses user document trees, Desktop, Downloads, and custom configured paths. Implements an asynchronous crawler that serializes indexed records to `%LOCALAPPDATA%\Orca-Light\file_index.cache` for instant access upon subsequent boots.

### 3. Integrated Math Expression Engine
Evaluates arithmetic expressions directly from the primary input field. Supports additions, subtractions, multiplications, divisions, exponentiations (`^`), modulo (`%`), parenthetical nesting, trigonometric functions (`sin`, `cos`, `tan`), square roots (`sqrt`), absolute values (`abs`), and mathematical constants (`pi`, `e`).

### 4. Live Currency Conversion Engine
Parses currency conversion patterns such as `100 USD to EUR`, `$50 in INR`, `2500 JPY to GBP`, or `€75 CAD`. Queries live exchange rates in the background via WinInet and caches responses locally with automatic offline fallback.

### 5. Native Win32 Clipboard Manager
Monitors Windows clipboard updates via `AddClipboardFormatListener`. Maintains a ring buffer of recent text entries. Press `Ctrl + Shift + V` or type `cb` / `@clip` to inspect, search, and paste historic clipboard records.

### 6. Built-in System Automation
Execute rapid system operations without launching cmd.exe or PowerShell:
- `> shutdown`: Gracefully powers down the operating system.
- `> restart`: Reboots Windows.
- `> lock`: Locks the current user workstation via `LockWorkStation`.
- `> sleep`: Places the system into suspended low-power state.
- `> empty recycle bin`: Purges deleted files via `SHEmptyRecycleBinW`.
- `> task manager`: Launches `taskmgr.exe` directly.
- `> settings`: Launches the Windows Settings hub.
- `> exit`: Quits Orca Light immediately.

---

## Keyboard Navigation Reference

| Shortcut | Context | Functional Behavior |
| :--- | :--- | :--- |
| `Win + Space` | Global | Summon or dismiss Orca Light (Primary hotkey) |
| `Ctrl + Space` | Global | Summon or dismiss Orca Light (Fallback secondary hotkey) |
| `Ctrl + Shift + V` | Global / Local | Open clipboard history manager directly |
| `Up Arrow` / `Down Arrow` | Results Active | Navigate highlighted selection up or down |
| `Page Up` / `Page Down` | Results Active | Jump selection by 5 items |
| `Enter` | Item Selected | Execute highlighted item, open file, or copy calculation |
| `Ctrl + Enter` | File Selected | Reveal target item in Windows File Explorer |
| `Ctrl + Shift + Enter` | App Selected | Run target application with elevated Administrator rights |
| `Ctrl + Shift + C` | Result Selected | Copy absolute path or evaluated result to clipboard |
| `Escape` | Window Active | Clear query input, or dismiss launcher window |

---

## Performance Benchmarks

All microbenchmarks recorded on an AMD Ryzen 7 5800X (8 cores / 16 threads @ 3.8 GHz), 32 GB DDR4-3600 RAM, Windows 11 Pro 64-bit (Build 22631), Samsung 980 Pro NVMe SSD.

| Benchmark Metric | Orca Light | PowerToys Run | Flow Launcher | Wox Launcher | Everything Run |
| :--- | :--- | :--- | :--- | :--- | :--- |
| **Cold Startup Time** | **18 ms** | 420 ms | 650 ms | 780 ms | 45 ms |
| **Keystroke-to-Paint Latency** | **< 3 ms** | ~45 ms | ~38 ms | ~52 ms | ~8 ms |
| **Resting Working Set RAM** | **~12.4 MB** | ~185 MB | ~95 MB | ~110 MB | ~28 MB |
| **Active Search RAM** | **~16.2 MB** | ~240 MB | ~130 MB | ~145 MB | ~34 MB |
| **Binary Package Size** | **1.6 MB** | ~220 MB | ~85 MB | ~45 MB | ~3.8 MB |
| **Runtime Dependencies** | **None (Native)** | .NET 8 / WinUI 3 | .NET Desktop | .NET Framework | None (Native) |
| **UI Rendering Engine** | **Direct2D Hardware** | XAML Islands | WPF | WPF | GDI / Native |

Detailed testing methodology and replication procedures are documented in [BENCHMARKS.md](BENCHMARKS.md).

---

## Installation & Deployment

Orca Light is engineered for both portable use and enterprise silent installation.

### Option 1: Automated Script Installation (Recommended)
1. Download the latest `Orca-Light-Windows-x64.zip` release archive.
2. Extract the archive completely to any directory.
3. Double-click `install.bat`.
   - Strips the Windows SmartScreen `Zone.Identifier` alternate data stream.
   - Installs files into `%LOCALAPPDATA%\Orca-Light`.
   - Configures automatic Windows startup via `HKCU\Software\Microsoft\Windows\CurrentVersion\Run`.
   - Generates a clean Start Menu shortcut.
   - Launches the process immediately.

### Option 2: Portable Execution
1. Extract `Orca-Light-Windows-x64.zip`.
2. Double-click `run.bat` or launch `Orca-Light.exe` directly.
3. Press `Win + Space` or `Ctrl + Space` to summon the interface.

### Option 3: Clean Uninstallation
1. Double-click `uninstall.bat`.
2. The script terminates active instances, unregisters registry autostart keys, deletes the Start Menu shortcut, and purges the installation directory.

---

## Configuration Reference

Orca Light reads its settings from `config.ini`, located in the application directory (or `%LOCALAPPDATA%\Orca-Light\config.ini` in installed mode).

```ini
; Orca Light Configuration File
; Ultra-lightweight native Windows launcher

[General]
hotkey=Win + Space
autostart=true
max_results=9
max_clipboard_items=50

[Directories]
; Add custom paths separated by new lines or standard semi-colon delimiters
; Example: dir1=D:\Projects

[Excludes]
exclude1=.git
exclude2=.svn
exclude3=node_modules
exclude4=AppData\Local\Temp
exclude5=AppData\Local\Microsoft
exclude6=$Recycle.Bin
exclude7=Windows\WinSxS
exclude8=System Volume Information
exclude9=__pycache__
exclude10=.vscode
exclude11=.idea
```

For the complete schema, available flags, and custom directory tokens, refer to [CONFIGURATION.md](CONFIGURATION.md).

---

## Diagnostics & Logging

Orca Light writes internal startup and diagnostic information to:
```
%LOCALAPPDATA%\Orca-Light\orca-light.log
```

Log messages record:
- Timestamped initialization sequences.
- Hotkey registration status and fallback bindings.
- Thread startup and indexer completion statistics.
- Hardware render target creation and adapter capabilities.

---

## Compilation & Building from Source

### Prerequisites
- Windows 10/11 x64 (or a Linux cross-compilation environment).
- Either:
  - Microsoft Visual Studio 2022 (MSVC v143 toolchain) with C++20 support.
  - Or MinGW-w64 (`x86_64-w64-mingw32-g++` supporting C++20).
  - Optional: CMake 3.20 or newer.

### Build with Microsoft Visual Studio (cl.exe / build.bat)
Open the **x64 Native Tools Command Prompt for VS 2022** and execute:
```cmd
build.bat
```

### Build with MinGW-w64
```bash
# Compile resources
windres -i res/resource.rc -o res/resource.o --target=pe-x86-64

# Compile static executable
x86_64-w64-mingw32-g++ -std=c++20 -O3 -mwindows -municode \
  -DUNICODE -D_UNICODE -DWIN32_LEAN_AND_MEAN -DNOMINMAX \
  -Isrc -Ires \
  src/main.cpp src/app.cpp \
  src/calculator/calc_engine.cpp src/calculator/currency_converter.cpp \
  src/clipboard/clipboard_history.cpp src/config/config_manager.cpp \
  src/indexer/app_indexer.cpp src/indexer/file_indexer.cpp \
  src/search/fuzzy_matcher.cpp src/search/search_engine.cpp \
  src/system/system_commands.cpp src/ui/renderer.cpp src/ui/window.cpp \
  src/utils/icon_loader.cpp src/utils/logger.cpp src/utils/shell_utils.cpp \
  src/utils/string_utils.cpp res/resource.o \
  -luuid -ld2d1 -ldwrite -lwindowscodecs -ldxgi -lshlwapi -lshell32 \
  -lole32 -loleaut32 -ladvapi32 -luser32 -lgdi32 -lpropsys -ldwmapi -lwininet \
  -static -static-libgcc -static-libstdc++ -Wl,-Bstatic -lstdc++ -lpthread -Wl,-Bdynamic -s -o Orca-Light.exe
```

### Build with CMake
```bash
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build . --config Release
```

---

## Repository Structure & Codebase Index

```
orca-light/
|-- .github/
|   |-- workflows/
|   |   |-- ci.yml                      # Continuous integration matrix
|   |   `-- release.yml                 # Automated release packager
|   |-- ISSUE_TEMPLATE/
|   |   |-- bug_report.yml              # Bug report schema
|   |   |-- config.yml                  # Issue template coordinator
|   |   `-- feature_request.yml         # Feature proposal schema
|   |-- CODEOWNERS                      # Repository code ownership
|   `-- PULL_REQUEST_TEMPLATE.md        # Pull request guidelines
|-- docs/
|   |-- INDEX.md                        # Master documentation index
|   |-- ARCHITECTURE_INTERNALS.md       # Systems deep-dive
|   |-- DIRECT2D_RENDER_PIPELINE.md     # Graphics subsystem specifications
|   |-- FUZZY_SEARCH_ALGORITHM.md       # Search scoring heuristics
|   |-- INDEXER_SUBSYSTEM.md            # Background crawler documentation
|   |-- KEYBOARD_SHORTCUTS.md           # Exhaustive shortcut matrix
|   `-- SECURITY_MODEL.md               # Security and trust boundaries
|-- res/
|   |-- icon.ico                        # High-resolution multi-layer icon
|   |-- orca_light.manifest             # DPI, OS compatibility, Common Controls manifest
|   |-- resource.h                      # Resource header definitions
|   `-- resource.rc                     # Windows resource script
|-- src/
|   |-- calculator/
|   |   |-- calc_engine.cpp             # Shunting-yard expression evaluator
|   |   |-- calc_engine.h               # Calculator declarations
|   |   |-- currency_converter.cpp      # Live currency converter with WinInet
|   |   `-- currency_converter.h        # Currency converter headers
|   |-- clipboard/
|   |   |-- clipboard_history.cpp       # Native clipboard listener and ring buffer
|   |   `-- clipboard_history.h         # Clipboard history interface
|   |-- config/
|   |   |-- config_manager.cpp          # INI parser and configuration coordinator
|   |   `-- config_manager.h            # Configuration structure
|   |-- indexer/
|   |   |-- app_indexer.cpp             # Start Menu, Registry, and UWP crawler
|   |   |-- app_indexer.h               # App indexer definitions
|   |   |-- file_indexer.cpp            # Low-priority DFS file crawler & cache
|   |   `-- file_indexer.h              # File indexer definitions
|   |-- search/
|   |   |-- fuzzy_matcher.cpp           # Bitap / modified Smith-Waterman matching
|   |   |-- fuzzy_matcher.h             # Fuzzy scoring algorithms
|   |   |-- search_engine.cpp           # Aggregated multi-index search query engine
|   |   `-- search_engine.h             # Search coordinator headers
|   |-- system/
|   |   |-- system_commands.cpp         # Native Windows maintenance dispatcher
|   |   `-- system_commands.h           # System command definitions
|   |-- ui/
|   |   |-- renderer.cpp                # Direct2D 1.1 double-buffered compositor
|   |   |-- renderer.h                  # Direct2D interfaces and brushes
|   |   |-- theme.h                     # Brutalist palette and color tokens
|   |   |-- window.cpp                  # Win32 window message procedure and DWM
|   |   `-- window.h                    # MainWindow declarations
|   |-- utils/
|   |   |-- icon_loader.cpp             # Shell icon extraction via IExtractIconW
|   |   |-- icon_loader.h               # Icon loader declarations
|   |   |-- logger.cpp                  # Thread-safe diagnostic file logger
|   |   |-- logger.h                    # Logger interface
|   |   |-- shell_utils.cpp             # ShellExecuteExW process spawn helpers
|   |   |-- shell_utils.h               # Shell utility declarations
|   |   |-- string_utils.cpp            # UTF-8 / UTF-16 wide string transformers
|   |   `-- string_utils.h              # String conversion utilities
|   |-- app.cpp                         # Single-instance mutex, worker coordination
|   |-- app.h                           # Application coordinator interface
|   `-- main.cpp                        # Win32 wWinMain entry point
|-- .clang-format                       # C++ code formatting standard
|-- .editorconfig                       # Workspace formatting rules
|-- .gitattributes                      # Git line ending configuration
|-- .gitignore                          # Build artifact ignore specifications
|-- ARCHITECTURE.md                     # System architecture specification
|-- BENCHMARKS.md                       # Performance whitepaper
|-- CHANGELOG.md                        # Version history and release notes
|-- CITATION.cff                        # Academic and software citation metadata
|-- CMakeLists.txt                      # Multi-platform CMake configuration
|-- CODE_OF_CONDUCT.md                  # Contributor Covenant Code of Conduct
|-- CONFIGURATION.md                    # Exhaustive INI parameter reference
|-- CONTRIBUTING.md                     # Contribution guidelines and coding standard
|-- LICENSE                             # MIT License
|-- README.md                           # Primary project documentation
|-- README.txt                          # Quick portable release guide
|-- SECURITY.md                         # Security policy and disclosure
|-- build.bat                           # Visual Studio / MSVC build orchestrator
|-- config.ini                          # Default configuration template
|-- install.bat                         # Automated installation script
|-- package.sh                          # Portable release archive bundler
|-- run.bat                             # Instant portable launcher script
`-- uninstall.bat                       # Automated uninstallation script
```

---

## Security & Privilege Model

Orca Light enforces strict privilege boundaries:
- **Zero Elevation by Default**: The application runs under the unprivileged standard user token (`asInvoker`).
- **Explicit Elevation via Modifier**: Executables are launched with administrative privileges only when the user explicitly triggers `Ctrl + Shift + Enter`.
- **Mark of the Web (MOTW) Handling**: The installation script actively removes `Zone.Identifier` ADS to prevent false-positive silent blocks from SmartScreen on downloaded archives.
- **Local Network Scope**: Network traffic is strictly limited to currency rate synchronization over HTTPS using standard Windows `WinInet`. No search queries, clipboard items, or indexing data are ever transmitted.

---

## Documentation Index

- [Master Documentation Index](docs/INDEX.md)
- [System Architecture](ARCHITECTURE.md)
- [Architecture Internals](docs/ARCHITECTURE_INTERNALS.md)
- [Configuration Reference](CONFIGURATION.md)
- [Performance Benchmarks](BENCHMARKS.md)
- [Fuzzy Search Algorithm](docs/FUZZY_SEARCH_ALGORITHM.md)
- [Direct2D Render Pipeline](docs/DIRECT2D_RENDER_PIPELINE.md)
- [Indexer Subsystem](docs/INDEXER_SUBSYSTEM.md)
- [Keyboard Shortcuts](docs/KEYBOARD_SHORTCUTS.md)
- [Security Model](docs/SECURITY_MODEL.md)
- [Contributing Guidelines](CONTRIBUTING.md)
- [Code of Conduct](CODE_OF_CONDUCT.md)
- [Security Policy](SECURITY.md)
- [Changelog](CHANGELOG.md)

---

## Contributing

Contributions are welcomed. Please read [CONTRIBUTING.md](CONTRIBUTING.md) and [CODE_OF_CONDUCT.md](CODE_OF_CONDUCT.md) before submitting pull requests. All contributions must adhere to the zero-external-dependency rule and pass MSVC and MinGW-w64 builds cleanly.

---

## License & Attributions

Orca Light is released under the **MIT License**. See [LICENSE](LICENSE) for details.

Built with native C++20 and bare-metal Win32.
