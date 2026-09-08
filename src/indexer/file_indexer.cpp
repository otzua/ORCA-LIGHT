#include "file_indexer.h"
#include "../utils/string_utils.h"
#include <fstream>
#include <algorithm>
#include <filesystem>

namespace orca_light::indexer {

namespace {

constexpr uint32_t CACHE_MAGIC = 0x53504F54; // "SPOT"
constexpr uint32_t CACHE_VERSION = 1;

void write_wstring(std::ofstream& ofs, const std::wstring& str) {
    uint32_t len = static_cast<uint32_t>(str.size());
    ofs.write(reinterpret_cast<const char*>(&len), sizeof(len));
    if (len > 0) {
        ofs.write(reinterpret_cast<const char*>(str.data()), len * sizeof(wchar_t));
    }
}

bool read_wstring(std::ifstream& ifs, std::wstring& str) {
    uint32_t len = 0;
    if (!ifs.read(reinterpret_cast<char*>(&len), sizeof(len))) {
        return false;
    }
    str.resize(len);
    if (len > 0) {
        if (!ifs.read(reinterpret_cast<char*>(str.data()), len * sizeof(wchar_t))) {
            return false;
        }
    }
    return true;
}

} // namespace

FileIndexer::FileIndexer() : items_(std::make_shared<std::vector<FileItem>>()) {
}

FileIndexer::~FileIndexer() {
    stop_indexing();
}

void FileIndexer::stop_indexing() {
    stop_requested_.store(true);
    if (worker_thread_) {
        WaitForSingleObject(worker_thread_, INFINITE);
        CloseHandle(worker_thread_);
        worker_thread_ = nullptr;
    }
}

bool FileIndexer::is_excluded(std::wstring_view path, const std::vector<std::wstring>& exclude_patterns) const {
    std::wstring lower_path = utils::to_lower(path);
    for (const auto& pattern : exclude_patterns) {
        std::wstring lower_pat = utils::to_lower(pattern);
        if (lower_path.find(lower_pat) != std::wstring::npos) {
            return true;
        }
    }
    return false;
}

void FileIndexer::scan_directory_recursive(const std::wstring& dir_path, 
                                          const std::vector<std::wstring>& exclude_patterns,
                                          std::vector<FileItem>& local_batch) {
    std::vector<std::wstring> stack;
    stack.push_back(dir_path);

    while (!stack.empty() && !stop_requested_.load()) {
        std::wstring current = stack.back();
        stack.pop_back();

        if (is_excluded(current, exclude_patterns)) {
            continue;
        }

        std::wstring search_pattern = current + L"\\*";
        WIN32_FIND_DATAW fd;
        HANDLE hFind = FindFirstFileExW(
            search_pattern.c_str(),
            FindExInfoBasic,
            &fd,
            FindExSearchNameMatch,
            nullptr,
            FIND_FIRST_EX_LARGE_FETCH
        );

        if (hFind == INVALID_HANDLE_VALUE) {
            continue;
        }

        do {
            if (wcscmp(fd.cFileName, L".") == 0 || wcscmp(fd.cFileName, L"..") == 0) {
                continue;
            }

            // Skip hidden or system files like desktop.ini, thumbs.db
            if (fd.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN || fd.dwFileAttributes & FILE_ATTRIBUTE_SYSTEM) {
                if (fd.cFileName[0] == L'.') {
                    continue;
                }
            }

            std::wstring full_path = current + L"\\" + fd.cFileName;

            if (is_excluded(full_path, exclude_patterns)) {
                continue;
            }

            FileItem item;
            item.name = fd.cFileName;
            item.path = full_path;
            item.is_directory = (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0;
            item.size = (static_cast<uint64_t>(fd.nFileSizeHigh) << 32) | fd.nFileSizeLow;
            item.modified_time = (static_cast<uint64_t>(fd.ftLastWriteTime.dwHighDateTime) << 32) | fd.ftLastWriteTime.dwLowDateTime;

            local_batch.push_back(std::move(item));

            if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
                stack.push_back(full_path);
            }
            // Sleep frequently to yield CPU/IO to the rest of the OS, and update live feed
            if (local_batch.size() % 20 == 0) {
                Sleep(1);
                std::lock_guard<std::mutex> lock(feed_mutex_);
                recent_scanned_paths_.push_back(full_path);
                if (recent_scanned_paths_.size() > 20) {
                    recent_scanned_paths_.erase(recent_scanned_paths_.begin());
                }
            }
        } while (FindNextFileW(hFind, &fd) && !stop_requested_.load());

        FindClose(hFind);
    }
}

void FileIndexer::indexing_worker(std::vector<std::wstring> directories, 
                                  std::vector<std::wstring> exclude_patterns) {
    // Drop priority to absolute lowest to prevent ANY lag on low-end PCs
    SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_LOWEST);
    
    // Give the app a small breather, but mostly start instantly on launch
    Sleep(50);
    
    is_indexing_.store(true);

    std::vector<FileItem> local_batch;
    local_batch.reserve(50000);

    for (const auto& dir : directories) {
        if (stop_requested_.load()) {
            break;
        }
        scan_directory_recursive(dir, exclude_patterns, local_batch);
    }

    if (!stop_requested_.load()) {
        auto new_items = std::make_shared<std::vector<FileItem>>(std::move(local_batch));
        std::lock_guard<std::mutex> lock(mutex_);
        items_ = new_items;
        if (progress_cb_) {
            progress_cb_(items_->size());
        }
    }

    is_indexing_.store(false);
}

struct IndexerThreadArgs {
    FileIndexer* instance;
    std::vector<std::wstring> directories;
    std::vector<std::wstring> exclude_patterns;
};

void FileIndexer::start_indexing(const std::vector<std::wstring>& directories, 
                                const std::vector<std::wstring>& exclude_patterns) {
    stop_indexing();
    stop_requested_.store(false);

    auto* args = new IndexerThreadArgs{this, directories, exclude_patterns};
    worker_thread_ = CreateThread(nullptr, 0, [](LPVOID lpParam) -> DWORD {
        auto* a = static_cast<IndexerThreadArgs*>(lpParam);
        a->instance->indexing_worker(a->directories, a->exclude_patterns);
        delete a;
        return 0;
    }, args, 0, nullptr);
}

size_t FileIndexer::get_total_indexed() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return items_->size();
}

std::shared_ptr<const std::vector<FileItem>> FileIndexer::get_all_items() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return items_;
}

std::vector<std::wstring> FileIndexer::get_live_feed(size_t max_count) const {
    std::lock_guard<std::mutex> lock(feed_mutex_);
    if (recent_scanned_paths_.empty()) return {};
    size_t count = std::min(recent_scanned_paths_.size(), max_count);
    return std::vector<std::wstring>(recent_scanned_paths_.end() - count, recent_scanned_paths_.end());
}

std::vector<FileItem> FileIndexer::search(std::wstring_view query, size_t max_results) const {
    std::shared_ptr<const std::vector<FileItem>> current_items;
    {
        std::lock_guard<std::mutex> lock(mutex_);
        current_items = items_;
    }

    if (query.empty()) {
        return {};
    }

    std::wstring lower_query = utils::to_lower(query);
    std::vector<FileItem> matches;
    matches.reserve(max_results);

    for (const auto& item : *current_items) {
        std::wstring lower_name = utils::to_lower(item.name);
        if (lower_name.find(lower_query) != std::wstring::npos) {
            matches.push_back(item);
            if (matches.size() >= max_results) {
                break;
            }
        }
    }

    return matches;
}

bool FileIndexer::save_cache(const std::wstring& cache_file_path) {
    std::shared_ptr<const std::vector<FileItem>> current_items;
    {
        std::lock_guard<std::mutex> lock(mutex_);
        current_items = items_;
    }

    std::filesystem::path path(cache_file_path);
    std::ofstream ofs(path, std::ios::binary);
    if (!ofs.is_open()) {
        return false;
    }

    uint32_t magic = CACHE_MAGIC;
    uint32_t version = CACHE_VERSION;
    uint32_t count = static_cast<uint32_t>(current_items->size());

    ofs.write(reinterpret_cast<const char*>(&magic), sizeof(magic));
    ofs.write(reinterpret_cast<const char*>(&version), sizeof(version));
    ofs.write(reinterpret_cast<const char*>(&count), sizeof(count));

    for (const auto& item : *current_items) {
        write_wstring(ofs, item.name);
        write_wstring(ofs, item.path);
        ofs.write(reinterpret_cast<const char*>(&item.size), sizeof(item.size));
        ofs.write(reinterpret_cast<const char*>(&item.modified_time), sizeof(item.modified_time));
        uint8_t is_dir = item.is_directory ? 1 : 0;
        ofs.write(reinterpret_cast<const char*>(&is_dir), sizeof(is_dir));
    }

    return true;
}

bool FileIndexer::load_cache(const std::wstring& cache_file_path) {
    std::filesystem::path path(cache_file_path);
    std::ifstream ifs(path, std::ios::binary);
    if (!ifs.is_open()) {
        return false;
    }

    uint32_t magic = 0;
    uint32_t version = 0;
    uint32_t count = 0;

    if (!ifs.read(reinterpret_cast<char*>(&magic), sizeof(magic)) || magic != CACHE_MAGIC) {
        return false;
    }
    if (!ifs.read(reinterpret_cast<char*>(&version), sizeof(version)) || version != CACHE_VERSION) {
        return false;
    }
    if (!ifs.read(reinterpret_cast<char*>(&count), sizeof(count))) {
        return false;
    }

    std::vector<FileItem> loaded;
    loaded.reserve(count);

    for (uint32_t i = 0; i < count; ++i) {
        FileItem item;
        if (!read_wstring(ifs, item.name)) return false;
        if (!read_wstring(ifs, item.path)) return false;
        if (!ifs.read(reinterpret_cast<char*>(&item.size), sizeof(item.size))) return false;
        if (!ifs.read(reinterpret_cast<char*>(&item.modified_time), sizeof(item.modified_time))) return false;
        uint8_t is_dir = 0;
        if (!ifs.read(reinterpret_cast<char*>(&is_dir), sizeof(is_dir))) return false;
        item.is_directory = (is_dir != 0);

        loaded.push_back(std::move(item));
    }

    auto new_items = std::make_shared<std::vector<FileItem>>(std::move(loaded));
    std::lock_guard<std::mutex> lock(mutex_);
    items_ = new_items;
    return true;
}

} // namespace orca_light::indexer
