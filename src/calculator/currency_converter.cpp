#include "currency_converter.h"
#include "../utils/string_utils.h"
#include "../utils/logger.h"
#include <windows.h>
#include <wininet.h>
#include <sstream>
#include <regex>
#include <unordered_map>
#include <mutex>

#pragma comment(lib, "wininet.lib")

namespace orca_light::calc {

namespace {
    std::unordered_map<std::wstring, double> s_rates;
    std::mutex s_rates_mutex;
    bool s_rates_loaded = false;

    // Simple custom JSON parser for {"USD":1,"EUR":0.85,...}
    void parse_rates(const std::string& json) {
        size_t rates_pos = json.find("\"rates\":{");
        if (rates_pos == std::string::npos) return;
        
        size_t end_pos = json.find("}", rates_pos);
        if (end_pos == std::string::npos) return;

        std::string rates_str = json.substr(rates_pos + 9, end_pos - (rates_pos + 9));
        
        std::lock_guard<std::mutex> lock(s_rates_mutex);
        s_rates.clear();
        
        size_t pos = 0;
        while (pos < rates_str.length()) {
            size_t quote1 = rates_str.find("\"", pos);
            if (quote1 == std::string::npos) break;
            size_t quote2 = rates_str.find("\"", quote1 + 1);
            if (quote2 == std::string::npos) break;
            
            std::string curr = rates_str.substr(quote1 + 1, quote2 - quote1 - 1);
            
            size_t colon = rates_str.find(":", quote2);
            if (colon == std::string::npos) break;
            
            size_t comma = rates_str.find(",", colon);
            if (comma == std::string::npos) comma = rates_str.length();
            
            std::string val_str = rates_str.substr(colon + 1, comma - colon - 1);
            
            try {
                double val = std::stod(val_str);
                s_rates[utils::utf8_to_wide(curr)] = val;
            } catch (...) {}
            
            pos = comma + 1;
        }
        if (!s_rates.empty()) {
            s_rates_loaded = true;
        }
    }
}

void CurrencyConverter::update_rates() {
    HINTERNET hInternet = InternetOpenW(L"Orca-Light", INTERNET_OPEN_TYPE_DIRECT, nullptr, nullptr, 0);
    if (!hInternet) return;

    HINTERNET hUrl = InternetOpenUrlW(hInternet, L"https://open.er-api.com/v6/latest/USD", nullptr, 0, INTERNET_FLAG_RELOAD | INTERNET_FLAG_SECURE, 0);
    if (hUrl) {
        std::string response;
        char buffer[4096];
        DWORD bytes_read = 0;
        while (InternetReadFile(hUrl, buffer, sizeof(buffer), &bytes_read) && bytes_read > 0) {
            response.append(buffer, bytes_read);
        }
        InternetCloseHandle(hUrl);
        
        if (!response.empty()) {
            parse_rates(response);
            utils::Logger::log("Currency rates updated successfully.");
        }
    }
    InternetCloseHandle(hInternet);
}

std::optional<CurrencyResult> CurrencyConverter::convert(std::wstring_view query) {
    if (!s_rates_loaded) return std::nullopt;

    std::wstring q = utils::to_lower(std::wstring(query));

    auto replace_all = [](std::wstring& str, const std::wstring& from, const std::wstring& to) {
        size_t start_pos = 0;
        while((start_pos = str.find(from, start_pos)) != std::wstring::npos) {
            str.replace(start_pos, from.length(), to);
            start_pos += to.length();
        }
    };

    replace_all(q, L"$", L" usd ");
    replace_all(q, L"€", L" eur ");
    replace_all(q, L"£", L" gbp ");
    replace_all(q, L"¥", L" jpy ");
    replace_all(q, L"₹", L" inr ");

    std::wregex rx(L"^\\s*(?:([a-z]{3})\\s+)?([\\d\\.]+)(?:\\s+([a-z]{3}))?\\s+(?:to\\s+|in\\s+|=\\s+)?([a-z]{3})\\s*$");
    std::wsmatch match;
    
    if (std::regex_match(q, match, rx)) {
        double amount = 0.0;
        try {
            amount = std::stod(match[2].str());
        } catch (...) { return std::nullopt; }

        std::wstring from;
        if (match[1].matched && match[1].length() > 0) {
            from = match[1].str();
        } else if (match[3].matched && match[3].length() > 0) {
            from = match[3].str();
        } else {
            return std::nullopt; // No source currency
        }
        
        from = orca_light::utils::to_upper(from);
        std::wstring to = orca_light::utils::to_upper(match[4].str());

        std::lock_guard<std::mutex> lock(s_rates_mutex);
        if (s_rates.count(from) && s_rates.count(to)) {
            double rate_from = s_rates[from];
            double rate_to = s_rates[to];
            
            // Rates are based on USD.
            // USD to target: amount * rate_to
            // Target to USD: amount / rate_from
            double result = (amount / rate_from) * rate_to;

            return CurrencyResult{amount, from, result, to};
        }
    }

    return std::nullopt;
}

} // namespace orca_light::calc
