#pragma once

#include <windows.h>
#include <string>
#include <vector>
#include <functional>

namespace orca_light::system {

struct SystemCommand {
    std::wstring id;
    std::wstring name;
    std::wstring description;
    std::vector<std::wstring> keywords;
    std::function<void(HWND hwnd)> execute;
};

class SystemCommandRegistry {
public:
    SystemCommandRegistry();

    const std::vector<SystemCommand>& get_all_commands() const {
        return commands_;
    }

    // Set callback to reload index or exit app
    void set_exit_callback(std::function<void()> cb) {
        exit_callback_ = cb;
    }
    void set_reindex_callback(std::function<void()> cb) {
        reindex_callback_ = cb;
    }
    void set_clear_clipboard_callback(std::function<void()> cb) {
        clear_clipboard_callback_ = cb;
    }
    void set_open_config_callback(std::function<void()> cb) {
        open_config_callback_ = cb;
    }

private:
    void register_builtins();

    std::vector<SystemCommand> commands_;
    std::function<void()> exit_callback_;
    std::function<void()> reindex_callback_;
    std::function<void()> clear_clipboard_callback_;
    std::function<void()> open_config_callback_;
};

} // namespace orca_light::system
