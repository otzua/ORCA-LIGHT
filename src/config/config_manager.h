#pragma once

#include <windows.h>
#include <string>
#include <vector>
#include <cstdint>

namespace orca_light::config {

struct AppConfig {
    // Hotkey settings
    UINT hotkey_modifiers = MOD_WIN;
    UINT hotkey_vk = VK_SPACE;
    std::wstring hotkey_display = L"Win + Space";

    // Startup
    bool autostart_with_windows = false;

    // Search limits
    uint32_t max_results = 9;
    uint32_t max_clipboard_items = 50;

    // Indexing paths
    std::vector<std::wstring> search_directories;
    std::vector<std::wstring> exclude_patterns;
};

class ConfigManager {
public:
    ConfigManager();

    bool load();
    bool save();

    const AppConfig& get() const { return config_; }
    AppConfig& get_mutable() { return config_; }

    const std::wstring& get_config_path() const { return config_file_path_; }

    // Windows Autostart registry helpers
    static bool set_autostart_registry(bool enable);
    static bool is_autostart_enabled();

private:
    void init_defaults();
    void ensure_config_dir_exists();

    AppConfig config_;
    std::wstring config_dir_path_;
    std::wstring config_file_path_;
};

} // namespace orca_light::config
