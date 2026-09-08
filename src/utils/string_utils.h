#pragma once

#include <string>
#include <string_view>
#include <vector>
#include <cstdint>

namespace orca_light::utils {

// UTF conversion
std::wstring utf8_to_wide(std::string_view utf8_str);
std::string wide_to_utf8(std::wstring_view wide_str);

// Case operations
std::wstring to_lower(std::wstring_view str);
std::string to_lower(std::string_view str);
wchar_t to_lower_char(wchar_t c);

// Whitespace and trimming
std::wstring trim(std::wstring_view str);
std::string trim(std::string_view str);
bool starts_with_case_insensitive(std::wstring_view str, std::wstring_view prefix);
bool equals_case_insensitive(std::wstring_view a, std::wstring_view b);

// Path utilities
std::wstring get_filename(std::wstring_view path);
std::wstring get_parent_directory(std::wstring_view path);
std::wstring get_extension(std::wstring_view path);
std::wstring normalize_path(std::wstring_view path);

// Size formatting
std::wstring format_file_size(uint64_t bytes);

// Tokenization
std::vector<std::wstring> split_whitespace(std::wstring_view str);

} // namespace orca_light::utils
