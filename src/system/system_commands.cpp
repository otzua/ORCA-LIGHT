#include "system_commands.h"
#include "../utils/shell_utils.h"

namespace orca_light::system {

SystemCommandRegistry::SystemCommandRegistry() {
    register_builtins();
}

void SystemCommandRegistry::register_builtins() {
    commands_ = {
        {
            L"lock",
            L"Lock Workstation",
            L"Lock the current session immediately",
            { L"lock", L"logout", L"secure" },
            [](HWND) { utils::lock_workstation(); }
        },
        {
            L"sleep",
            L"Sleep",
            L"Put the system into low-power sleep state",
            { L"sleep", L"suspend", L"standby" },
            [](HWND) { utils::sleep_system(); }
        },
        {
            L"restart",
            L"Restart PC",
            L"Reboot the operating system",
            { L"restart", L"reboot" },
            [](HWND) { utils::restart_system(); }
        },
        {
            L"shutdown",
            L"Shutdown PC",
            L"Turn off the computer",
            { L"shutdown", L"power off", L"turn off" },
            [](HWND) { utils::shutdown_system(); }
        },
        {
            L"signout",
            L"Sign Out",
            L"Log out current user account",
            { L"sign out", L"log off", L"signout", L"logoff" },
            [](HWND) { utils::sign_out_user(); }
        },
        {
            L"emptyrecyclebin",
            L"Empty Recycle Bin",
            L"Permanently delete all files in the Recycle Bin",
            { L"empty recycle bin", L"clean bin", L"trash", L"recycle bin" },
            [](HWND hwnd) { utils::empty_recycle_bin(hwnd); }
        },
        {
            L"taskmgr",
            L"Task Manager",
            L"Open Windows Task Manager",
            { L"task manager", L"taskmgr", L"processes", L"performance" },
            [](HWND) { utils::open_task_manager(); }
        },
        {
            L"settings",
            L"Windows Settings",
            L"Open Windows System Settings",
            { L"settings", L"preferences", L"config" },
            [](HWND) { utils::open_system_settings(); }
        },
        {
            L"controlpanel",
            L"Control Panel",
            L"Open classic Windows Control Panel",
            { L"control panel", L"control", L"cpl" },
            [](HWND) { utils::open_control_panel(); }
        },
        {
            L"devicemanager",
            L"Device Manager",
            L"Open Device Manager to inspect hardware and drivers",
            { L"device manager", L"devmgmt", L"drivers" },
            [](HWND) { utils::open_device_manager(); }
        },
        {
            L"cmd",
            L"Command Prompt",
            L"Open Windows Command Prompt (cmd.exe)",
            { L"cmd", L"terminal", L"console", L"command prompt" },
            [](HWND) { utils::open_command_prompt(false); }
        },
        {
            L"cmdadmin",
            L"Command Prompt (Administrator)",
            L"Open elevated Windows Command Prompt",
            { L"cmd admin", L"elevated cmd", L"admin console" },
            [](HWND) { utils::open_command_prompt(true); }
        },
        {
            L"powershell",
            L"PowerShell",
            L"Open Windows PowerShell console",
            { L"powershell", L"posh", L"terminal", L"pwsh" },
            [](HWND) { utils::open_powershell(false); }
        },
        {
            L"powershelladmin",
            L"PowerShell (Administrator)",
            L"Open elevated Windows PowerShell console",
            { L"powershell admin", L"elevated powershell", L"admin posh" },
            [](HWND) { utils::open_powershell(true); }
        },
        {
            L"orca-lightsettings",
            L"Open Orca-Light Settings",
            L"Open and edit Orca-Light configuration file",
            { L"orca-light settings", L"hotkey", L"config", L"preferences" },
            [this](HWND) {
                if (open_config_callback_) {
                    open_config_callback_();
                }
            }
        },
        {
            L"reindex",
            L"Reindex Files and Apps",
            L"Scan filesystem and rebuild search index in background",
            { L"reindex", L"refresh index", L"rescan" },
            [this](HWND) {
                if (reindex_callback_) {
                    reindex_callback_();
                }
            }
        },
        {
            L"clearclipboard",
            L"Clear Clipboard History",
            L"Erase all saved clipboard history items",
            { L"clear clipboard", L"clean clipboard", L"empty clipboard" },
            [this](HWND) {
                if (clear_clipboard_callback_) {
                    clear_clipboard_callback_();
                }
            }
        },
        {
            L"exitorca-light",
            L"Quit Orca-Light",
            L"Exit the Orca-Light launcher application",
            { L"exit", L"quit", L"close orca-light" },
            [this](HWND) {
                if (exit_callback_) {
                    exit_callback_();
                }
            }
        }
    };
}

} // namespace orca_light::system
