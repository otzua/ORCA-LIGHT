#include "logger.h"
#include "string_utils.h"
#include <shlobj.h>
#include <fstream>
#include <chrono>
#include <iomanip>
#include <sstream>
#include <filesystem>

namespace orca_light::utils {

namespace {

std::wstring s_log_path;
bool s_initialized = false;

std::string current_timestamp() {
    auto now = std::chrono::system_clock::now();
    auto in_time_t = std::chrono::system_clock::to_time_t(now);
    std::tm tm_buf{};
#if defined(_WIN32)
    localtime_s(&tm_buf, &in_time_t);
#else
    localtime_r(&in_time_t, &tm_buf);
#endif
    std::stringstream ss;
    ss << std::put_time(&tm_buf, "%Y-%m-%d %H:%M:%S");
    return ss.str();
}

} // namespace

void Logger::init() {
    if (s_initialized) return;

    PWSTR localappdata = nullptr;
    if (SUCCEEDED(SHGetKnownFolderPath(FOLDERID_LocalAppData, 0, nullptr, &localappdata))) {
        std::wstring dir = std::wstring(localappdata) + L"\\Orca-Light";
        CreateDirectoryW(dir.c_str(), nullptr);
        s_log_path = dir + L"\\orca-light.log";
        CoTaskMemFree(localappdata);
    } else {
        s_log_path = L"orca-light.log";
    }

    s_initialized = true;

    // Reset log file for fresh session
    std::filesystem::path p(s_log_path);
    std::ofstream ofs(p, std::ios::trunc);
    if (ofs.is_open()) {
        ofs << "=== Orca-Light Started at " << current_timestamp() << " ===\n";
    }
}

const std::wstring& Logger::get_log_path() {
    if (!s_initialized) init();
    return s_log_path;
}

void Logger::log(std::string_view message) {
    if (!s_initialized) init();

    std::filesystem::path p(s_log_path);
    std::ofstream ofs(p, std::ios::app);
    if (ofs.is_open()) {
        ofs << "[" << current_timestamp() << "] " << message << "\n";
    }
}

void Logger::log(std::wstring_view message) {
    log(wide_to_utf8(message));
}

void Logger::error(std::wstring_view message, HRESULT hr) {
    std::wstringstream wss;
    wss << message;
    if (FAILED(hr)) {
        wss << L" (HRESULT: 0x" << std::hex << hr << L")";
    }
    log(wss.str());

    std::wstring dialog_msg = wss.str() + L"\n\nLog file: " + s_log_path;
    MessageBoxW(nullptr, dialog_msg.c_str(), L"Orca-Light Error", MB_ICONERROR | MB_OK);
}

} // namespace orca_light::utils
