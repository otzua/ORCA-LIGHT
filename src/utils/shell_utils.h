#pragma once

#include <windows.h>
#include <string>
#include <string_view>

namespace orca_light::utils {

// Shell execution
bool launch_item(std::wstring_view path, std::wstring_view args = L"", std::wstring_view work_dir = L"");
bool launch_as_admin(std::wstring_view path, std::wstring_view args = L"");
bool open_containing_folder(std::wstring_view file_path);

// Clipboard operations
bool set_clipboard_text(HWND hwnd, std::wstring_view text);
std::wstring get_clipboard_text(HWND hwnd);

// System actions
bool empty_recycle_bin(HWND hwnd);
bool lock_workstation();
bool sleep_system();
bool restart_system();
bool shutdown_system();
bool sign_out_user();
bool open_system_settings(std::wstring_view subpage = L"");
bool open_task_manager();
bool open_control_panel();
bool open_device_manager();
bool open_command_prompt(bool as_admin = false);
bool open_powershell(bool as_admin = false);

} // namespace orca_light::utils
