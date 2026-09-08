#pragma once

#include <windows.h>
#include <string>
#include <string_view>
#include <vector>
#include <memory>
#include <functional>
#include <shared_mutex>
#include <unordered_map>
#include "../indexer/app_indexer.h"
#include "../indexer/file_indexer.h"
#include "../clipboard/clipboard_history.h"
#include "../system/system_commands.h"

namespace orca_light::search {

enum class ResultType {
    App,
    File,
    Folder,
    Calculator,
    Command,
    Clipboard
};

struct SearchResult {
    ResultType type = ResultType::App;
    std::wstring title;
    std::wstring subtitle;
    std::wstring path;
    std::wstring arguments;
    std::wstring badge;
    int32_t score = 0;
    std::vector<uint32_t> matched_indices;
    std::function<void(HWND hwnd)> custom_action;
};

class SearchEngine {
public:
    SearchEngine(indexer::AppIndexer& app_indexer,
                 indexer::FileIndexer& file_indexer,
                 clipboard::ClipboardHistory& clipboard_history,
                 system::SystemCommandRegistry& command_registry);

    // Perform instantaneous fuzzy search across all sources
    std::vector<SearchResult> query(std::wstring_view query_text, size_t max_results = 9);

    // Get default items when query is empty (recent/frequent apps and common commands)
    std::vector<SearchResult> get_default_results(size_t max_results = 9);

    // Mark an item as launched (MRU tracking)
    void record_launch(const std::wstring& path);

    bool is_indexing() const {
        return file_indexer_.is_indexing();
    }

private:
    void search_calculator(std::wstring_view query_text, std::vector<SearchResult>& out_results);
    void search_commands(std::wstring_view query_text, std::vector<SearchResult>& out_results, bool force_commands_only);
    void search_clipboard(std::wstring_view query_text, std::vector<SearchResult>& out_results);
    void search_apps(std::wstring_view query_text, std::vector<SearchResult>& out_results);
    void search_files(std::wstring_view query_text, std::vector<SearchResult>& out_results);

    indexer::AppIndexer& app_indexer_;
    indexer::FileIndexer& file_indexer_;
    clipboard::ClipboardHistory& clipboard_history_;
    system::SystemCommandRegistry& command_registry_;

    mutable std::shared_mutex launch_mutex_;
    std::unordered_map<std::wstring, uint32_t> launch_counts_;
};

} // namespace orca_light::search
