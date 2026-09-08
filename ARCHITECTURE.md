# Orca Light System Architecture & Design Specification

This document provides a detailed technical specification of the Orca Light architecture, subsystem interactions, memory management policies, and threading model.

---

## 1. High-Level Architectural Topology

Orca Light operates as an event-driven Win32 application centered around a lightweight UI thread, isolated worker threads, and hardware-accelerated Direct2D rendering.

```
+------------------------------------------------------------------------------------+
|                                    MAIN THREAD                                     |
|                                                                                    |
|  +------------------------------------------------------------------------------+  |
|  |                          Win32 Window Message Loop                           |  |
|  |                (GetMessageW / TranslateMessage / DispatchMessageW)           |  |
|  +------------------------------------------------------------------------------+  |
|         ^                     |                              |                     |
|         |                     |                              |                     |
|  [LL Keyboard Hook]           v                              v                     |
|  WH_KEYBOARD_LL        [Event Router]            [Direct2D Render Pipeline]        |
|  WM_HOTKEY fallback           |                  ID2D1HwndRenderTarget            |
|                               v                  Double-Buffered Compositor        |
|                    +----------------------+                                        |
|                    | SearchEngine Router  |                                        |
|                    +----------------------+                                        |
+-------------------------------|----------------------------------------------------+
                                | (Thread-Safe Query Dispatch)
        +-----------------------+-----------------------+
        |                                               |
        v                                               v
+-------------------------------+               +-------------------------------+
|     WORKER THREAD: INDEXER    |               |    WORKER THREAD: WININET     |
|                               |               |                               |
| Priority:                     |               | Priority:                     |
| THREAD_PRIORITY_LOWEST        |               | THREAD_PRIORITY_BELOW_NORMAL  |
|                               |               |                               |
| Functionality:                |               | Functionality:                |
| - 2,000ms Startup Deferral    |               | - Currency Rate Sync          |
| - Recursive DFS Traversal     |               | - Non-blocking HTTPS GET      |
| - Micro-Sleep Yielding        |               | - Atomic In-Memory Table      |
| - Binary Cache Serialization  |               | - Offline Fallback Cache      |
+-------------------------------+               +-------------------------------+
```

---

## 2. Process Lifecycle & Single-Instance Enforcement

### 2.1 Mutex Synchronization
Orca Light uses a named Win32 mutex to enforce a strict single-instance invariant:
```cpp
constexpr const wchar_t* MUTEX_NAME = L"Local\\Orca-Light_Launcher_SingleInstance";
HANDLE hMutex = CreateMutexW(nullptr, TRUE, MUTEX_NAME);
if (GetLastError() == ERROR_ALREADY_EXISTS) {
    // Another instance is active. Signal existing window to activate.
    HWND existingHwnd = FindWindowW(L"Orca-LightWindowClass", L"Orca-Light");
    if (existingHwnd) {
        PostMessageW(existingHwnd, WM_USER + 101, 0, 0);
    }
    return 0;
}
```
This guarantees that secondary executions (such as accidental multiple clicks on `run.bat` or shortcuts) cleanly yield foreground focus to the already running daemon process.

### 2.2 Startup Grace Sequence
Upon launch, Orca Light initializes its internal data structures in a strictly prioritized order:
1. Initialize Win32 COM subsystem (`CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED)`).
2. Configure DPI awareness per monitor (`SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2)`).
3. Initialize the diagnostic file logger (`%LOCALAPPDATA%\Orca-Light\orca-light.log`).
4. Read configuration from `config.ini`.
5. Spawn the primary Win32 window (`WS_EX_TOOLWINDOW | WS_EX_TOPMOST | WS_EX_LAYERED`).
6. Register global hotkeys (`RegisterHotKey` with fallback to `WH_KEYBOARD_LL`).
7. Spawn the asynchronous file indexer thread with a deferred start of 2,000ms.
8. Spawn the currency conversion sync thread.
9. Enter the primary message pump.

---

## 3. Window Management & Desktop Compositing

### 3.1 Window Attributes
Orca Light creates a borderless window configured with:
- `WS_POPUP`: Strips standard title bars, borders, and window frames.
- `WS_EX_TOOLWINDOW`: Prevents the launcher from cluttering the Alt+Tab switcher or taskbar.
- `WS_EX_TOPMOST`: Ensures the launcher appears above all full-screen or modal desktop windows.
- `WS_EX_LAYERED`: Enables hardware alpha-blending and per-pixel Direct2D transparency.

### 3.2 DWM Blur-Behind & Tinted Glass
To avoid CPU-bound software blurs, Orca Light calls the Desktop Window Manager blur-behind interface:
```cpp
DWM_BLURBEHIND bb = {0};
bb.dwFlags = DWM_BB_ENABLE;
bb.fEnable = TRUE;
bb.hRgnBlur = nullptr;
DwmEnableBlurBehindWindow(hwnd_, &bb);
```
Direct2D then draws an ultra-dark alpha-blended acrylic layer (`rgba(16, 16, 18, 0.94)`) with an inner 1-pixel border (`rgba(255, 255, 255, 0.08)`), yielding a modern brutalist aesthetic at 0% GPU load.

### 3.3 Focus & Activation Mechanics
To prevent accidental dismissals during window switching:
- A 350ms activation grace timer (`TIMER_ACTIVATION_GRACE`) suppresses spurious `WM_KILLFOCUS` events generated immediately upon window creation.
- Clicking anywhere outside the launcher boundary dispatches `ShowWindow(hwnd_, SW_HIDE)`.
- Re-summoning the window resets the query buffer and restores selected index to 0.

---

## 4. Input Subsystem & Hotkey Routing

### 4.1 Hybrid Hotkey Architecture
Orca Light implements a dual hotkey interception pipeline:
1. **`RegisterHotKey` Engine**: Attempts standard registration for `MOD_WIN | MOD_NOREPEAT, VK_SPACE`. If a registered shell component (such as Windows default IME or Cortana) holds the key, the engine gracefully falls back to `MOD_CONTROL, VK_SPACE`.
2. **`WH_KEYBOARD_LL` Low-Level Hook**: If `RegisterHotKey` encounters conflicts, a low-level keyboard hook monitors `WM_KEYDOWN` and `WM_SYSKEYDOWN`. To maintain high system responsiveness, the hook procedure executes zero disk I/O, does not call `GetAsyncKeyState`, and forwards state transitions asynchronously via `PostMessageW`.

---

## 5. Direct2D Rendering Pipeline

### 5.1 Factory & Target Lifecycle
Orca Light uses Direct2D 1.1 and DirectWrite:
- `ID2D1Factory`: Initialized once per process (`D2D1_FACTORY_TYPE_SINGLE_THREADED`).
- `ID2D1HwndRenderTarget`: Bound directly to the window HWND with `D2D1_RENDER_TARGET_TYPE_DEFAULT` and `D2D1_PRESENT_OPTIONS_IMMEDIATELY`.
- `IDWriteFactory`: Constructs text formats for the search query (22pt, Light/Regular) and result items (14pt, Regular/SemiBold).

### 5.2 Device Loss Recovery
If the graphics driver resets or display resolution changes (`D2DERR_RECREATE_TARGET`), Orca Light safely discards all cached brushes, recreates the HWND render target, and redraws the current frame without process termination.

### 5.3 Frame Drawing Sequence
Each render call executes in a single pass:
1. `BeginDraw()`
2. `Clear(rgba(16, 16, 18, 0.94))`
3. Draw window border rectangle (1px solid, `#2A2A2E`)
4. Draw search input box and placeholder/query text
5. Draw animated blinking caret (`1.5px` width)
6. Loop through visible results (`max_results = 9`):
   - Draw selection background pill for active index (`#222226` with accent edge)
   - Draw cached shell icon (32x32) via Direct2D bitmap converter
   - Draw primary title text (`#FFFFFF`)
   - Draw secondary subtitle / path text (`#8E8E93`)
7. `EndDraw()`

---

## 6. Search & Fuzzy Matching Subsystem

### 6.1 Unified Search Query Router
Incoming queries are classified instantly:
1. **Math Expression**: If the query matches mathematical syntax (e.g., contains numbers and operators `+`, `-`, `*`, `/`, `^`, `sqrt`), the math engine processes it first.
2. **Currency Conversion**: If the query matches currency keywords (e.g., `in`, `to`, `$`, `EUR`), the currency converter evaluates the conversion.
3. **System Command**: If the query begins with `>` or matches keywords (`shutdown`, `lock`, `restart`), system commands receive high priority.
4. **Clipboard Search**: If the query begins with `cb` or `@clip`, the clipboard manager receives the query.
5. **App & File Search**: The query is broadcast across indexed applications and file records through the fuzzy matcher.

### 6.2 Fuzzy Scoring Mechanics
Orca Light's fuzzy matching engine computes match confidence using an optimized scoring model:
- **Base Score**: 0
- **Exact Case Match**: +100 bonus
- **Exact Prefix Match**: +500 bonus
- **Word Boundary Match**: +250 bonus (characters immediately following spaces, dots, hyphens, or underscores)
- **CamelCase Transition Match**: +200 bonus
- **Contiguous Run Bonus**: +150 per sequential matched character
- **Path Depth Penalty**: -10 per path hierarchy level to favor root items over deeply nested files

---

## 7. Asynchronous File Indexer

### 7.1 Thread Priority & Throttling
The file indexer thread runs under:
```cpp
SetThreadPriority(hThread, THREAD_PRIORITY_LOWEST);
```
To ensure that background indexing does not affect gaming, compilation, or general UI fluidity, the indexer:
- Pauses for 2,000ms after startup.
- Sleeps for 2ms every 100 traversed directories.
- Skips known heavy directories (`node_modules`, `.git`, `WinSxS`, `AppData\Local\Temp`).

### 7.2 Cache Serialization Format
The crawler saves its index state to `%LOCALAPPDATA%\Orca-Light\file_index.cache`. Upon restart, Orca Light memory-maps and deserializes this cache in under 8 milliseconds, ensuring instant file search before any background indexing pass completes.

---

## 8. Memory Management & Safety Guarantees

- **Zero Dynamic Allocation on Keystrokes**: Query buffers use pre-allocated static arrays and fixed-size result containers (`std::vector<SearchResult>` with pre-allocated capacity).
- **RAII COM Wrappers**: DirectX and Shell interfaces are managed via `ATL::CComPtr` or custom smart wrappers, preventing resource leaks.
- **Exception Safety**: All core Win32 system interop calls are guarded with standard C++ `noexcept` specifications and structured exception boundaries.
