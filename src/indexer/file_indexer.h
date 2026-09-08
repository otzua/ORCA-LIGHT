#pragma once

#include <windows.h>
#include <string>
#include <vector>
#include <mutex>
#include <atomic>
#include <thread>
#include <functional>

namespace orca_light::indexer {

struct FileItem {
    std::wstring name;
    std::wstring path;
    uint64_t size = 0;
    uint64_t modified_time = 0;
    bool is_directory = false;
};

class FileIndexer {
public:
    FileIndexer();
    ~FileIndexer();

    // Start background indexing of specified directories
    void start_indexing(const std::vector<std::wstring>& directories, 
                        const std::vector<std::wstring>& exclude_patterns);

    // Stop active indexing thread
    void stop_indexing();

    // Query files matching a pattern
    std::vector<FileItem> search(std::wstring_view query, size_t max_results = 50) const;

    // Direct access to all indexed items
    std::vector<FileItem> get_all_items() const;

    // Cache persistence
    bool save_cache(const std::wstring& cache_file_path);
    bool load_cache(const std::wstring& cache_file_path);

    bool is_indexing() const { return is_indexing_.load(); }
    size_t get_total_indexed() const;

    void set_progress_callback(std::function<void(size_t count)> cb) {
        progress_cb_ = cb;
    }

private:
    void indexing_worker(std::vector<std::wstring> directories, 
                         std::vector<std::wstring> exclude_patterns);

    void scan_directory_recursive(const std::wstring& dir_path, 
                                 const std::vector<std::wstring>& exclude_patterns,
                                 std::vector<FileItem>& local_batch);

    bool is_excluded(std::wstring_view path, const std::vector<std::wstring>& exclude_patterns) const;

    mutable std::mutex mutex_;
    std::vector<FileItem> items_;
    HANDLE worker_thread_ = nullptr;
    std::atomic<bool> stop_requested_{ false };
    std::atomic<bool> is_indexing_{ false };
    std::function<void(size_t)> progress_cb_;
};

} // namespace orca_light::indexer
