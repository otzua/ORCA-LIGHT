#pragma once

#include <string>
#include <string_view>
#include <vector>
#include <cstdint>

namespace orca_light::search {

struct MatchResult {
    bool matched = false;
    int32_t score = 0;
    std::vector<uint32_t> matched_indices; // Indices of matched characters for highlighting
};

class FuzzyMatcher {
public:
    // Computes fuzzy match score between pattern and candidate text
    // Higher score means better match
    static MatchResult match(std::wstring_view pattern, std::wstring_view candidate);

    // Fast check if candidate contains all characters of pattern in order (case-insensitive)
    static bool contains_subsequence(std::wstring_view pattern, std::wstring_view candidate);

    // Fast acronym / initial letters check (e.g. "np" -> "Notepad++")
    static bool is_acronym_match(std::wstring_view pattern, std::wstring_view candidate);
};

} // namespace orca_light::search
