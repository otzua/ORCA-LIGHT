#include "shell_utils.h"
#include <shlobj.h>
#include <shellapi.h>
#include <shlwapi.h>
#include <vector>

namespace orca_light::utils {

namespace {

bool enable_shutdown_privilege() {
    HANDLE token = nullptr;
    if (!OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &token)) {
        return false;
    }

    TOKEN_PRIVILEGES tp;
    LUID luid;
    if (!LookupPrivilegeValueW(nullptr, SE_SHUTDOWN_NAME, &luid)) {
        CloseHandle(token);
        return false;
    }

    tp.PrivilegeCount = 1;
    tp.Privileges[0].Luid = luid;
    tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

    BOOL ok = AdjustTokenPrivileges(token, FALSE, &tp, sizeof(TOKEN_PRIVILEGES), nullptr, nullptr);
    CloseHandle(token);
    return ok && (GetLastError() != ERROR_NOT_ALL_ASSIGNED);
}

} // namespace

bool launch_item(std::wstring_view path, std::wstring_view args, std::wstring_view work_dir) {
    std::wstring path_str(path);
    std::wstring args_str(args);
    std::wstring dir_str(work_dir);

    SHELLEXECUTEINFOW sei = {};
    sei.cbSize = sizeof(SHELLEXECUTEINFOW);
    sei.fMask = SEE_MASK_FLAG_NO_UI | SEE_MASK_ASYNCOK;
    sei.lpVerb = L"open";
    sei.lpFile = path_str.c_str();
    sei.lpParameters = args_str.empty() ? nullptr : args_str.c_str();
    sei.lpDirectory = dir_str.empty() ? nullptr : dir_str.c_str();
    sei.nShow = SW_SHOWNORMAL;

    return ShellExecuteExW(&sei) == TRUE;
}

bool launch_as_admin(std::wstring_view path, std::wstring_view args) {
    std::wstring path_str(path);
    std::wstring args_str(args);

    SHELLEXECUTEINFOW sei = {};
    sei.cbSize = sizeof(SHELLEXECUTEINFOW);
    sei.fMask = SEE_MASK_FLAG_NO_UI | SEE_MASK_ASYNCOK;
    sei.lpVerb = L"runas";
    sei.lpFile = path_str.c_str();
    sei.lpParameters = args_str.empty() ? nullptr : args_str.c_str();
    sei.nShow = SW_SHOWNORMAL;

    return ShellExecuteExW(&sei) == TRUE;
}

bool open_containing_folder(std::wstring_view file_path) {
    std::wstring full_path(file_path);
    PIDLIST_ABSOLUTE pidl = ILCreateFromPathW(full_path.c_str());
    if (pidl) {
        HRESULT hr = SHOpenFolderAndSelectItems(pidl, 0, nullptr, 0);
        ILFree(pidl);
        if (SUCCEEDED(hr)) {
            return true;
        }
    }

    // Fallback to explorer /select
    std::wstring params = L"/select,\"" + full_path + L"\"";
    return launch_item(L"explorer.exe", params);
}

bool set_clipboard_text(HWND hwnd, std::wstring_view text) {
    if (!OpenClipboard(hwnd)) {
        return false;
    }
    EmptyClipboard();

    size_t byte_count = (text.size() + 1) * sizeof(wchar_t);
    HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE, byte_count);
    if (!hMem) {
        CloseClipboard();
        return false;
    }

    void* ptr = GlobalLock(hMem);
    if (ptr) {
        memcpy(ptr, text.data(), text.size() * sizeof(wchar_t));
        reinterpret_cast<wchar_t*>(ptr)[text.size()] = L'\0';
        GlobalUnlock(hMem);
        SetClipboardData(CF_UNICODETEXT, hMem);
    } else {
        GlobalFree(hMem);
    }

    CloseClipboard();
    return true;
}

std::wstring get_clipboard_text(HWND hwnd) {
    if (!OpenClipboard(hwnd)) {
        return L"";
    }

    std::wstring result;
    HANDLE hData = GetClipboardData(CF_UNICODETEXT);
    if (hData) {
        const wchar_t* pText = static_cast<const wchar_t*>(GlobalLock(hData));
        if (pText) {
            result = pText;
            GlobalUnlock(hData);
        }
    }

    CloseClipboard();
    return result;
}

bool empty_recycle_bin(HWND hwnd) {
    HRESULT hr = SHEmptyRecycleBinW(hwnd, nullptr, SHERB_NOCONFIRMATION | SHERB_NOPROGRESSUI | SHERB_NOSOUND);
    return SUCCEEDED(hr);
}

bool lock_workstation() {
    return LockWorkStation() == TRUE;
}

bool sleep_system() {
    HMODULE hPowrprof = LoadLibraryW(L"powrprof.dll");
    if (!hPowrprof) {
        return false;
    }
    typedef BOOLEAN(WINAPI* SetSuspendStateFn)(BOOLEAN, BOOLEAN, BOOLEAN);
    SetSuspendStateFn fn = reinterpret_cast<SetSuspendStateFn>(GetProcAddress(hPowrprof, "SetSuspendState"));
    bool success = false;
    if (fn) {
        success = (fn(FALSE, FALSE, FALSE) == TRUE);
    }
    FreeLibrary(hPowrprof);
    return success;
}

bool restart_system() {
    if (!enable_shutdown_privilege()) {
        return false;
    }
    return ExitWindowsEx(EWX_REBOOT | EWX_FORCEIFHUNG, SHTDN_REASON_MAJOR_OPERATINGSYSTEM | SHTDN_REASON_MINOR_RECONFIG) == TRUE;
}

bool shutdown_system() {
    if (!enable_shutdown_privilege()) {
        return false;
    }
    return ExitWindowsEx(EWX_SHUTDOWN | EWX_FORCEIFHUNG, SHTDN_REASON_MAJOR_OPERATINGSYSTEM | SHTDN_REASON_MINOR_MAINTENANCE) == TRUE;
}

bool sign_out_user() {
    return ExitWindowsEx(EWX_LOGOFF | EWX_FORCEIFHUNG, 0) == TRUE;
}

bool open_system_settings(std::wstring_view subpage) {
    std::wstring uri = L"ms-settings:";
    if (!subpage.empty()) {
        uri += subpage;
    }
    return launch_item(uri);
}

bool open_task_manager() {
    return launch_item(L"taskmgr.exe");
}

bool open_control_panel() {
    return launch_item(L"control.exe");
}

bool open_device_manager() {
    return launch_item(L"devmgmt.msc");
}

bool open_command_prompt(bool as_admin) {
    if (as_admin) {
        return launch_as_admin(L"cmd.exe");
    }
    return launch_item(L"cmd.exe");
}

bool open_powershell(bool as_admin) {
    if (as_admin) {
        return launch_as_admin(L"powershell.exe");
    }
    return launch_item(L"powershell.exe");
}

} // namespace orca_light::utils
