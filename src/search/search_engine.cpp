#include "search_engine.h"
#include "fuzzy_matcher.h"
#include "../calculator/calc_engine.h"
#include "../calculator/currency_converter.h"
#include "../utils/string_utils.h"
#include "../utils/shell_utils.h"
#include <algorithm>
#include <sstream>
#include <iomanip>

namespace orca_light::search {

SearchEngine::SearchEngine(indexer::AppIndexer& app_indexer,
                           indexer::FileIndexer& file_indexer,
                           clipboard::ClipboardHistory& clipboard_history,
                           system::SystemCommandRegistry& command_registry)
    : app_indexer_(app_indexer),
      file_indexer_(file_indexer),
      clipboard_history_(clipboard_history),
      command_registry_(command_registry) {
}

void SearchEngine::record_launch(const std::wstring& path) {
    if (!path.empty()) {
        std::unique_lock<std::shared_mutex> lock(launch_mutex_);
        launch_counts_[utils::to_lower(path)]++;
    }
}

void SearchEngine::search_calculator(std::wstring_view query_text, std::vector<SearchResult>& out_results) {
    auto calc_res = calc::CalcEngine::evaluate(query_text);
    if (calc_res.has_value()) {
        SearchResult res;
        res.type = ResultType::Calculator;
        res.title = calc_res->formatted_result;
        res.subtitle = L"= " + calc_res->expression;
        res.badge = L"CALC";
        res.score = 50000; // Pin to top
        res.custom_action = [result_val = calc_res->formatted_result](HWND hwnd) {
            utils::set_clipboard_text(hwnd, result_val);
        };
        out_results.push_back(std::move(res));
    }

    auto curr_res = calc::CurrencyConverter::convert(query_text);
    if (curr_res.has_value()) {
        SearchResult res;
        res.type = ResultType::Calculator;
        
        std::wstringstream ss;
        ss << std::fixed << std::setprecision(2) << curr_res->result << L" " << curr_res->to_currency;
        res.title = ss.str();
        
        std::wstringstream sub_ss;
        // avoid scientific notation for amount
        sub_ss << std::fixed << std::setprecision(2) << curr_res->amount << L" " << curr_res->from_currency << L" = " << res.title;
        res.subtitle = sub_ss.str();
        
        res.badge = L"CURRENCY";
        res.score = 50000; // Pin to top
        res.custom_action = [result_val = res.title](HWND hwnd) {
            utils::set_clipboard_text(hwnd, result_val);
        };
        out_results.push_back(std::move(res));
    }
}

void SearchEngine::search_commands(std::wstring_view query_text, std::vector<SearchResult>& out_results, bool force_commands_only) {
    const auto& commands = command_registry_.get_all_commands();

    for (const auto& cmd : commands) {
        int32_t best_score = -10000;
        std::vector<uint32_t> matched_idx;

        // Match against command name
        auto name_match = FuzzyMatcher::match(query_text, cmd.name);
        if (name_match.matched) {
            best_score = name_match.score + 1000;
            matched_idx = std::move(name_match.matched_indices);
        }

        // Match against keywords/aliases
        for (const auto& kw : cmd.keywords) {
            auto kw_match = FuzzyMatcher::match(query_text, kw);
            if (kw_match.matched) {
                int32_t s = kw_match.score + 800;
                if (s > best_score) {
                    best_score = s;
                }
            }
        }

        if (force_commands_only || best_score > 0) {
            SearchResult res;
            res.type = ResultType::Command;
            res.title = cmd.name;
            res.subtitle = cmd.description;
            res.badge = L"CMD";
            res.score = force_commands_only ? (best_score > 0 ? best_score : 100) : best_score;
            res.matched_indices = std::move(matched_idx);
            res.custom_action = cmd.execute;
            out_results.push_back(std::move(res));
        }
    }
}

void SearchEngine::search_clipboard(std::wstring_view query_text, std::vector<SearchResult>& out_results) {
    auto clip_items = clipboard_history_.search(query_text);
    for (const auto& item : clip_items) {
        SearchResult res;
        res.type = ResultType::Clipboard;
        res.title = item.preview;
        res.subtitle = L"Copy text to clipboard";
        res.badge = L"CLIP";
        res.score = 2500;
        res.custom_action = [text = item.text](HWND hwnd) {
            utils::set_clipboard_text(hwnd, text);
        };
        out_results.push_back(std::move(res));
    }
}

void SearchEngine::search_apps(std::wstring_view query_text, std::vector<SearchResult>& out_results) {
    auto apps = app_indexer_.get_apps();
    std::shared_lock<std::shared_mutex> lock(launch_mutex_);

    for (const auto& app : *apps) {
        auto match = FuzzyMatcher::match(query_text, app.name);
        if (match.matched) {
            SearchResult res;
            res.type = ResultType::App;
            res.title = app.name;
            res.subtitle = !app.description.empty() ? app.description : app.target_path;
            res.path = app.target_path;
            res.arguments = app.arguments;
            res.badge = L"APP";

            // App priority bonus
            int32_t score = match.score + 2000;

            // Recency / launch frequency bonus
            auto count_it = launch_counts_.find(utils::to_lower(app.target_path));
            if (count_it != launch_counts_.end()) {
                score += static_cast<int32_t>(std::min<uint32_t>(count_it->second * 100, 1500));
            }

            res.score = score;
            res.matched_indices = std::move(match.matched_indices);
            out_results.push_back(std::move(res));
        }
    }
}

void SearchEngine::search_files(std::wstring_view query_text, std::vector<SearchResult>& out_results) {
    auto files = file_indexer_.get_all_items();
    std::shared_lock<std::shared_mutex> lock(launch_mutex_);

    for (const auto& item : *files) {
        auto match = FuzzyMatcher::match(query_text, item.name);
        if (match.matched) {
            SearchResult res;
            res.type = item.is_directory ? ResultType::Folder : ResultType::File;
            res.title = item.name;
            res.subtitle = item.path;
            res.path = item.path;
            res.badge = item.is_directory ? L"DIR" : L"FILE";

            int32_t score = match.score;
            if (item.is_directory) {
                score += 300; // Prefer directories slightly over arbitrary files
            }

            auto count_it = launch_counts_.find(utils::to_lower(item.path));
            if (count_it != launch_counts_.end()) {
                score += static_cast<int32_t>(std::min<uint32_t>(count_it->second * 100, 1000));
            }

            res.score = score;
            res.matched_indices = std::move(match.matched_indices);
            out_results.push_back(std::move(res));
        }
    }
}

std::vector<SearchResult> SearchEngine::query(std::wstring_view query_text, size_t max_results) {
    std::wstring trimmed(utils::trim(query_text));
    if (trimmed.empty()) {
        return get_default_results(max_results);
    }

    std::vector<SearchResult> results;
    results.reserve(64);

    // Check for command prefix '>'
    if (trimmed.front() == L'>') {
        std::wstring_view cmd_query = trimmed.substr(1);
        cmd_query = utils::trim(cmd_query);
        search_commands(cmd_query, results, true);
        std::sort(results.begin(), results.end(), [](const SearchResult& a, const SearchResult& b) {
            return a.score > b.score;
        });
        if (results.size() > max_results) {
            results.resize(max_results);
        }
        return results;
    }

    // Check for clipboard prefix 'cb ' or '@clip'
    if (utils::starts_with_case_insensitive(trimmed, L"cb ") ||
        utils::starts_with_case_insensitive(trimmed, L"@clip")) {
        size_t offset = (trimmed[0] == L'@') ? 5 : 3;
        std::wstring_view clip_query = utils::trim(trimmed.substr(offset));
        search_clipboard(clip_query, results);
        if (results.size() > max_results) {
            results.resize(max_results);
        }
        return results;
    }

    // Calculator check
    search_calculator(trimmed, results);

    // Search Apps
    search_apps(trimmed, results);

    // Search Commands
    search_commands(trimmed, results, false);

    // Search Files
    search_files(trimmed, results);

    // Stable sort by score descending
    std::stable_sort(results.begin(), results.end(), [](const SearchResult& a, const SearchResult& b) {
        return a.score > b.score;
    });

    if (results.size() > max_results) {
        results.resize(max_results);
    }

    return results;
}

std::vector<SearchResult> SearchEngine::get_default_results(size_t max_results) {
    std::vector<SearchResult> results;

    // Show top apps
    auto apps = app_indexer_.get_apps();
    std::shared_lock<std::shared_mutex> lock(launch_mutex_);

    for (const auto& app : *apps) {
        SearchResult res;
        res.type = ResultType::App;
        res.title = app.name;
        res.subtitle = !app.description.empty() ? app.description : app.target_path;
        res.path = app.target_path;
        res.arguments = app.arguments;
        res.badge = L"APP";

        auto count_it = launch_counts_.find(utils::to_lower(app.target_path));
        res.score = (count_it != launch_counts_.end()) ? static_cast<int32_t>(count_it->second * 100) : 0;
        results.push_back(std::move(res));
    }

    std::stable_sort(results.begin(), results.end(), [](const SearchResult& a, const SearchResult& b) {
        return a.score > b.score;
    });

    if (results.size() > max_results) {
        results.resize(max_results);
    }

    // Live Feed Hack: if we are indexing, prepend the live feed paths
    if (file_indexer_.is_indexing()) {
        auto live = file_indexer_.get_live_feed(max_results);
        if (!live.empty()) {
            results.clear(); // Replace defaults with live feed
            for (auto it = live.rbegin(); it != live.rend(); ++it) {
                SearchResult res;
                res.type = ResultType::File;
                size_t slash = it->find_last_of(L"\\/");
                res.title = (slash != std::wstring::npos) ? it->substr(slash + 1) : *it;
                res.subtitle = *it;
                res.path = *it;
                res.badge = L"SCAN";
                results.push_back(std::move(res));
                if (results.size() >= max_results) break;
            }
        }
    }

    // If few or no apps, add frequent system commands
    if (results.size() < 4) {
        const auto& commands = command_registry_.get_all_commands();
        for (const auto& cmd : commands) {
            if (results.size() >= max_results) break;
            SearchResult res;
            res.type = ResultType::Command;
            res.title = cmd.name;
            res.subtitle = cmd.description;
            res.badge = L"CMD";
            res.custom_action = cmd.execute;
            results.push_back(std::move(res));
        }
    }

    return results;
}

} // namespace orca_light::search
