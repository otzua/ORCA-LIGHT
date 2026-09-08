#include "calculator/currency_converter.h"
#include "app.h"
#include "utils/shell_utils.h"
#include "utils/logger.h"
#include <shlobj.h>

namespace orca_light {

namespace {

constexpr const wchar_t* MUTEX_NAME = L"Local\\Orca-Light_Launcher_SingleInstance";

static HHOOK s_keyboard_hook = nullptr;
static Application* s_app_instance = nullptr;
static bool s_win_is_down = false;

static LRESULT CALLBACK LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam) {
    if (nCode == HC_ACTION) {
        KBDLLHOOKSTRUCT* pKbd = reinterpret_cast<KBDLLHOOKSTRUCT*>(lParam);
        if (pKbd->vkCode == VK_LWIN || pKbd->vkCode == VK_RWIN) {
            if (wParam == WM_KEYDOWN || wParam == WM_SYSKEYDOWN) {
                s_win_is_down = true;
            } else if (wParam == WM_KEYUP || wParam == WM_SYSKEYUP) {
                s_win_is_down = false;
            }
        } else if (pKbd->vkCode == VK_SPACE && (wParam == WM_KEYDOWN || wParam == WM_SYSKEYDOWN)) {
            if (s_win_is_down) {
                if (s_app_instance && s_app_instance->get_hwnd()) {
                    PostMessageW(s_app_instance->get_hwnd(), WM_USER + 101, 0, 0);
                    return 1; // Suppress default Windows IME/language bar
                }
            }
        }
    }
    return CallNextHookEx(s_keyboard_hook, nCode, wParam, lParam);
}

} // namespace

Application::Application() {
}

Application::~Application() {
    shutdown();
}

bool Application::initialize(HINSTANCE hInstance) {
    instance_ = hInstance;
    utils::Logger::init();
    utils::Logger::log("Orca-Light Application initializing...");

    // Single instance check
    mutex_handle_ = CreateMutexW(nullptr, TRUE, MUTEX_NAME);
    if (GetLastError() == ERROR_ALREADY_EXISTS) {
        utils::Logger::log("Another instance of Orca-Light is already running. Signaling it to open.");
        // Find existing instance window and tell it to display
        HWND existing_hwnd = FindWindowW(L"Orca-LightWindowClass", L"Orca-Light");
        if (existing_hwnd) {
            PostMessageW(existing_hwnd, WM_USER + 101, 0, 0);
            SetForegroundWindow(existing_hwnd);
        }
        if (mutex_handle_) {
            CloseHandle(mutex_handle_);
            mutex_handle_ = nullptr;
        }
        return false;
    }

    // Load configuration
    config_manager_.load();

    // Wire command registry callbacks
    command_registry_.set_exit_callback([this]() {
        PostQuitMessage(0);
    });

    command_registry_.set_reindex_callback([this]() {
        trigger_reindex();
    });

    command_registry_.set_clear_clipboard_callback([this]() {
        clipboard_history_.clear();
    });

    command_registry_.set_open_config_callback([this]() {
        open_settings_file();
    });

    // Create search engine
    search_engine_ = std::make_unique<search::SearchEngine>(
        app_indexer_,
        file_indexer_,
        clipboard_history_,
        command_registry_
    );

    // Create main window
    main_window_ = std::make_unique<ui::MainWindow>(*search_engine_);
    if (!main_window_->create()) {
        return false;
    }

    // Wire clipboard updates
    main_window_->set_clipboard_update_callback([this](HWND h) {
        clipboard_history_.on_clipboard_updated(h);
    });

    // Register global hotkey
    register_global_hotkey();

    // Install low-level hook for seamless Win + Space handling
    s_app_instance = this;
    install_keyboard_hook();

    // Fast startup: load existing index cache from disk first
    PWSTR appdata = nullptr;
    std::wstring cache_path = L"file_index.cache";
    if (SUCCEEDED(SHGetKnownFolderPath(FOLDERID_RoamingAppData, 0, nullptr, &appdata))) {
        cache_path = std::wstring(appdata) + L"\\Orca-Light\\file_index.cache";
        CoTaskMemFree(appdata);
    }
    file_indexer_.load_cache(cache_path);

    // Scan applications and start background file indexing
    CreateThread(nullptr, 0, [](LPVOID lpParam) -> DWORD {
        auto* self = static_cast<Application*>(lpParam);
        self->app_indexer_.scan_apps();
        const auto& c = self->config_manager_.get();
        self->file_indexer_.start_indexing(c.search_directories, c.exclude_patterns);
        return 0;
    }, this, 0, nullptr);

    // Currency updater thread (starts silently in background)
    CreateThread(nullptr, 0, [](LPVOID) -> DWORD {
        calc::CurrencyConverter::update_rates();
        return 0;
    }, nullptr, 0, nullptr);

    // Show window immediately on launch
    main_window_->show();

    return true;
}

HWND Application::get_hwnd() const {
    return main_window_ ? main_window_->get_hwnd() : nullptr;
}

void Application::register_global_hotkey() {
    if (!main_window_ || !main_window_->get_hwnd()) {
        return;
    }
    const auto& conf = config_manager_.get();
    // Primary configurable hotkey (Win + Space)
    if (!RegisterHotKey(main_window_->get_hwnd(), HOTKEY_ID, conf.hotkey_modifiers | MOD_NOREPEAT, conf.hotkey_vk)) {
        utils::Logger::log("Warning: Failed to register primary hotkey (Win + Space). It might be in use.");
    }
    // Secondary fallback hotkey (Ctrl + Space)
    if (!RegisterHotKey(main_window_->get_hwnd(), HOTKEY_ID + 1, MOD_CONTROL | MOD_NOREPEAT, VK_SPACE)) {
        utils::Logger::log("Warning: Failed to register fallback hotkey (Ctrl + Space).");
    }
}

void Application::unregister_global_hotkey() {
    if (main_window_ && main_window_->get_hwnd()) {
        UnregisterHotKey(main_window_->get_hwnd(), HOTKEY_ID);
        UnregisterHotKey(main_window_->get_hwnd(), HOTKEY_ID + 1);
    }
}

void Application::install_keyboard_hook() {
    if (!s_keyboard_hook) {
        s_keyboard_hook = SetWindowsHookExW(
            WH_KEYBOARD_LL,
            LowLevelKeyboardProc,
            GetModuleHandleW(nullptr),
            0
        );
    }
}

void Application::uninstall_keyboard_hook() {
    if (s_keyboard_hook) {
        UnhookWindowsHookEx(s_keyboard_hook);
        s_keyboard_hook = nullptr;
    }
}

void Application::toggle_window() {
    if (!main_window_) {
        return;
    }
    if (main_window_->is_visible()) {
        main_window_->hide();
    } else {
        main_window_->show();
    }
}

void Application::open_settings_file() {
    const std::wstring& path = config_manager_.get_config_path();
    utils::launch_item(path);
}

void Application::trigger_reindex() {
    const auto& conf = config_manager_.get();
    app_indexer_.scan_apps();
    file_indexer_.start_indexing(conf.search_directories, conf.exclude_patterns);
}

int Application::run() {
    MSG msg = {};
    while (GetMessageW(&msg, nullptr, 0, 0)) {
        if (msg.message == WM_HOTKEY && (msg.wParam == HOTKEY_ID || msg.wParam == HOTKEY_ID + 1)) {
            toggle_window();
            continue;
        }

        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }

    return static_cast<int>(msg.wParam);
}

void Application::shutdown() {
    uninstall_keyboard_hook();
    unregister_global_hotkey();

    if (main_window_) {
        main_window_->hide();
    }

    // Save cache on exit
    PWSTR appdata = nullptr;
    if (SUCCEEDED(SHGetKnownFolderPath(FOLDERID_RoamingAppData, 0, nullptr, &appdata))) {
        std::wstring cache_path = std::wstring(appdata) + L"\\Orca-Light\\file_index.cache";
        file_indexer_.save_cache(cache_path);
        CoTaskMemFree(appdata);
    }

    config_manager_.save();

    if (mutex_handle_) {
        CloseHandle(mutex_handle_);
        mutex_handle_ = nullptr;
    }
}

} // namespace orca_light
