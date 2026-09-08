#include "string_utils.h"
#include <windows.h>
#include <algorithm>
#include <cwctype>
#include <sstream>
#include <iomanip>

namespace orca_light::utils {

std::wstring utf8_to_wide(std::string_view utf8_str) {
    if (utf8_str.empty()) {
        return L"";
    }
    int len = MultiByteToWideChar(CP_UTF8, 0, utf8_str.data(), static_cast<int>(utf8_str.size()), nullptr, 0);
    if (len <= 0) {
        return L"";
    }
    std::wstring result(len, L'\0');
    MultiByteToWideChar(CP_UTF8, 0, utf8_str.data(), static_cast<int>(utf8_str.size()), result.data(), len);
    return result;
}

std::string wide_to_utf8(std::wstring_view wide_str) {
    if (wide_str.empty()) {
        return "";
    }
    int len = WideCharToMultiByte(CP_UTF8, 0, wide_str.data(), static_cast<int>(wide_str.size()), nullptr, 0, nullptr, nullptr);
    if (len <= 0) {
        return "";
    }
    std::string result(len, '\0');
    WideCharToMultiByte(CP_UTF8, 0, wide_str.data(), static_cast<int>(wide_str.size()), result.data(), len, nullptr, nullptr);
    return result;
}

std::wstring to_lower(std::wstring_view str) {
    std::wstring result;
    result.reserve(str.size());
    for (wchar_t c : str) {
        result.push_back(static_cast<wchar_t>(towlower(c)));
    }
    return result;
}

std::string to_lower(std::string_view str) {
    std::string result;
    result.reserve(str.size());
    for (char c : str) {
        result.push_back(static_cast<char>(tolower(static_cast<unsigned char>(c))));
    }
    return result;
}

wchar_t to_lower_char(wchar_t c) {
    return static_cast<wchar_t>(towlower(c));
}

std::wstring trim(std::wstring_view str) {
    size_t first = 0;
    while (first < str.size() && iswspace(str[first])) {
        ++first;
    }
    if (first == str.size()) {
        return L"";
    }
    size_t last = str.size() - 1;
    while (last > first && iswspace(str[last])) {
        --last;
    }
    return std::wstring(str.substr(first, last - first + 1));
}

std::string trim(std::string_view str) {
    size_t first = 0;
    while (first < str.size() && isspace(static_cast<unsigned char>(str[first]))) {
        ++first;
    }
    if (first == str.size()) {
        return "";
    }
    size_t last = str.size() - 1;
    while (last > first && isspace(static_cast<unsigned char>(str[last]))) {
        --last;
    }
    return std::string(str.substr(first, last - first + 1));
}

bool starts_with_case_insensitive(std::wstring_view str, std::wstring_view prefix) {
    if (str.size() < prefix.size()) {
        return false;
    }
    for (size_t i = 0; i < prefix.size(); ++i) {
        if (towlower(str[i]) != towlower(prefix[i])) {
            return false;
        }
    }
    return true;
}

bool equals_case_insensitive(std::wstring_view a, std::wstring_view b) {
    if (a.size() != b.size()) {
        return false;
    }
    for (size_t i = 0; i < a.size(); ++i) {
        if (towlower(a[i]) != towlower(b[i])) {
            return false;
        }
    }
    return true;
}

std::wstring get_filename(std::wstring_view path) {
    size_t last_slash = path.find_last_of(L"\\/");
    if (last_slash == std::wstring_view::npos) {
        return std::wstring(path);
    }
    return std::wstring(path.substr(last_slash + 1));
}

std::wstring get_parent_directory(std::wstring_view path) {
    size_t last_slash = path.find_last_of(L"\\/");
    if (last_slash == std::wstring_view::npos) {
        return L"";
    }
    return std::wstring(path.substr(0, last_slash));
}

std::wstring get_extension(std::wstring_view path) {
    size_t last_dot = path.find_last_of(L".");
    size_t last_slash = path.find_last_of(L"\\/");
    if (last_dot == std::wstring_view::npos || (last_slash != std::wstring_view::npos && last_dot < last_slash)) {
        return L"";
    }
    return std::wstring(path.substr(last_dot));
}

std::wstring normalize_path(std::wstring_view path) {
    std::wstring result(path);
    for (wchar_t& c : result) {
        if (c == L'/') {
            c = L'\\';
        }
    }
    // Remove trailing backslash if not root (e.g. C:\)
    while (result.size() > 3 && result.back() == L'\\') {
        result.pop_back();
    }
    return result;
}

std::wstring format_file_size(uint64_t bytes) {
    const wchar_t* units[] = { L"B", L"KB", L"MB", L"GB", L"TB" };
    int unit_idx = 0;
    double size = static_cast<double>(bytes);

    while (size >= 1024.0 && unit_idx < 4) {
        size /= 1024.0;
        ++unit_idx;
    }

    std::wostringstream oss;
    if (unit_idx == 0) {
        oss << static_cast<uint64_t>(size) << L" " << units[unit_idx];
    } else {
        oss << std::fixed << std::setprecision(1) << size << L" " << units[unit_idx];
    }
    return oss.str();
}

std::vector<std::wstring> split_whitespace(std::wstring_view str) {
    std::vector<std::wstring> tokens;
    size_t i = 0;
    while (i < str.size()) {
        while (i < str.size() && iswspace(str[i])) {
            ++i;
        }
        if (i >= str.size()) {
            break;
        }
        size_t start = i;
        while (i < str.size() && !iswspace(str[i])) {
            ++i;
        }
        tokens.emplace_back(str.substr(start, i - start));
    }
    return tokens;
}

} // namespace orca_light::utils
