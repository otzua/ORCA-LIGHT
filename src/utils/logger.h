#pragma once

#include <windows.h>
#include <string>
#include <string_view>

namespace orca_light::utils {

class Logger {
public:
    static void init();
    static void log(std::string_view message);
    static void log(std::wstring_view message);
    static void error(std::wstring_view message, HRESULT hr = S_OK);
    static const std::wstring& get_log_path();
};

} // namespace orca_light::utils
