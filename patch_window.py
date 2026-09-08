import re

with open('src/ui/window.cpp', 'r') as f:
    content = f.read()

# Add includes
content = content.replace('#include "window.h"', '#include "window.h"\n#include <shellapi.h>\n#include <dwmapi.h>\n#pragma comment(lib, "dwmapi.lib")')

# Add constants
content = content.replace('constexpr UINT_PTR TIMER_ACTIVATION_GRACE = 1002;', 'constexpr UINT_PTR TIMER_ACTIVATION_GRACE = 1002;\nconstexpr UINT_PTR TIMER_ANIMATION = 1003;\nconstexpr UINT WM_TRAYICON = WM_USER + 102;')

# Add glass and tray to create()
create_hook = 'AddClipboardFormatListener(hwnd_);'
create_impl = 'enable_glass();\n    setup_tray_icon();\n    ' + create_hook
content = content.replace(create_hook, create_impl)

# Add destructor cleanup
destroy_hook = 'KillTimer(hwnd_, TIMER_ACTIVATION_GRACE);'
destroy_impl = destroy_hook + '\n        KillTimer(hwnd_, TIMER_ANIMATION);\n        remove_tray_icon();'
content = content.replace(destroy_hook, destroy_impl)

# Add show animation
show_hook = 'SetTimer(hwnd_, TIMER_ACTIVATION_GRACE, 350, nullptr);'
show_impl = 'anim_progress_ = 0.0f;\n    anim_start_time_ = GetTickCount64();\n    SetTimer(hwnd_, TIMER_ANIMATION, 16, nullptr);\n    ' + show_hook
content = content.replace(show_hook, show_impl)

# Add implementations
impls = """
void MainWindow::enable_glass() {
    DWM_BLURBEHIND bb = {0};
    bb.dwFlags = DWM_BB_ENABLE;
    bb.fEnable = TRUE;
    bb.hRgnBlur = nullptr;
    DwmEnableBlurBehindWindow(hwnd_, &bb);
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
    AppendMenuW(hMenu, MF_STRING, 1, L"Show Orca-Light\\t(Win+Space)");
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
"""
content = content.replace('void MainWindow::center_on_active_monitor() {', impls)

# Modify window_proc
wm_activate = 'case WM_ACTIVATE: {'
wm_tray = '''        case WM_TRAYICON: {
            if (lParam == WM_LBUTTONUP) {
                toggle_visibility();
            } else if (lParam == WM_RBUTTONUP) {
                show_tray_menu();
            }
            return 0;
        }
        
        ''' + wm_activate
content = content.replace(wm_activate, wm_tray)

render_call = '''            renderer_.render(
                hwnd,
                current_query_,
                cursor_pos_,
                show_cursor_,
                current_results_,
                selected_index_,
                hovered_index_
            );'''
render_impl = '''            renderer_.render(
                hwnd,
                current_query_,
                cursor_pos_,
                show_cursor_,
                current_results_,
                selected_index_,
                hovered_index_,
                anim_progress_
            );'''
content = content.replace(render_call, render_impl)

wm_timer = '''            } else if (wParam == TIMER_ACTIVATION_GRACE) {
                just_shown_ = false;
                KillTimer(hwnd, TIMER_ACTIVATION_GRACE);
            }'''
wm_timer_impl = wm_timer + ''' else if (wParam == TIMER_ANIMATION) {
                ULONGLONG now = GetTickCount64();
                float elapsed = static_cast<float>(now - anim_start_time_);
                anim_progress_ = elapsed / 150.0f;
                if (anim_progress_ >= 1.0f) {
                    anim_progress_ = 1.0f;
                    KillTimer(hwnd, TIMER_ANIMATION);
                }
                InvalidateRect(hwnd, nullptr, FALSE);
            }'''
content = content.replace(wm_timer, wm_timer_impl)

with open('src/ui/window.cpp', 'w') as f:
    f.write(content)

