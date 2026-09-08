#pragma once

#include <windows.h>
#include <memory>
#include "config/config_manager.h"
#include "indexer/app_indexer.h"
#include "indexer/file_indexer.h"
#include "clipboard/clipboard_history.h"
#include "system/system_commands.h"
#include "search/search_engine.h"
#include "ui/window.h"

namespace orca_light {

class Application {
public:
    Application();
    ~Application();

    bool initialize(HINSTANCE hInstance);
    int run();
    void shutdown();
    void toggle_window();
    HWND get_hwnd() const;

private:
    void register_global_hotkey();
    void unregister_global_hotkey();
    void install_keyboard_hook();
    void uninstall_keyboard_hook();
    void open_settings_file();
    void trigger_reindex();

    HINSTANCE instance_ = nullptr;
    HANDLE mutex_handle_ = nullptr;

    config::ConfigManager config_manager_;
    indexer::AppIndexer app_indexer_;
    indexer::FileIndexer file_indexer_;
    clipboard::ClipboardHistory clipboard_history_;
    system::SystemCommandRegistry command_registry_;

    std::unique_ptr<search::SearchEngine> search_engine_;
    std::unique_ptr<ui::MainWindow> main_window_;

    static constexpr int HOTKEY_ID = 1001;
};

} // namespace orca_light
