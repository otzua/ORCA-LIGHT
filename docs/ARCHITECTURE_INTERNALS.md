# Subsystem Architecture & Internals

This specification provides an in-depth exploration of the internal subsystems powering Orca Light.

---

## 1. Win32 Threading & Concurrency Model

Orca Light maintains a strict decoupling between the user interface thread and background processing tasks to guarantee zero UI frame drops:

```
[Hardware Input]
       |
       v
+------------------+         PostMessageW          +------------------+
| Low-Level Hook   | ----------------------------> | UI Thread        |
| (WH_KEYBOARD_LL) |                               | (GetMessageW)    |
+------------------+                               +------------------+
                                                            |
                                             +--------------+--------------+
                                             |                             |
                                             v                             v
                                  +--------------------+       +--------------------+
                                  | File Indexer       |       | Currency Worker    |
                                  | Priority: LOWEST   |       | Priority: BELOW_NORM|
                                  +--------------------+       +--------------------+
```

### 1.1 UI Message Loop Execution
The primary thread spends virtually its entire lifecycle blocked inside `GetMessageW`, consuming 0.00% CPU when idle:
```cpp
MSG msg;
while (GetMessageW(&msg, nullptr, 0, 0)) {
    TranslateMessage(&msg);
    DispatchMessageW(&msg);
}
```

### 1.2 Thread Priorities
- **UI Thread**: `THREAD_PRIORITY_NORMAL`. Prioritizes responsive window redrawing and user keyboard input.
- **Background File Indexer**: `THREAD_PRIORITY_LOWEST`. Scheduled only when the CPU has idle execution capacity.
- **Currency Network Worker**: `THREAD_PRIORITY_BELOW_NORMAL`. Performs asynchronous socket reads without starving background compilation or audio tasks.

---

## 2. Window Lifecycle & Activation Management

### 2.1 Creation Parameters
The main window is registered with the following class and style configuration:
```cpp
WNDCLASSEXW wc = {sizeof(WNDCLASSEXW)};
wc.lpfnWndProc = MainWindow::window_proc_thunk;
wc.hInstance = hInstance;
wc.lpszClassName = L"Orca-LightWindowClass";
wc.hCursor = LoadCursorW(nullptr, IDC_ARROW);
RegisterClassExW(&wc);

hwnd_ = CreateWindowExW(
    WS_EX_TOOLWINDOW | WS_EX_TOPMOST | WS_EX_LAYERED,
    L"Orca-LightWindowClass",
    L"Orca-Light",
    WS_POPUP,
    x, y, width, height,
    nullptr, nullptr, hInstance, this
);
```

### 2.2 Activation Grace Period
When a user launches a full-screen application or switches windows rapidly, Windows sends sequential `WM_ACTIVATE` and `WM_KILLFOCUS` messages. To prevent Orca Light from dismissing itself immediately upon creation:
1. When `show()` is executed, a high-resolution one-shot timer (`TIMER_ACTIVATION_GRACE = 1002`) is armed for 350 milliseconds.
2. During this 350ms window, `WM_KILLFOCUS` handlers ignore blur events.
3. Once the timer fires, normal auto-dismissal on loss of focus is engaged.

---

## 3. Desktop Window Manager (DWM) Integration

### 3.1 Tinted Glass Compositing
Modern Windows desktop compositing relies on the Desktop Window Manager (DWM). Orca Light configures DWM blur-behind via `DwmEnableBlurBehindWindow`:
```cpp
DWM_BLURBEHIND bb = {0};
bb.dwFlags = DWM_BB_ENABLE;
bb.fEnable = TRUE;
bb.hRgnBlur = nullptr;
DwmEnableBlurBehindWindow(hwnd_, &bb);
```
Once enabled, Direct2D paints translucent background layers directly into the DWM surface buffer using premultiplied alpha channels. This achieves high visual contrast while offloading compositing work entirely to the GPU's fixed-function display pipeline.

---

## 4. Clipboard Ring Buffer Subsystem

### 4.1 Clip Event Interception
Orca Light uses the modern Windows Clipboard Listener API:
```cpp
AddClipboardFormatListener(hwnd_);
```
When another program copies text, Windows broadcasts `WM_CLIPBOARDUPDATE` to Orca Light. The clipboard subsystem opens the clipboard via `OpenClipboard(hwnd_)`, extracts `CF_UNICODETEXT`, verifies text validity, and prepends the record into a fixed-capacity ring buffer (`max_clipboard_items = 50`).

### 4.2 Deduplication & Sanitization
- Duplicate sequential clippings are detected and moved to the front of the queue without allocating duplicate memory.
- Pure whitespace entries and oversized text clips exceeding 64 kilobytes are filtered out to protect working set memory.
