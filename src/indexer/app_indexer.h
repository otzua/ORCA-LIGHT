#pragma once

#include <windows.h>
#include <string>
#include <vector>
#include <mutex>
#include <memory>

namespace orca_light::indexer {

struct AppItem {
    std::wstring name;
    std::wstring description;
    std::wstring target_path;
    std::wstring arguments;
    std::wstring shortcut_path;
    std::wstring icon_path;
    int icon_index = 0;
    bool is_uwp = false;
};

class AppIndexer {
public:
    AppIndexer();

    // Rebuild installed application index
    void scan_apps();

    // Get current indexed applications (thread-safe)
    std::shared_ptr<const std::vector<AppItem>> get_apps() const;

private:
    void scan_directory_shortcuts(const std::wstring& dir_path, std::vector<AppItem>& local_apps);
    void scan_registry_app_paths(std::vector<AppItem>& local_apps);
    void scan_known_tools(std::vector<AppItem>& local_apps);
    bool resolve_shell_link(const std::wstring& lnk_path, AppItem& out_app);

    mutable std::mutex mutex_;
    std::shared_ptr<const std::vector<AppItem>> apps_;
};

} // namespace orca_light::indexer
