#pragma once

#include <windows.h>
#include <string>
#include <vector>
#include <memory>
#include <functional>
#include "renderer.h"
#include "../search/search_engine.h"

namespace orca_light::ui {

class MainWindow {
public:
    MainWindow(search::SearchEngine& search_engine);
    ~MainWindow();

    bool create();
    void show();
    void hide();
    void toggle_visibility();
    bool is_visible() const;

    HWND get_hwnd() const { return hwnd_; }

    // Callbacks
    void set_clipboard_update_callback(std::function<void(HWND)> cb) {
        on_clipboard_update_ = cb;
    }

private:
    static LRESULT CALLBACK window_proc_static(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
    LRESULT window_proc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

    void update_layout();
    void center_on_active_monitor();
    void bring_to_foreground();
    void execute_selected_primary();
    void execute_selected_secondary();
    void execute_selected_copy_path();
    void execute_selected_admin();

    void perform_search();

    HWND hwnd_ = nullptr;
    Renderer renderer_;
    search::SearchEngine& search_engine_;

    std::wstring current_query_;
    size_t cursor_pos_ = 0;
    bool show_cursor_ = true;

    std::vector<search::SearchResult> current_results_;
    int selected_index_ = 0;
    int hovered_index_ = -1;

    std::function<void(HWND)> on_clipboard_update_;
    bool is_visible_ = false;
    bool just_shown_ = false;

    // Tray Icon
    void setup_tray_icon();
    void remove_tray_icon();
    void show_tray_menu();

    // Animations & Glass
    void enable_glass();
    float anim_progress_ = 0.0f;
    ULONGLONG anim_start_time_ = 0;
};

} // namespace orca_light::ui
