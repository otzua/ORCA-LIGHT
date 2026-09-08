# Contributing to Orca Light

Thank you for your interest in contributing to Orca Light. This document outlines the standards, workflow, and guidelines for proposing changes, opening issues, and submitting pull requests.

---

## 1. Core Engineering Philosophy

Orca Light is engineered with strict constraints. All contributions must adhere to these non-negotiable principles:

1. **Zero External Runtime Dependencies**
   The project must compile to a standalone native Windows binary without requiring .NET runtimes, WebView2, Electron, or external DLL runtimes. Use native Win32 APIs and standard C++20.
2. **Minimal CPU & Memory Footprint**
   Every CPU cycle and allocation matters. Hot code paths (such as keystroke dispatch and frame rendering) must perform zero dynamic memory allocation. Background workers must run at `THREAD_PRIORITY_LOWEST` and yield cooperatively.
3. **Brutalist, High-Contrast Aesthetics**
   Avoid bloated UI frameworks. The visual design is strictly minimal, dark-mode, hardware-accelerated via Direct2D, and keyboard-first.
4. **Zero Telemetry & Local Execution**
   No user data, search strings, or analytics may ever be transmitted across the network. Network usage is strictly restricted to currency rate caching over HTTPS.
5. **No Emojis Anywhere**
   Never use emojis in UI copy, button labels, icons, code comments, commit messages, or documentation. Use standard text, custom SVGs, or Win32 glyph fonts.

---

## 2. Development Environment Setup

### 2.1 Toolchain Requirements
- Windows 10 or Windows 11 (64-bit)
- Either:
  - **MSVC Toolchain**: Visual Studio 2022 (Community, Professional, or Enterprise) with Desktop Development with C++ (v143 toolchain).
  - Or **MinGW-w64**: GCC 11+ with C++20 support (`x86_64-w64-mingw32-g++`).
- Optional: CMake 3.20+

### 2.2 Building Locally
Using the provided automated build script in the x64 Native Tools Command Prompt:
```cmd
build.bat
```

Using CMake:
```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
```

---

## 3. Code Standards & Style

Orca Light follows modern C++20 conventions:

- **Standard**: C++20 (`/std:c++20` or `-std=c++20`).
- **Formatting**: Adhere to `.clang-format` (4 spaces indentation, no tabs, 100 column limit).
- **Naming Conventions**:
  - Types / Classes / Structs: `PascalCase` (e.g. `MainWindow`, `SearchEngine`, `ConfigManager`).
  - Functions / Methods: `snake_case` (e.g. `center_on_active_monitor()`, `evaluate_expression()`).
  - Member Variables: `snake_case_` with a trailing underscore (e.g. `hwnd_`, `anim_progress_`, `is_indexing_`).
  - Constants / Enums: `ALL_CAPS` (e.g. `TIMER_ACTIVATION_GRACE`, `MUTEX_NAME`).
  - Namespaces: `snake_case` (e.g. `orca_light::search`, `orca_light::ui`).
- **Memory Management**:
  - Prefer value semantics and stack allocation.
  - Manage COM interfaces using smart pointers (`CComPtr` or custom RAII wrappers).
  - Never call raw `new` or `delete`; use standard containers with pre-reserved capacities.
- **Error Handling**:
  - Use `HRESULT` checks (`SUCCEEDED` / `FAILED`) for COM interfaces.
  - Check Win32 error codes (`GetLastError()`) and log failures via `utils::Logger`.
  - Avoid throwing exceptions across Win32 window callback boundaries.

---

## 4. Submitting Pull Requests

1. **Fork the repository** on GitHub.
2. **Create a topic branch** from `main`:
   ```bash
   git checkout -b feature/your-feature-name
   ```
3. **Commit your changes**:
   - Write clear, imperative commit messages (e.g. `Add trigonometric functions to math engine`).
   - Do NOT include emojis in commit messages.
4. **Verify your build**:
   - Verify that the code compiles cleanly under MSVC and MinGW-w64 with zero warnings.
   - Verify that memory consumption does not regress.
5. **Open a Pull Request**:
   - Complete the PR template in full.
   - Link any related issues or discussions.
   - Be responsive to code review feedback.
