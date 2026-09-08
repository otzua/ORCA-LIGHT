#include "app_indexer.h"
#include "../utils/string_utils.h"
#include <shlobj.h>
#include <shlwapi.h>
#include <unordered_set>
#include <algorithm>
#include <memory>

namespace orca_light::indexer {

namespace {

bool is_ignored_shortcut(std::wstring_view name) {
    std::wstring lower = utils::to_lower(name);
    if (lower.find(L"uninstall") != std::wstring_view::npos ||
        lower.find(L"remove") != std::wstring_view::npos ||
        lower.find(L"readme") != std::wstring_view::npos ||
        lower.find(L"help") != std::wstring_view::npos ||
        lower.find(L"documentation") != std::wstring_view::npos ||
        lower.find(L"website") != std::wstring_view::npos ||
        lower.find(L"release notes") != std::wstring_view::npos) {
        return true;
    }
    return false;
}

} // namespace

AppIndexer::AppIndexer() : apps_(std::make_shared<std::vector<AppItem>>()) {
}

bool AppIndexer::resolve_shell_link(const std::wstring& lnk_path, AppItem& out_app) {
    IShellLinkW* psl = nullptr;
    HRESULT hr = CoCreateInstance(CLSID_ShellLink, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&psl));
    if (FAILED(hr) || !psl) {
        return false;
    }

    IPersistFile* ppf = nullptr;
    hr = psl->QueryInterface(IID_PPV_ARGS(&ppf));
    if (FAILED(hr) || !ppf) {
        psl->Release();
        return false;
    }

    hr = ppf->Load(lnk_path.c_str(), STGM_READ);
    if (SUCCEEDED(hr)) {
        wchar_t target[MAX_PATH] = {};
        wchar_t args[MAX_PATH] = {};
        wchar_t desc[MAX_PATH] = {};
        wchar_t icon_path[MAX_PATH] = {};
        int icon_idx = 0;

        WIN32_FIND_DATAW wfd = {};
        psl->GetPath(target, MAX_PATH, &wfd, SLGP_UNCPRIORITY);
        psl->GetArguments(args, MAX_PATH);
        psl->GetDescription(desc, MAX_PATH);
        psl->GetIconLocation(icon_path, MAX_PATH, &icon_idx);

        out_app.target_path = target;
        out_app.arguments = args;
        out_app.description = desc;
        out_app.shortcut_path = lnk_path;
        out_app.icon_path = (icon_path[0] != L'\0') ? icon_path : target;
        out_app.icon_index = icon_idx;

        // If target is empty, we still keep the shortcut path to execute via ShellExecute
        if (out_app.target_path.empty()) {
            out_app.target_path = lnk_path;
            out_app.icon_path = lnk_path;
        }
    }

    ppf->Release();
    psl->Release();
    return SUCCEEDED(hr);
}

void AppIndexer::scan_directory_shortcuts(const std::wstring& dir_path, std::vector<AppItem>& local_apps) {
    if (dir_path.empty()) {
        return;
    }

    std::vector<std::wstring> dirs_to_scan = { dir_path };

    while (!dirs_to_scan.empty()) {
        std::wstring current_dir = dirs_to_scan.back();
        dirs_to_scan.pop_back();

        std::wstring search_pattern = current_dir + L"\\*";
        WIN32_FIND_DATAW fd;
        HANDLE hFind = FindFirstFileExW(search_pattern.c_str(), FindExInfoBasic, &fd, FindExSearchNameMatch, nullptr, FIND_FIRST_EX_LARGE_FETCH);
        if (hFind == INVALID_HANDLE_VALUE) {
            continue;
        }

        do {
            if (wcscmp(fd.cFileName, L".") == 0 || wcscmp(fd.cFileName, L"..") == 0) {
                continue;
            }

            std::wstring full_path = current_dir + L"\\" + fd.cFileName;

            if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
                dirs_to_scan.push_back(full_path);
            } else {
                std::wstring ext = utils::to_lower(utils::get_extension(fd.cFileName));
                if (ext == L".lnk") {
                    std::wstring raw_name = utils::get_filename(fd.cFileName);
                    // Strip .lnk extension
                    if (raw_name.size() > 4) {
                        raw_name = raw_name.substr(0, raw_name.size() - 4);
                    }

                    if (is_ignored_shortcut(raw_name)) {
                        continue;
                    }

                    AppItem app;
                    app.name = raw_name;
                    if (resolve_shell_link(full_path, app)) {
                        local_apps.push_back(std::move(app));
                    }
                }
            }
        } while (FindNextFileW(hFind, &fd));

        FindClose(hFind);
    }
}

void AppIndexer::scan_registry_app_paths(std::vector<AppItem>& local_apps) {
    HKEY hKey = nullptr;
    const wchar_t* subkey = L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\App Paths";
    
    if (RegOpenKeyExW(HKEY_LOCAL_MACHINE, subkey, 0, KEY_READ, &hKey) == ERROR_SUCCESS) {
        DWORD index = 0;
        wchar_t key_name[256];
        DWORD key_name_len = 256;

        while (RegEnumKeyExW(hKey, index, key_name, &key_name_len, nullptr, nullptr, nullptr, nullptr) == ERROR_SUCCESS) {
            HKEY hSub = nullptr;
            if (RegOpenKeyExW(hKey, key_name, 0, KEY_READ, &hSub) == ERROR_SUCCESS) {
                wchar_t path_val[MAX_PATH] = {};
                DWORD val_len = sizeof(path_val);
                DWORD type = 0;
                if (RegQueryValueExW(hSub, nullptr, nullptr, &type, reinterpret_cast<LPBYTE>(path_val), &val_len) == ERROR_SUCCESS) {
                    if (path_val[0] != L'\0') {
                        std::wstring app_name = key_name;
                        if (app_name.size() > 4 && utils::to_lower(app_name.substr(app_name.size() - 4)) == L".exe") {
                            app_name = app_name.substr(0, app_name.size() - 4);
                        }

                        // Check if file exists
                        if (GetFileAttributesW(path_val) != INVALID_FILE_ATTRIBUTES) {
                            AppItem item;
                            item.name = app_name;
                            item.target_path = path_val;
                            item.icon_path = path_val;
                            local_apps.push_back(std::move(item));
                        }
                    }
                }
                RegCloseKey(hSub);
            }
            key_name_len = 256;
            ++index;
        }
        RegCloseKey(hKey);
    }
}

void AppIndexer::scan_known_tools(std::vector<AppItem>& local_apps) {
    wchar_t sys_dir[MAX_PATH] = {};
    GetSystemDirectoryW(sys_dir, MAX_PATH);
    std::wstring sys_path(sys_dir);

    struct KnownTool {
        const wchar_t* name;
        const wchar_t* exe;
        const wchar_t* desc;
    };

    const KnownTool tools[] = {
        { L"Notepad", L"\\notepad.exe", L"Text Editor" },
        { L"Calculator", L"\\calc.exe", L"Windows Calculator" },
        { L"Paint", L"\\mspaint.exe", L"Image Editor" },
        { L"Registry Editor", L"\\regedit.exe", L"Windows Registry Editor" },
        { L"File Explorer", L"\\..\\explorer.exe", L"Windows File Manager" },
        { L"Remote Desktop Connection", L"\\mstsc.exe", L"Connect to remote computers" },
        { L"Snipping Tool", L"\\SnippingTool.exe", L"Capture screenshots" },
        { L"Disk Cleanup", L"\\cleanmgr.exe", L"Free up disk space" },
        { L"Resource Monitor", L"\\resmon.exe", L"Real-time system resources" }
    };

    for (const auto& t : tools) {
        std::wstring target = sys_path + t.exe;
        if (GetFileAttributesW(target.c_str()) != INVALID_FILE_ATTRIBUTES) {
            AppItem item;
            item.name = t.name;
            item.description = t.desc;
            item.target_path = target;
            item.icon_path = target;
            local_apps.push_back(std::move(item));
        }
    }
}

void AppIndexer::scan_apps() {
    std::vector<AppItem> local_apps;

    PWSTR user_programs = nullptr;
    if (SUCCEEDED(SHGetKnownFolderPath(FOLDERID_Programs, 0, nullptr, &user_programs))) {
        scan_directory_shortcuts(user_programs, local_apps);
        CoTaskMemFree(user_programs);
    }

    PWSTR common_programs = nullptr;
    if (SUCCEEDED(SHGetKnownFolderPath(FOLDERID_CommonPrograms, 0, nullptr, &common_programs))) {
        scan_directory_shortcuts(common_programs, local_apps);
        CoTaskMemFree(common_programs);
    }

    PWSTR user_desktop = nullptr;
    if (SUCCEEDED(SHGetKnownFolderPath(FOLDERID_Desktop, 0, nullptr, &user_desktop))) {
        scan_directory_shortcuts(user_desktop, local_apps);
        CoTaskMemFree(user_desktop);
    }

    PWSTR common_desktop = nullptr;
    if (SUCCEEDED(SHGetKnownFolderPath(FOLDERID_PublicDesktop, 0, nullptr, &common_desktop))) {
        scan_directory_shortcuts(common_desktop, local_apps);
        CoTaskMemFree(common_desktop);
    }

    scan_registry_app_paths(local_apps);
    scan_known_tools(local_apps);

    // Deduplicate by case-insensitive name and target path
    std::unordered_set<std::wstring> seen;
    auto unique_apps = std::make_shared<std::vector<AppItem>>();
    unique_apps->reserve(local_apps.size());

    for (auto& item : local_apps) {
        std::wstring key = utils::to_lower(item.name) + L"|" + utils::to_lower(item.target_path);
        if (seen.find(key) == seen.end()) {
            seen.insert(key);
            unique_apps->push_back(std::move(item));
        }
    }

    std::lock_guard<std::mutex> lock(mutex_);
    apps_ = unique_apps;
}

std::shared_ptr<const std::vector<AppItem>> AppIndexer::get_apps() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return apps_;
}

} // namespace orca_light::indexer
