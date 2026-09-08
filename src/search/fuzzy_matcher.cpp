#include "fuzzy_matcher.h"
#include <cwctype>
#include <algorithm>

namespace orca_light::search {

namespace {

inline bool is_word_separator(wchar_t c) {
    return c == L' ' || c == L'-' || c == L'_' || c == L'.' || c == L'/' || c == L'\\' || c == L':' || c == L'(' || c == L'[';
}

inline bool is_word_start(std::wstring_view str, size_t index) {
    if (index == 0) {
        return true;
    }
    wchar_t prev = str[index - 1];
    wchar_t curr = str[index];
    if (is_word_separator(prev)) {
        return true;
    }
    // CamelCase transition: lowercase followed by uppercase
    if (iswlower(prev) && iswupper(curr)) {
        return true;
    }
    return false;
}

} // namespace

bool FuzzyMatcher::contains_subsequence(std::wstring_view pattern, std::wstring_view candidate) {
    if (pattern.empty()) {
        return true;
    }
    if (pattern.size() > candidate.size()) {
        return false;
    }

    size_t pat_idx = 0;
    for (size_t cand_idx = 0; cand_idx < candidate.size() && pat_idx < pattern.size(); ++cand_idx) {
        if (towlower(candidate[cand_idx]) == towlower(pattern[pat_idx])) {
            ++pat_idx;
        }
    }
    return pat_idx == pattern.size();
}

bool FuzzyMatcher::is_acronym_match(std::wstring_view pattern, std::wstring_view candidate) {
    if (pattern.empty() || candidate.empty()) {
        return false;
    }

    size_t pat_idx = 0;
    for (size_t i = 0; i < candidate.size() && pat_idx < pattern.size(); ++i) {
        if (is_word_start(candidate, i)) {
            if (towlower(candidate[i]) == towlower(pattern[pat_idx])) {
                ++pat_idx;
            }
        }
    }
    return pat_idx == pattern.size();
}

MatchResult FuzzyMatcher::match(std::wstring_view pattern, std::wstring_view candidate) {
    MatchResult result;
    if (pattern.empty()) {
        result.matched = true;
        result.score = 0;
        return result;
    }

    if (pattern.size() > candidate.size()) {
        result.matched = false;
        return result;
    }

    // Check fast subsequence first
    if (!contains_subsequence(pattern, candidate)) {
        result.matched = false;
        return result;
    }

    // Exact match
    if (pattern.size() == candidate.size()) {
        bool exact = true;
        for (size_t i = 0; i < pattern.size(); ++i) {
            if (towlower(pattern[i]) != towlower(candidate[i])) {
                exact = false;
                break;
            }
        }
        if (exact) {
            result.matched = true;
            result.score = 10000;
            result.matched_indices.reserve(pattern.size());
            for (uint32_t i = 0; i < pattern.size(); ++i) {
                result.matched_indices.push_back(i);
            }
            return result;
        }
    }

    // Check exact prefix match
    bool is_prefix = true;
    for (size_t i = 0; i < pattern.size(); ++i) {
        if (towlower(candidate[i]) != towlower(pattern[i])) {
            is_prefix = false;
            break;
        }
    }
    if (is_prefix) {
        result.matched = true;
        result.score = 5000 - static_cast<int32_t>(candidate.size() - pattern.size());
        result.matched_indices.reserve(pattern.size());
        for (uint32_t i = 0; i < pattern.size(); ++i) {
            result.matched_indices.push_back(i);
        }
        return result;
    }

    // General fuzzy matching with optimal greedy/lookahead scoring
    int32_t total_score = 0;
    size_t pat_idx = 0;
    size_t prev_matched_idx = static_cast<size_t>(-1);
    int32_t consecutive_matches = 0;

    result.matched_indices.reserve(pattern.size());

    for (size_t cand_idx = 0; cand_idx < candidate.size() && pat_idx < pattern.size(); ++cand_idx) {
        wchar_t pat_c = towlower(pattern[pat_idx]);
        wchar_t cand_c = towlower(candidate[cand_idx]);

        if (pat_c == cand_c) {
            result.matched_indices.push_back(static_cast<uint32_t>(cand_idx));
            int32_t char_score = 10;

            // First character bonus/penalty
            if (pat_idx == 0) {
                char_score -= static_cast<int32_t>(cand_idx) * 2; // Penalty for leading characters
            }

            // Word boundary bonus
            if (is_word_start(candidate, cand_idx)) {
                char_score += 50;
            }

            // Consecutive matches bonus
            if (prev_matched_idx != static_cast<size_t>(-1) && cand_idx == prev_matched_idx + 1) {
                ++consecutive_matches;
                char_score += 30 * consecutive_matches;
            } else {
                consecutive_matches = 0;
                if (prev_matched_idx != static_cast<size_t>(-1)) {
                    int32_t gap = static_cast<int32_t>(cand_idx - prev_matched_idx - 1);
                    char_score -= std::min(gap, 20); // Penalty for gaps between matches
                }
            }

            // Exact case bonus
            if (pattern[pat_idx] == candidate[cand_idx]) {
                char_score += 5;
            }

            total_score += char_score;
            prev_matched_idx = cand_idx;
            ++pat_idx;
        }
    }

    if (pat_idx == pattern.size()) {
        result.matched = true;
        // Acronym match bonus
        if (is_acronym_match(pattern, candidate)) {
            total_score += 200;
        }
        // Shorter candidates that match get bonus over very long ones
        total_score -= static_cast<int32_t>(candidate.size() / 4);
        result.score = total_score;
    } else {
        result.matched = false;
        result.matched_indices.clear();
    }

    return result;
}

} // namespace orca_light::search
