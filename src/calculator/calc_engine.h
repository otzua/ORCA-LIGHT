#pragma once

#include <string>
#include <string_view>
#include <optional>

namespace orca_light::calc {

struct CalcResult {
    bool valid = false;
    double value = 0.0;
    std::wstring expression;
    std::wstring formatted_result;
};

class CalcEngine {
public:
    // Evaluates a math expression. Returns nullopt if not a valid math expression.
    static std::optional<CalcResult> evaluate(std::wstring_view expr);

    // Quick heuristic check if query looks like a calculation attempt
    static bool is_likely_expression(std::wstring_view expr);
};

} // namespace orca_light::calc
