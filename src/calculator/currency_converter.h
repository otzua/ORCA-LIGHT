#pragma once
#include <string>
#include <string_view>
#include <optional>

namespace orca_light::calc {

struct CurrencyResult {
    double amount;
    std::wstring from_currency;
    double result;
    std::wstring to_currency;
};

class CurrencyConverter {
public:
    static void update_rates();
    static std::optional<CurrencyResult> convert(std::wstring_view query);
};

} // namespace orca_light::calc
