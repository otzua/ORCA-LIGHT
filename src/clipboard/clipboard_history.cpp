#include "clipboard_history.h"
#include "../utils/shell_utils.h"
#include "../utils/string_utils.h"
#include <algorithm>
#include <cwctype>

namespace orca_light::clipboard {

ClipboardHistory::ClipboardHistory(size_t max_items)
    : max_items_(max_items) {
}

std::wstring ClipboardHistory::sanitize_preview(std::wstring_view text) const {
    std::wstring preview;
    preview.reserve(std::min<size_t>(text.size(), 120));

    bool prev_space = false;
    for (wchar_t c : text) {
        if (preview.size() >= 100) {
            preview += L"...";
            break;
        }
        if (c == L'\r' || c == L'\n' || c == L'\t' || c == L' ') {
            if (!prev_space && !preview.empty()) {
                preview.push_back(L' ');
                prev_space = true;
            }
        } else {
            preview.push_back(c);
            prev_space = false;
        }
    }
    return preview;
}

void ClipboardHistory::on_clipboard_updated(HWND hwnd) {
    std::wstring current_text = utils::get_clipboard_text(hwnd);
    if (current_text.empty()) {
        return;
    }

    std::wstring trimmed = utils::trim(current_text);
    if (trimmed.empty()) {
        return;
    }

    std::lock_guard<std::mutex> lock(mutex_);

    // Do not re-add if identical to the most recent entry
    if (trimmed == last_text_) {
        return;
    }

    last_text_ = trimmed;

    // Check if item already exists in history; if so, remove old occurrence and bring to front
    for (auto it = items_.begin(); it != items_.end(); ++it) {
        if (it->text == trimmed) {
            items_.erase(it);
            break;
        }
    }

    ClipboardItem item;
    item.text = trimmed;
    item.preview = sanitize_preview(trimmed);
    item.timestamp = std::chrono::system_clock::now();
    item.id = next_id_++;

    items_.push_front(std::move(item));

    while (items_.size() > max_items_) {
        items_.pop_back();
    }
}

std::vector<ClipboardItem> ClipboardHistory::get_items() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return std::vector<ClipboardItem>(items_.begin(), items_.end());
}

std::vector<ClipboardItem> ClipboardHistory::search(std::wstring_view query) const {
    std::lock_guard<std::mutex> lock(mutex_);
    if (query.empty()) {
        return std::vector<ClipboardItem>(items_.begin(), items_.end());
    }

    std::wstring lower_query = utils::to_lower(query);
    std::vector<ClipboardItem> results;

    for (const auto& item : items_) {
        std::wstring lower_preview = utils::to_lower(item.preview);
        if (lower_preview.find(lower_query) != std::wstring::npos) {
            results.push_back(item);
        } else {
            std::wstring lower_text = utils::to_lower(item.text);
            if (lower_text.find(lower_query) != std::wstring::npos) {
                results.push_back(item);
            }
        }
    }
    return results;
}

void ClipboardHistory::clear() {
    std::lock_guard<std::mutex> lock(mutex_);
    items_.clear();
    last_text_.clear();
}

void ClipboardHistory::set_max_items(size_t count) {
    std::lock_guard<std::mutex> lock(mutex_);
    max_items_ = count;
    while (items_.size() > max_items_) {
        items_.pop_back();
    }
}

} // namespace orca_light::clipboard
