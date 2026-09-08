<div align="center">
  <img src="res/icon.ico" width="128" height="128" alt="Orca-Light Logo">
  <h1>Orca-Light</h1>
  <p><strong>A blazing fast, native Win32, brutalist Spotlight alternative for Windows.</strong></p>

  [![License: MIT](https://img.shields.io/badge/License-MIT-blue.svg)](https://opensource.org/licenses/MIT)
  [![C++20](https://img.shields.io/badge/C++-20-blue.svg)](https://isocpp.org/)
  [![Platform](https://img.shields.io/badge/Platform-Windows%2010%2F11-lightgrey.svg)]()
  [![Build](https://img.shields.io/badge/Build-MinGW--w64-green.svg)]()
  [![Zero Bloat](https://img.shields.io/badge/Bloat-0%25-success.svg)]()
</div>

<br/>

## 🐋 Overview

**Orca-Light** is an ultra-lightweight, native C++ application launcher and productivity tool for Windows. Inspired by macOS Spotlight but built with the speed and philosophy of bare-metal Win32, Orca-Light ditches Electron, WebView2, and heavy frameworks in favor of pure performance. 

It features a brutalist, keyboard-first UI with hardware-accelerated Direct2D graphics, a custom real-time fuzzy-search engine, offline asynchronous file indexing, and a live background currency converter—all compiled into a single, standalone executable that consumes near-zero system resources.

---

## ⚡ Features

*   **🏎️ Blazing Fast Launch**: Instantaneous invocation via global hotkeys (`Win + Space` or `Ctrl + Space`) with zero visual lag.
*   **🪶 Zero Bloat**: No HTML/JS, no .NET, no Electron. Written in pure C++20.
*   **🔍 Fuzzy Search & File Indexing**: Instantly search for files, applications, and paths. A custom-built asynchronous indexer scans your drives seamlessly in the background (using `THREAD_PRIORITY_LOWEST` to ensure zero impact on low-end systems).
*   **🧮 Smart Calculator**: Type natural math expressions (`(15 * 40) / 2`) directly into the search bar.
*   **💱 Live Currency Conversion**: Real-time currency conversions using an offline/async cache. Type `$20 in inr`, `100 EUR to USD`, or `£50 JPY` and instantly get exact live exchange rates.
*   **📋 Clipboard History**: Press `Ctrl + Shift + V` to access a lightweight, native clipboard manager.
*   **💻 System Commands**: Built-in commands like `sleep`, `restart`, `shutdown`, `lock`, `empty trash`, and more.
*   **🎨 Brutalist UI & Tinted Glass**: Features a gorgeous, minimalist dark-mode interface utilizing Direct2D hardware acceleration and a sleek alpha-blended "Tinted Glass" composite effect.

---

## 🛠️ Architecture & Tech Stack

*   **Language**: Modern C++ (C++20)
*   **Graphics**: Direct2D & DirectWrite (Hardware Accelerated)
*   **System APIs**: Pure Win32 API
*   **Threading**: Native Win32 Threads (`CreateThread`) with thread-safe atomic queues to avoid MinGW `libwinpthread` static linking vulnerabilities.
*   **Networking**: Native `WinInet` API for background currency rate fetching.
*   **Compiler**: `x86_64-w64-mingw32-g++`

> **Note on Performance Optimization**: Orca-Light utilizes highly tuned execution flows. Background indexing halts for 2,000ms upon initialization to prevent UI thread contention, dynamically yields CPU cycles via micro-sleeps, and avoids invoking expensive `GetAsyncKeyState` system interrupts within the low-level `WH_KEYBOARD_LL` hook.

---

## 🚀 Installation & Usage

Orca-Light is designed to be fully portable.

### Method 1: Portable Script (Recommended)
1. Download the latest `Orca-Light-Windows-x64.zip` release.
2. Extract the archive to any folder.
3. Double click `run.bat` or `install.bat`. 
   > *Note: The batch scripts natively strip the Windows SmartScreen "Mark of the Web" Alternate Data Stream (`Zone.Identifier`) to prevent silent blocks.*

### Method 2: Manual
1. Download and extract the archive.
2. Run `Orca-Light.exe`.
3. Press `Win + Space` or `Ctrl + Space` to summon the launcher.

---

## ⌨️ Keyboard Shortcuts

| Shortcut | Action |
| :--- | :--- |
| `Win + Space` / `Ctrl + Space` | Toggle Orca-Light Launcher |
| `Up / Down Arrows` | Navigate search results |
| `Enter` | Launch application, open file, or copy calculator result |
| `Esc` | Hide launcher |

---

## 🏗️ Compiling from Source

If you want to build Orca-Light from scratch, you will need the MinGW-w64 toolchain.

```bash
# Clone the repository
git clone https://github.com/YOUR_USERNAME/orca-light.git
cd orca-light

# Compile the executable statically
x86_64-w64-mingw32-g++ -std=c++20 -O3 -mwindows -municode \
  -DUNICODE -D_UNICODE -DWIN32_LEAN_AND_MEAN -DNOMINMAX \
  -Isrc -Ires \
  src/main.cpp src/app.cpp src/calculator/calc_engine.cpp src/calculator/currency_converter.cpp \
  src/clipboard/clipboard_history.cpp src/config/config_manager.cpp \
  src/indexer/app_indexer.cpp src/indexer/file_indexer.cpp \
  src/search/fuzzy_matcher.cpp src/search/search_engine.cpp \
  src/system/system_commands.cpp src/ui/renderer.cpp src/ui/window.cpp \
  src/utils/icon_loader.cpp src/utils/shell_utils.cpp src/utils/string_utils.cpp \
  src/utils/logger.cpp res/resource.o \
  -luuid -ld2d1 -ldwrite -lwindowscodecs -ldxgi -lshlwapi -lshell32 -lole32 -loleaut32 -ladvapi32 -luser32 -lgdi32 -lpropsys -ldwmapi -lwininet \
  -static -static-libgcc -static-libstdc++ -Wl,-Bstatic -lstdc++ -lpthread -Wl,-Bdynamic -s -o Orca-Light.exe
```

---

## 🤝 Contributing

Contributions are heavily encouraged! Please review the [CONTRIBUTING.md](CONTRIBUTING.md) file for guidelines on how to open issues, submit pull requests, and follow our brutalist design philosophy. 

---

## 📄 License

This project is licensed under the MIT License. See the [LICENSE](LICENSE) file for details.

<br/>
<div align="center">
  <sub>Built with pure C++ and Win32. 🖤</sub>
</div>
