#include "calc_engine.h"
#include <vector>
#include <cwctype>
#include <cmath>
#include <sstream>
#include <iomanip>
#include <limits>
#include <cstdint>

namespace orca_light::calc {

namespace {

enum class TokenType {
    Number,
    Plus,
    Minus,
    Multiply,
    Divide,
    Modulo,
    Power,
    LParen,
    RParen,
    Identifier,
    End
};

struct Token {
    TokenType type = TokenType::End;
    double number_val = 0.0;
    std::wstring text_val;
};

class Lexer {
public:
    explicit Lexer(std::wstring_view input) : input_(input), pos_(0) {}

    Token next_token() {
        skip_whitespace();
        if (pos_ >= input_.size()) {
            return { TokenType::End };
        }

        wchar_t c = input_[pos_];

        if (iswdigit(c) || c == L'.') {
            size_t start = pos_;
            bool has_dot = (c == L'.');
            ++pos_;
            while (pos_ < input_.size()) {
                wchar_t ch = input_[pos_];
                if (iswdigit(ch)) {
                    ++pos_;
                } else if (ch == L'.' && !has_dot) {
                    has_dot = true;
                    ++pos_;
                } else {
                    break;
                }
            }
            std::wstring num_str(input_.substr(start, pos_ - start));
            try {
                double val = std::stod(num_str);
                return { TokenType::Number, val, num_str };
            } catch (...) {
                return { TokenType::End };
            }
        }

        if (iswalpha(c)) {
            size_t start = pos_;
            while (pos_ < input_.size() && (iswalpha(input_[pos_]) || iswdigit(input_[pos_]))) {
                ++pos_;
            }
            std::wstring id(input_.substr(start, pos_ - start));
            for (wchar_t& ch : id) {
                ch = static_cast<wchar_t>(towlower(ch));
            }
            return { TokenType::Identifier, 0.0, id };
        }

        ++pos_;
        switch (c) {
            case L'+': return { TokenType::Plus, 0.0, L"+" };
            case L'-': return { TokenType::Minus, 0.0, L"-" };
            case L'*': return { TokenType::Multiply, 0.0, L"*" };
            case L'x':
            case L'X': return { TokenType::Multiply, 0.0, L"*" };
            case L'/': return { TokenType::Divide, 0.0, L"/" };
            case L'%': return { TokenType::Modulo, 0.0, L"%" };
            case L'^': return { TokenType::Power, 0.0, L"^" };
            case L'(': return { TokenType::LParen, 0.0, L"(" };
            case L')': return { TokenType::RParen, 0.0, L")" };
            default:   return { TokenType::End };
        }
    }

private:
    void skip_whitespace() {
        while (pos_ < input_.size() && iswspace(input_[pos_])) {
            ++pos_;
        }
    }

    std::wstring_view input_;
    size_t pos_ = 0;
};

class Parser {
public:
    explicit Parser(Lexer& lexer) : lexer_(lexer) {
        advance();
    }

    bool parse(double& out_value) {
        has_error_ = false;
        out_value = parse_expression();
        if (has_error_ || current_.type != TokenType::End || std::isnan(out_value) || std::isinf(out_value)) {
            return false;
        }
        return true;
    }

private:
    void advance() {
        current_ = lexer_.next_token();
    }

    double parse_expression() {
        double left = parse_term();
        while (!has_error_ && (current_.type == TokenType::Plus || current_.type == TokenType::Minus)) {
            TokenType op = current_.type;
            advance();
            double right = parse_term();
            if (op == TokenType::Plus) {
                left += right;
            } else {
                left -= right;
            }
        }
        return left;
    }

    double parse_term() {
        double left = parse_power();
        while (!has_error_ && (current_.type == TokenType::Multiply ||
                               current_.type == TokenType::Divide ||
                               current_.type == TokenType::Modulo)) {
            TokenType op = current_.type;
            advance();
            double right = parse_power();
            if (op == TokenType::Multiply) {
                left *= right;
            } else if (op == TokenType::Divide) {
                if (std::abs(right) < 1e-15) {
                    has_error_ = true;
                    return 0.0;
                }
                left /= right;
            } else if (op == TokenType::Modulo) {
                if (std::abs(right) < 1e-15) {
                    has_error_ = true;
                    return 0.0;
                }
                left = std::fmod(left, right);
            }
        }
        return left;
    }

    double parse_power() {
        double base = parse_factor();
        if (!has_error_ && current_.type == TokenType::Power) {
            advance();
            double exponent = parse_factor();
            base = std::pow(base, exponent);
        }
        return base;
    }

    double parse_factor() {
        if (current_.type == TokenType::Plus) {
            advance();
            return parse_factor();
        }
        if (current_.type == TokenType::Minus) {
            advance();
            return -parse_factor();
        }
        return parse_primary();
    }

    double parse_primary() {
        if (current_.type == TokenType::Number) {
            double val = current_.number_val;
            advance();
            return val;
        }

        if (current_.type == TokenType::LParen) {
            advance();
            double val = parse_expression();
            if (current_.type != TokenType::RParen) {
                has_error_ = true;
                return 0.0;
            }
            advance();
            return val;
        }

        if (current_.type == TokenType::Identifier) {
            std::wstring id = current_.text_val;
            advance();

            // Constants
            if (id == L"pi") {
                return 3.14159265358979323846;
            }
            if (id == L"e") {
                return 2.71828182845904523536;
            }

            // Functions: expect '('
            if (current_.type != TokenType::LParen) {
                has_error_ = true;
                return 0.0;
            }
            advance();
            double arg = parse_expression();
            if (current_.type != TokenType::RParen) {
                has_error_ = true;
                return 0.0;
            }
            advance();

            if (id == L"sqrt") return (arg >= 0.0) ? std::sqrt(arg) : (has_error_ = true, 0.0);
            if (id == L"cbrt") return std::cbrt(arg);
            if (id == L"sin")  return std::sin(arg);
            if (id == L"cos")  return std::cos(arg);
            if (id == L"tan")  return std::tan(arg);
            if (id == L"asin") return (arg >= -1.0 && arg <= 1.0) ? std::asin(arg) : (has_error_ = true, 0.0);
            if (id == L"acos") return (arg >= -1.0 && arg <= 1.0) ? std::acos(arg) : (has_error_ = true, 0.0);
            if (id == L"atan") return std::atan(arg);
            if (id == L"abs")  return std::abs(arg);
            if (id == L"ln")   return (arg > 0.0) ? std::log(arg) : (has_error_ = true, 0.0);
            if (id == L"log" || id == L"log10") return (arg > 0.0) ? std::log10(arg) : (has_error_ = true, 0.0);
            if (id == L"log2") return (arg > 0.0) ? std::log2(arg) : (has_error_ = true, 0.0);
            if (id == L"exp")  return std::exp(arg);
            if (id == L"floor") return std::floor(arg);
            if (id == L"ceil")  return std::ceil(arg);
            if (id == L"round") return std::round(arg);

            has_error_ = true;
            return 0.0;
        }

        has_error_ = true;
        return 0.0;
    }

    Lexer& lexer_;
    Token current_;
    bool has_error_ = false;
};

std::wstring format_value(double val) {
    if (std::abs(val - std::round(val)) < 1e-11 && std::abs(val) < 1e15) {
        return std::to_wstring(static_cast<int64_t>(std::round(val)));
    }

    std::wostringstream oss;
    oss << std::setprecision(10) << val;
    std::wstring s = oss.str();
    // Trim trailing zeros if decimal point is present
    if (s.find(L'.') != std::wstring::npos) {
        while (s.size() > 1 && s.back() == L'0') {
            s.pop_back();
        }
        if (s.back() == L'.') {
            s.pop_back();
        }
    }
    return s;
}

} // namespace

bool CalcEngine::is_likely_expression(std::wstring_view expr) {
    if (expr.empty()) {
        return false;
    }

    bool has_digit = false;
    bool has_operator = false;

    for (wchar_t c : expr) {
        if (iswdigit(c)) {
            has_digit = true;
        } else if (c == L'+' || c == L'-' || c == L'*' || c == L'/' || c == L'%' || c == L'^' || c == L'=' ) {
            has_operator = true;
        }
    }

    // Also check for math functions
    if (expr.find(L"sqrt(") != std::wstring_view::npos ||
        expr.find(L"sin(") != std::wstring_view::npos ||
        expr.find(L"cos(") != std::wstring_view::npos ||
        expr.find(L"tan(") != std::wstring_view::npos ||
        expr.find(L"log(") != std::wstring_view::npos ||
        expr.find(L"abs(") != std::wstring_view::npos ||
        expr.find(L"pi") != std::wstring_view::npos) {
        return true;
    }

    // Require either (digits and operators) or at least two digits with spaces or a single digit expression
    return has_digit && has_operator;
}

std::optional<CalcResult> CalcEngine::evaluate(std::wstring_view expr) {
    if (expr.empty() || !is_likely_expression(expr)) {
        return std::nullopt;
    }

    // Strip trailing '=' if user typed e.g. "5 + 5 ="
    std::wstring clean_expr(expr);
    while (!clean_expr.empty() && (clean_expr.back() == L'=' || iswspace(clean_expr.back()))) {
        clean_expr.pop_back();
    }

    if (clean_expr.empty()) {
        return std::nullopt;
    }

    Lexer lexer(clean_expr);
    Parser parser(lexer);
    double result_value = 0.0;

    if (!parser.parse(result_value)) {
        return std::nullopt;
    }

    CalcResult res;
    res.valid = true;
    res.value = result_value;
    res.expression = clean_expr;
    res.formatted_result = format_value(result_value);
    return res;
}

} // namespace orca_light::calc
