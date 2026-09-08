#include "config_manager.h"
#include "../utils/string_utils.h"
#include <shlobj.h>
#include <fstream>
#include <sstream>
#include <filesystem>
#include <algorithm>

namespace orca_light::config {

namespace {

const wchar_t* REG_RUN_KEY = L"Software\\Microsoft\\Windows\\CurrentVersion\\Run";
const wchar_t* REG_APP_NAME = L"Orca-Light";

std::wstring get_current_exe_path() {
    wchar_t path[MAX_PATH];
    GetModuleFileNameW(nullptr, path, MAX_PATH);
    return std::wstring(path);
}

} // namespace

ConfigManager::ConfigManager() {
    init_defaults();
}

void ConfigManager::init_defaults() {
    // AppData folder path
    PWSTR appdata_path = nullptr;
    if (SUCCEEDED(SHGetKnownFolderPath(FOLDERID_RoamingAppData, 0, nullptr, &appdata_path))) {
        config_dir_path_ = std::wstring(appdata_path) + L"\\Orca-Light";
        config_file_path_ = config_dir_path_ + L"\\config.ini";
        CoTaskMemFree(appdata_path);
    } else {
        config_dir_path_ = L".";
        config_file_path_ = L"config.ini";
    }

    config_.hotkey_modifiers = MOD_WIN;
    config_.hotkey_vk = VK_SPACE;
    config_.hotkey_display = L"Win + Space";
    config_.autostart_with_windows = is_autostart_enabled();
    config_.max_results = 9;
    config_.max_clipboard_items = 50;

    // Default search folders: Desktop, Documents, Downloads
    PWSTR known_path = nullptr;
    if (SUCCEEDED(SHGetKnownFolderPath(FOLDERID_Desktop, 0, nullptr, &known_path))) {
        config_.search_directories.push_back(known_path);
        CoTaskMemFree(known_path);
    }
    if (SUCCEEDED(SHGetKnownFolderPath(FOLDERID_Documents, 0, nullptr, &known_path))) {
        config_.search_directories.push_back(known_path);
        CoTaskMemFree(known_path);
    }
    if (SUCCEEDED(SHGetKnownFolderPath(FOLDERID_Downloads, 0, nullptr, &known_path))) {
        config_.search_directories.push_back(known_path);
        CoTaskMemFree(known_path);
    }

    // Default exclude patterns
    config_.exclude_patterns = {
        L".git",
        L".svn",
        L"node_modules",
        L"AppData\\Local\\Temp",
        L"AppData\\Local\\Microsoft",
        L"$Recycle.Bin",
        L"Windows\\WinSxS",
        L"System Volume Information",
        L"__pycache__",
        L".vscode",
        L".idea"
    };
}

void ConfigManager::ensure_config_dir_exists() {
    CreateDirectoryW(config_dir_path_.c_str(), nullptr);
}

bool ConfigManager::load() {
    ensure_config_dir_exists();

    std::filesystem::path path(config_file_path_);
    std::ifstream file(path);
    if (!file.is_open()) {
        // Save default config
        save();
        return false;
    }

    std::string line;
    std::string section;
    std::vector<std::wstring> custom_dirs;
    std::vector<std::wstring> custom_excludes;

    while (std::getline(file, line)) {
        std::string trimmed = utils::trim(line);
        if (trimmed.empty() || trimmed[0] == ';' || trimmed[0] == '#') {
            continue;
        }

        if (trimmed.front() == '[' && trimmed.back() == ']') {
            section = utils::to_lower(trimmed.substr(1, trimmed.size() - 2));
            continue;
        }

        size_t eq = trimmed.find('=');
        if (eq == std::string::npos) {
            continue;
        }

        std::string key = utils::to_lower(utils::trim(trimmed.substr(0, eq)));
        std::string val = utils::trim(trimmed.substr(eq + 1));
        std::wstring wval = utils::utf8_to_wide(val);

        if (section == "general") {
            if (key == "hotkey") {
                config_.hotkey_display = wval;
                UINT mods = 0;
                UINT vk = VK_SPACE;
                if (val.find("Ctrl") != std::string::npos || val.find("Control") != std::string::npos) mods |= MOD_CONTROL;
                if (val.find("Alt") != std::string::npos) mods |= MOD_ALT;
                if (val.find("Shift") != std::string::npos) mods |= MOD_SHIFT;
                if (val.find("Win") != std::string::npos || val.find("Windows") != std::string::npos || val.find("Super") != std::string::npos) mods |= MOD_WIN;

                if (val.find("Space") != std::string::npos) vk = VK_SPACE;
                else if (val.find("F1") != std::string::npos) vk = VK_F1;
                else if (val.find("F2") != std::string::npos) vk = VK_F2;
                else if (val.find("F3") != std::string::npos) vk = VK_F3;
                else if (val.find("F4") != std::string::npos) vk = VK_F4;
                else if (val.find("Tab") != std::string::npos) vk = VK_TAB;
                
                config_.hotkey_modifiers = mods;
                config_.hotkey_vk = vk;
            } else if (key == "autostart") {
                config_.autostart_with_windows = (val == "1" || val == "true" || val == "yes");
                set_autostart_registry(config_.autostart_with_windows);
            } else if (key == "max_results") {
                try {
                    config_.max_results = static_cast<uint32_t>(std::clamp(std::stoi(val), 3, 20));
                } catch (...) {}
            } else if (key == "max_clipboard_items") {
                try {
                    config_.max_clipboard_items = static_cast<uint32_t>(std::clamp(std::stoi(val), 5, 200));
                } catch (...) {}
            }
        } else if (section == "directories") {
            if (!wval.empty()) {
                custom_dirs.push_back(wval);
            }
        } else if (section == "excludes") {
            if (!wval.empty()) {
                custom_excludes.push_back(wval);
            }
        }
    }

    if (!custom_dirs.empty()) {
        config_.search_directories = std::move(custom_dirs);
    }
    if (!custom_excludes.empty()) {
        config_.exclude_patterns = std::move(custom_excludes);
    }

    return true;
}

bool ConfigManager::save() {
    ensure_config_dir_exists();

    std::filesystem::path path(config_file_path_);
    std::ofstream file(path);
    if (!file.is_open()) {
        return false;
    }

    file << "; Orca-Light Configuration File\n";
    file << "; Ultra-lightweight native Windows launcher\n\n";

    file << "[General]\n";
    file << "hotkey=" << utils::wide_to_utf8(config_.hotkey_display) << "\n";
    file << "autostart=" << (config_.autostart_with_windows ? "true" : "false") << "\n";
    file << "max_results=" << config_.max_results << "\n";
    file << "max_clipboard_items=" << config_.max_clipboard_items << "\n\n";

    file << "[Directories]\n";
    for (size_t i = 0; i < config_.search_directories.size(); ++i) {
        file << "dir" << (i + 1) << "=" << utils::wide_to_utf8(config_.search_directories[i]) << "\n";
    }
    file << "\n";

    file << "[Excludes]\n";
    for (size_t i = 0; i < config_.exclude_patterns.size(); ++i) {
        file << "exclude" << (i + 1) << "=" << utils::wide_to_utf8(config_.exclude_patterns[i]) << "\n";
    }
    file << "\n";

    return true;
}

bool ConfigManager::set_autostart_registry(bool enable) {
    HKEY hKey = nullptr;
    LONG res = RegOpenKeyExW(HKEY_CURRENT_USER, REG_RUN_KEY, 0, KEY_SET_VALUE, &hKey);
    if (res != ERROR_SUCCESS) {
        return false;
    }

    if (enable) {
        std::wstring exe_path = L"\"" + get_current_exe_path() + L"\"";
        res = RegSetValueExW(hKey, REG_APP_NAME, 0, REG_SZ, 
                             reinterpret_cast<const BYTE*>(exe_path.c_str()), 
                             static_cast<DWORD>((exe_path.size() + 1) * sizeof(wchar_t)));
    } else {
        res = RegDeleteValueW(hKey, REG_APP_NAME);
    }

    RegCloseKey(hKey);
    return res == ERROR_SUCCESS;
}

bool ConfigManager::is_autostart_enabled() {
    HKEY hKey = nullptr;
    LONG res = RegOpenKeyExW(HKEY_CURRENT_USER, REG_RUN_KEY, 0, KEY_QUERY_VALUE, &hKey);
    if (res != ERROR_SUCCESS) {
        return false;
    }

    DWORD type = 0;
    DWORD data_size = 0;
    res = RegQueryValueExW(hKey, REG_APP_NAME, nullptr, &type, nullptr, &data_size);
    RegCloseKey(hKey);

    return res == ERROR_SUCCESS;
}

} // namespace orca_light::config
