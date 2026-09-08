#pragma once

#include <windows.h>
#include <string>
#include <vector>
#include <deque>
#include <mutex>
#include <chrono>

namespace orca_light::clipboard {

struct ClipboardItem {
    std::wstring text;
    std::wstring preview;
    std::chrono::system_clock::time_point timestamp;
    uint64_t id = 0;
};

class ClipboardHistory {
public:
    explicit ClipboardHistory(size_t max_items = 50);

    // Called on WM_CLIPBOARDUPDATE
    void on_clipboard_updated(HWND hwnd);

    // Retrieve items (thread-safe copy)
    std::vector<ClipboardItem> get_items() const;

    // Search clipboard history
    std::vector<ClipboardItem> search(std::wstring_view query) const;

    // Erase all items
    void clear();

    // Set maximum items
    void set_max_items(size_t count);

private:
    std::wstring sanitize_preview(std::wstring_view text) const;

    mutable std::mutex mutex_;
    std::deque<ClipboardItem> items_;
    size_t max_items_;
    uint64_t next_id_ = 1;
    std::wstring last_text_;
};

} // namespace orca_light::clipboard
