#include "window.h"
#include <shellapi.h>
#include <dwmapi.h>
#pragma comment(lib, "dwmapi.lib")
#include "../utils/shell_utils.h"
#include "../utils/string_utils.h"
#include "../utils/logger.h"
#include <windowsx.h>
#include <algorithm>

namespace orca_light::ui {

namespace {

constexpr UINT_PTR TIMER_CURSOR_BLINK = 1001;
constexpr UINT_PTR TIMER_ACTIVATION_GRACE = 1002;
constexpr UINT_PTR TIMER_ANIMATION = 1003;
constexpr UINT WM_TRAYICON = WM_USER + 102;
constexpr const wchar_t* WINDOW_CLASS_NAME = L"Orca-LightWindowClass";

} // namespace

MainWindow::MainWindow(search::SearchEngine& search_engine)
    : search_engine_(search_engine) {
}

MainWindow::~MainWindow() {
    if (hwnd_) {
        RemoveClipboardFormatListener(hwnd_);
        KillTimer(hwnd_, TIMER_CURSOR_BLINK);
        KillTimer(hwnd_, TIMER_ACTIVATION_GRACE);
        KillTimer(hwnd_, TIMER_ANIMATION);
        remove_tray_icon();
        DestroyWindow(hwnd_);
        hwnd_ = nullptr;
    }
}

bool MainWindow::create() {
    WNDCLASSEXW wc = {};
    wc.cbSize = sizeof(WNDCLASSEXW);
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = window_proc_static;
    wc.hInstance = GetModuleHandleW(nullptr);
    wc.hCursor = LoadCursorW(nullptr, IDC_ARROW);
    wc.hbrBackground = nullptr;
    wc.lpszClassName = WINDOW_CLASS_NAME;

    RegisterClassExW(&wc);

    float base_w = Theme::BASE_WINDOW_WIDTH;
    float base_h = Theme::BASE_SEARCH_HEIGHT + Theme::BASE_FOOTER_HEIGHT;

    hwnd_ = CreateWindowExW(
        WS_EX_TOOLWINDOW | WS_EX_TOPMOST,
        WINDOW_CLASS_NAME,
        L"Orca-Light",
        WS_POPUP,
        100, 100,
        static_cast<int>(base_w),
        static_cast<int>(base_h),
        nullptr,
        nullptr,
        GetModuleHandleW(nullptr),
        this
    );

    if (!hwnd_) {
        utils::Logger::error(L"Failed to create Orca-Light window handle.");
        return false;
    }

    utils::Logger::log("Orca-Light window created successfully.");

    // Set high DPI awareness if available
    UINT dpi = 96;
    HMODULE hUser32 = GetModuleHandleW(L"user32.dll");
    if (hUser32) {
        typedef UINT(WINAPI* GetDpiForWindowFn)(HWND);
        GetDpiForWindowFn fn = reinterpret_cast<GetDpiForWindowFn>(GetProcAddress(hUser32, "GetDpiForWindow"));
        if (fn) {
            UINT d = fn(hwnd_);
            if (d > 0) dpi = d;
        }
    }
    if (!renderer_.initialize(hwnd_)) {
        utils::Logger::error(L"Failed to initialize Direct2D graphics engine.");
        return false;
    }
    
    renderer_.update_dpi(static_cast<float>(dpi) / 96.0f);
    utils::Logger::log("Direct2D and DirectWrite initialized successfully.");

    enable_glass();
    setup_tray_icon();
    AddClipboardFormatListener(hwnd_);
    SetTimer(hwnd_, TIMER_CURSOR_BLINK, 500, nullptr);

    // Initial search with empty query to populate defaults
    perform_search();

    return true;
}

LRESULT CALLBACK MainWindow::window_proc_static(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    MainWindow* self = nullptr;

    if (msg == WM_NCCREATE) {
        CREATESTRUCTW* cs = reinterpret_cast<CREATESTRUCTW*>(lParam);
        self = reinterpret_cast<MainWindow*>(cs->lpCreateParams);
        SetWindowLongPtrW(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(self));
    } else {
        self = reinterpret_cast<MainWindow*>(GetWindowLongPtrW(hwnd, GWLP_USERDATA));
    }

    if (self) {
        return self->window_proc(hwnd, msg, wParam, lParam);
    }
    return DefWindowProcW(hwnd, msg, wParam, lParam);
}


void MainWindow::enable_glass() {
    // Completely removed expensive Acrylic / DWM Blur to optimize for low-end PCs.
    // Instead, we use simple, incredibly fast layered alpha blending.
    SetLayeredWindowAttributes(hwnd_, 0, 240, LWA_ALPHA);
}

void MainWindow::setup_tray_icon() {
    NOTIFYICONDATAW nid = {};
    nid.cbSize = sizeof(NOTIFYICONDATAW);
    nid.hWnd = hwnd_;
    nid.uID = 1;
    nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
    nid.uCallbackMessage = WM_TRAYICON;
    // Load a default system icon if custom is not available
    nid.hIcon = LoadIconW(nullptr, IDI_APPLICATION); 
    wcscpy_s(nid.szTip, L"Orca-Light (Running)");
    Shell_NotifyIconW(NIM_ADD, &nid);
}

void MainWindow::remove_tray_icon() {
    NOTIFYICONDATAW nid = {};
    nid.cbSize = sizeof(NOTIFYICONDATAW);
    nid.hWnd = hwnd_;
    nid.uID = 1;
    Shell_NotifyIconW(NIM_DELETE, &nid);
}

void MainWindow::show_tray_menu() {
    POINT pt;
    GetCursorPos(&pt);
    HMENU hMenu = CreatePopupMenu();
    AppendMenuW(hMenu, MF_STRING, 1, L"Show Orca-Light\t(Win+Space)");
    AppendMenuW(hMenu, MF_SEPARATOR, 0, nullptr);
    AppendMenuW(hMenu, MF_STRING, 2, L"Exit");
    
    SetForegroundWindow(hwnd_);
    int cmd = TrackPopupMenu(hMenu, TPM_RETURNCMD | TPM_NONOTIFY, pt.x, pt.y, 0, hwnd_, nullptr);
    DestroyMenu(hMenu);
    
    if (cmd == 1) {
        show();
    } else if (cmd == 2) {
        PostQuitMessage(0);
    }
}

void MainWindow::center_on_active_monitor() {

    POINT cursor_pt;
    GetCursorPos(&cursor_pt);
    HMONITOR hMon = MonitorFromPoint(cursor_pt, MONITOR_DEFAULTTONEAREST);

    MONITORINFO mi = {};
    mi.cbSize = sizeof(MONITORINFO);
    GetMonitorInfoW(hMon, &mi);

    float dpi_scale = renderer_.get_dpi_scale();
    float win_w = Theme::BASE_WINDOW_WIDTH * dpi_scale;
    float win_h = renderer_.calculate_window_height(current_results_.size());

    int mon_w = mi.rcWork.right - mi.rcWork.left;
    int mon_h = mi.rcWork.bottom - mi.rcWork.top;

    int x = mi.rcWork.left + static_cast<int>((mon_w - win_w) * 0.5f);
    // macOS Orca-Light positioning: ~22% from top of monitor
    int y = mi.rcWork.top + static_cast<int>(mon_h * 0.20f);

    SetWindowPos(
        hwnd_,
        HWND_TOPMOST,
        x, y,
        static_cast<int>(win_w),
        static_cast<int>(win_h),
        SWP_NOACTIVATE
    );

    renderer_.resize(static_cast<UINT>(win_w), static_cast<UINT>(win_h));
}

void MainWindow::update_layout() {
    float dpi_scale = renderer_.get_dpi_scale();
    float win_w = Theme::BASE_WINDOW_WIDTH * dpi_scale;
    float win_h = renderer_.calculate_window_height(current_results_.size());

    SetWindowPos(
        hwnd_,
        nullptr,
        0, 0,
        static_cast<int>(win_w),
        static_cast<int>(win_h),
        SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE
    );

    renderer_.resize(static_cast<UINT>(win_w), static_cast<UINT>(win_h));
    InvalidateRect(hwnd_, nullptr, FALSE);
}

void MainWindow::bring_to_foreground() {
    HWND fg = GetForegroundWindow();
    DWORD fgThread = fg ? GetWindowThreadProcessId(fg, nullptr) : 0;
    DWORD curThread = GetCurrentThreadId();

    if (fgThread != 0 && fgThread != curThread) {
        AttachThreadInput(curThread, fgThread, TRUE);
        SetForegroundWindow(hwnd_);
        SetFocus(hwnd_);
        AttachThreadInput(curThread, fgThread, FALSE);
    } else {
        SetForegroundWindow(hwnd_);
        SetFocus(hwnd_);
    }
}

void MainWindow::show() {
    is_visible_ = true;
    just_shown_ = true;
    current_query_.clear();
    cursor_pos_ = 0;
    selected_index_ = 0;
    hovered_index_ = -1;

    perform_search();
    center_on_active_monitor();

    ShowWindow(hwnd_, SW_SHOW);
    bring_to_foreground();
    anim_progress_ = 0.0f;
    anim_start_time_ = GetTickCount64();
    SetTimer(hwnd_, TIMER_ANIMATION, 16, nullptr);
    SetTimer(hwnd_, TIMER_ACTIVATION_GRACE, 350, nullptr);
    InvalidateRect(hwnd_, nullptr, FALSE);
}

void MainWindow::hide() {
    if (!is_visible_) {
        return;
    }
    is_visible_ = false;
    just_shown_ = false;
    KillTimer(hwnd_, TIMER_ACTIVATION_GRACE);
        KillTimer(hwnd_, TIMER_ANIMATION);
        remove_tray_icon();
    ShowWindow(hwnd_, SW_HIDE);
}

bool MainWindow::is_visible() const {
    return is_visible_ && IsWindowVisible(hwnd_);
}

void MainWindow::toggle_visibility() {
    if (is_visible_) {
        hide();
    } else {
        show();
    }
}

void MainWindow::perform_search() {
    current_results_ = search_engine_.query(current_query_, 9);
    if (selected_index_ >= static_cast<int>(current_results_.size())) {
        selected_index_ = current_results_.empty() ? 0 : static_cast<int>(current_results_.size() - 1);
    }
    update_layout();
}

void MainWindow::execute_selected_primary() {
    if (current_results_.empty() || selected_index_ < 0 || selected_index_ >= static_cast<int>(current_results_.size())) {
        return;
    }

    const auto& item = current_results_[selected_index_];
    search_engine_.record_launch(item.path);

    hide();

    if (item.custom_action) {
        item.custom_action(hwnd_);
    } else if (!item.path.empty()) {
        utils::launch_item(item.path, item.arguments);
    }
}

void MainWindow::execute_selected_secondary() {
    if (current_results_.empty() || selected_index_ < 0 || selected_index_ >= static_cast<int>(current_results_.size())) {
        return;
    }

    const auto& item = current_results_[selected_index_];
    if (!item.path.empty()) {
        hide();
        utils::open_containing_folder(item.path);
    }
}

void MainWindow::execute_selected_copy_path() {
    if (current_results_.empty() || selected_index_ < 0 || selected_index_ >= static_cast<int>(current_results_.size())) {
        return;
    }

    const auto& item = current_results_[selected_index_];
    std::wstring to_copy = !item.path.empty() ? item.path : item.title;
    utils::set_clipboard_text(hwnd_, to_copy);
    hide();
}

void MainWindow::execute_selected_admin() {
    if (current_results_.empty() || selected_index_ < 0 || selected_index_ >= static_cast<int>(current_results_.size())) {
        return;
    }

    const auto& item = current_results_[selected_index_];
    if (!item.path.empty()) {
        hide();
        utils::launch_as_admin(item.path, item.arguments);
    }
}

LRESULT MainWindow::window_proc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_HOTKEY:
        case WM_USER + 101: {
            toggle_visibility();
            return 0;
        }

                case WM_TRAYICON: {
            if (lParam == WM_LBUTTONUP) {
                toggle_visibility();
            } else if (lParam == WM_RBUTTONUP) {
                show_tray_menu();
            }
            return 0;
        }
        
        case WM_ACTIVATE: {
            if (LOWORD(wParam) == WA_INACTIVE) {
                if (is_visible_ && !just_shown_) {
                    hide();
                }
            } else {
                just_shown_ = false;
                KillTimer(hwnd, TIMER_ACTIVATION_GRACE);
            }
            return 0;
        }

        case WM_PAINT: {
            PAINTSTRUCT ps;
            BeginPaint(hwnd, &ps);
            renderer_.render(
                hwnd,
                current_query_,
                cursor_pos_,
                show_cursor_,
                current_results_,
                selected_index_,
                hovered_index_,
                anim_progress_
            );
            EndPaint(hwnd, &ps);
            return 0;
        }

        case WM_ERASEBKGND:
            return 1; // Prevent flickering

        case WM_TIMER: {
            if (wParam == TIMER_CURSOR_BLINK) {
                show_cursor_ = !show_cursor_;
                if (is_visible_) {
                    // Repaint search bar region
                    InvalidateRect(hwnd, nullptr, FALSE);
                }
            } else if (wParam == TIMER_ACTIVATION_GRACE) {
                just_shown_ = false;
                KillTimer(hwnd, TIMER_ACTIVATION_GRACE);
            } else if (wParam == TIMER_ANIMATION) {
                ULONGLONG now = GetTickCount64();
                float elapsed = static_cast<float>(now - anim_start_time_);
                anim_progress_ = elapsed / 150.0f;
                if (anim_progress_ >= 1.0f) {
                    anim_progress_ = 1.0f;
                    KillTimer(hwnd, TIMER_ANIMATION);
                }
                InvalidateRect(hwnd, nullptr, FALSE);
            }
            return 0;
        }

        case WM_DPICHANGED: {
            UINT new_dpi = HIWORD(wParam);
            renderer_.update_dpi(static_cast<float>(new_dpi) / 96.0f);
            RECT* prc = reinterpret_cast<RECT*>(lParam);
            SetWindowPos(
                hwnd,
                nullptr,
                prc->left, prc->top,
                prc->right - prc->left,
                prc->bottom - prc->top,
                SWP_NOZORDER | SWP_NOACTIVATE
            );
            renderer_.resize(prc->right - prc->left, prc->bottom - prc->top);
            InvalidateRect(hwnd, nullptr, FALSE);
            return 0;
        }

        case WM_CLIPBOARDUPDATE: {
            if (on_clipboard_update_) {
                on_clipboard_update_(hwnd);
            }
            return 0;
        }

        case WM_KEYDOWN: {
            show_cursor_ = true;
            bool ctrl = (GetKeyState(VK_CONTROL) & 0x8000) != 0;
            bool shift = (GetKeyState(VK_SHIFT) & 0x8000) != 0;

            switch (wParam) {
                case VK_ESCAPE:
                    if (!current_query_.empty()) {
                        current_query_.clear();
                        cursor_pos_ = 0;
                        perform_search();
                    } else {
                        hide();
                    }
                    return 0;

                case VK_DOWN:
                    if (!current_results_.empty()) {
                        selected_index_ = (selected_index_ + 1) % static_cast<int>(current_results_.size());
                        InvalidateRect(hwnd, nullptr, FALSE);
                    }
                    return 0;

                case VK_UP:
                    if (!current_results_.empty()) {
                        selected_index_ = (selected_index_ - 1 + static_cast<int>(current_results_.size())) % static_cast<int>(current_results_.size());
                        InvalidateRect(hwnd, nullptr, FALSE);
                    }
                    return 0;

                case VK_PRIOR: // Page Up
                    if (!current_results_.empty()) {
                        selected_index_ = std::max(0, selected_index_ - 5);
                        InvalidateRect(hwnd, nullptr, FALSE);
                    }
                    return 0;

                case VK_NEXT: // Page Down
                    if (!current_results_.empty()) {
                        selected_index_ = std::min(static_cast<int>(current_results_.size() - 1), selected_index_ + 5);
                        InvalidateRect(hwnd, nullptr, FALSE);
                    }
                    return 0;

                case VK_RETURN:
                    if (ctrl && shift) {
                        execute_selected_admin();
                    } else if (ctrl) {
                        execute_selected_secondary();
                    } else {
                        execute_selected_primary();
                    }
                    return 0;

                case 'C':
                    if (ctrl && shift) {
                        execute_selected_copy_path();
                        return 0;
                    }
                    break;

                case VK_LEFT:
                    if (cursor_pos_ > 0) {
                        --cursor_pos_;
                        InvalidateRect(hwnd, nullptr, FALSE);
                    }
                    return 0;

                case VK_RIGHT:
                    if (cursor_pos_ < current_query_.size()) {
                        ++cursor_pos_;
                        InvalidateRect(hwnd, nullptr, FALSE);
                    }
                    return 0;

                case VK_HOME:
                    cursor_pos_ = 0;
                    InvalidateRect(hwnd, nullptr, FALSE);
                    return 0;

                case VK_END:
                    cursor_pos_ = current_query_.size();
                    InvalidateRect(hwnd, nullptr, FALSE);
                    return 0;

                case VK_BACK:
                    if (cursor_pos_ > 0 && !current_query_.empty()) {
                        current_query_.erase(cursor_pos_ - 1, 1);
                        --cursor_pos_;
                        perform_search();
                    }
                    return 0;

                case VK_DELETE:
                    if (cursor_pos_ < current_query_.size()) {
                        current_query_.erase(cursor_pos_, 1);
                        perform_search();
                    }
                    return 0;

                case VK_TAB:
                    // Cycle or secondary execute
                    execute_selected_secondary();
                    return 0;
            }
            break;
        }

        case WM_CHAR: {
            wchar_t ch = static_cast<wchar_t>(wParam);
            bool ctrl = (GetKeyState(VK_CONTROL) & 0x8000) != 0;

            if (ctrl) {
                // Ctrl+V Paste
                if (ch == 22) {
                    std::wstring text = utils::get_clipboard_text(hwnd);
                    if (!text.empty()) {
                        // Collapse newlines into space for single-line search
                        for (wchar_t& c : text) {
                            if (c == L'\r' || c == L'\n') c = L' ';
                        }
                        current_query_.insert(cursor_pos_, text);
                        cursor_pos_ += text.size();
                        perform_search();
                    }
                    return 0;
                }
                // Ctrl+A Select All / Clear
                if (ch == 1) {
                    current_query_.clear();
                    cursor_pos_ = 0;
                    perform_search();
                    return 0;
                }
                return 0;
            }

            // Normal printable unicode characters
            if (ch >= 32 && ch != 127) {
                current_query_.insert(cursor_pos_, 1, ch);
                ++cursor_pos_;
                perform_search();
                return 0;
            }
            break;
        }

        case WM_MOUSEMOVE: {
            int y = GET_Y_LPARAM(lParam);
            float dpi_scale = renderer_.get_dpi_scale();
            float search_h = Theme::BASE_SEARCH_HEIGHT * dpi_scale;
            float item_h = Theme::BASE_ITEM_HEIGHT * dpi_scale;

            if (y >= search_h) {
                int index = static_cast<int>((y - search_h) / item_h);
                if (index >= 0 && index < static_cast<int>(current_results_.size())) {
                    if (hovered_index_ != index) {
                        hovered_index_ = index;
                        InvalidateRect(hwnd, nullptr, FALSE);
                    }
                } else if (hovered_index_ != -1) {
                    hovered_index_ = -1;
                    InvalidateRect(hwnd, nullptr, FALSE);
                }
            } else if (hovered_index_ != -1) {
                hovered_index_ = -1;
                InvalidateRect(hwnd, nullptr, FALSE);
            }
            return 0;
        }

        case WM_LBUTTONDOWN: {
            int y = GET_Y_LPARAM(lParam);
            float dpi_scale = renderer_.get_dpi_scale();
            float search_h = Theme::BASE_SEARCH_HEIGHT * dpi_scale;
            float item_h = Theme::BASE_ITEM_HEIGHT * dpi_scale;

            if (y >= search_h) {
                int index = static_cast<int>((y - search_h) / item_h);
                if (index >= 0 && index < static_cast<int>(current_results_.size())) {
                    selected_index_ = index;
                    execute_selected_primary();
                }
            }
            return 0;
        }

        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
    }

    return DefWindowProcW(hwnd, msg, wParam, lParam);
}

} // namespace orca_light::ui
