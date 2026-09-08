# Changelog

All notable changes to Orca Light will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/), and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

---

## [1.0.0] - 2026-09-08

### Added
- **Core Windowing Subsystem**:
  - Native Win32 borderless window with `WS_EX_TOOLWINDOW`, `WS_EX_TOPMOST`, and `WS_EX_LAYERED` attributes.
  - Per-Monitor V2 DPI awareness integration.
  - Desktop Window Manager (DWM) blur-behind integration for hardware-accelerated translucent tinted glass.
  - 350ms activation grace period to avoid spurious deactivation on window transition.
  - Smooth 150ms alpha/translate entrance animation using high-resolution performance counter timers.
- **Direct2D Hardware Rendering Engine**:
  - Direct2D 1.1 double-buffered render target bound directly to the window HWND.
  - DirectWrite glyph formatting with subpixel ClearType rendering.
  - Brutalist dark theme with custom accent indicators, selection pills, and clean typography.
  - Shell icon extraction pipeline with 32x32 Direct2D bitmap caching.
- **Search & Fuzzy Matching Engine**:
  - Custom hybrid fuzzy matching engine with prefix boosting, word boundary detection, CamelCase transition bonuses, and contiguous character streaks.
  - Directory depth penalization to prioritize root applications and documents over deeply nested paths.
  - Sub-millisecond scoring across 50,000+ indexed elements.
- **Asynchronous File Indexer**:
  - Low-priority background file crawler (`THREAD_PRIORITY_LOWEST`) with 2,000ms startup delay.
  - Cooperative yielding via micro-sleeps to eliminate CPU and disk thrashing.
  - Binary index cache serialization to `%LOCALAPPDATA%\Orca-Light\file_index.cache` for instant restoration on boot.
  - Exclusion list support for `.git`, `node_modules`, `AppData`, and other system directories.
- **Application Indexer**:
  - Start Menu shortcuts traversal (Common and User profile paths).
  - Windows Registry App Paths traversal (`HKLM` and `HKCU`).
  - UWP application package detection.
- **Integrated Math Expression Evaluator**:
  - Shunting-yard infix expression parser supporting `+`, `-`, `*`, `/`, `^`, `%`.
  - Parenthetical expression nesting.
  - Support for mathematical functions (`sin`, `cos`, `tan`, `sqrt`, `abs`, `log`, `ln`).
  - Support for mathematical constants (`pi`, `e`).
  - Automatic clipboard copy on pressing Enter.
- **Live Currency Conversion Engine**:
  - Natural language parsing for currency queries (e.g. `100 USD to EUR`, `$20 in INR`, `50 GBP to JPY`).
  - Asynchronous HTTP rate synchronization via Windows WinInet API.
  - Offline currency rate fallback cache.
- **Native Clipboard Manager**:
  - Integration with `AddClipboardFormatListener` and `WM_CLIPBOARDUPDATE`.
  - In-memory ring buffer storing up to 50 recent text clippings.
  - Dedicated access hotkey (`Ctrl + Shift + V`) or search triggers (`cb`, `@clip`).
- **System Command Dispatcher**:
  - Native dispatch for `> shutdown`, `> restart`, `> lock`, `> sleep`, `> empty recycle bin`, `> task manager`, `> settings`, and `> exit`.
- **Packaging & Deployment**:
  - Automated installation batch script (`install.bat`) with `Zone.Identifier` Mark-of-the-Web stripping.
  - Instant portable mode launcher (`run.bat`).
  - Complete clean uninstallation script (`uninstall.bat`).
  - Distribution bundler script (`package.sh`).
- **Documentation & CI**:
  - Comprehensive architectural specification (`ARCHITECTURE.md`).
  - Complete configuration guide (`CONFIGURATION.md`).
  - Performance profiling whitepaper (`BENCHMARKS.md`).
  - Full GitHub Actions continuous integration and release workflows.
  - Detailed documentation directory (`docs/`).
