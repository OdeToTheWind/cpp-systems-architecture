/**
 * @file
 * Day 02 – String Manipulation.
 *
 * Scenario: a *conference badge printer* that turns messy sign-up rows such as
 * `"  ada LOVELACE ;  analytical engines ltd "` into clean, centred, fixed-width badges.
 *
 * Deliverables (syllabus):
 * - std::string length, find and substr
 * - Concatenation
 * - Trimming and case-folding input
 * - Aligned formatting
 */
#pragma once

#include <algorithm>
#include <cctype>
#include <cstddef>
#include <istream>
#include <optional>
#include <ostream>
#include <string>
#include <string_view>
#include <utility>

#include "cppm/lesson.hpp"

namespace cppm::day02 {

inline constexpr Deliverable DELIVERABLES[] = {
    {"trimming whitespace with find_first_not_of / find_last_not_of", "trim"},
    {"case-folding with std::tolower / std::toupper", "to_title_case"},
    {"splitting with find and substr", "split_once"},
    {"searching repeatedly with find", "count_occurrences"},
    {"concatenation and aligned formatting", "make_badge"},
    {"comparing characters while ignoring case and punctuation", "is_palindrome"},
};

inline constexpr std::string_view whitespace = " \t\r\n";

/// Remove leading and trailing whitespace; an all-blank string becomes empty.
inline std::string trim(std::string_view text) {
    const auto first = text.find_first_not_of(whitespace);
    if (first == std::string_view::npos) {
        return {};
    }
    const auto last = text.find_last_not_of(whitespace);
    return std::string(text.substr(first, last - first + 1));
}

/// Lower-case every letter. The cast to unsigned char avoids undefined behaviour for negative chars.
inline std::string to_lower(std::string text) {
    std::transform(text.begin(), text.end(), text.begin(),
                   [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    return text;
}

/// "ada LOVELACE" -> "Ada Lovelace": capitalise the first letter of every word, also after '-' or '\''.
inline std::string to_title_case(std::string_view text) {
    std::string result = to_lower(trim(text));
    bool start_of_word = true;
    for (char& c : result) {
        const auto u = static_cast<unsigned char>(c);
        if (std::isalpha(u)) {
            if (start_of_word) {
                c = static_cast<char>(std::toupper(u));
            }
            start_of_word = false;
        } else {
            start_of_word = (c == ' ' || c == '-' || c == '\'');
        }
    }
    // collapse runs of inner spaces: "Ada    Lovelace" -> "Ada Lovelace"
    result.erase(std::unique(result.begin(), result.end(), [](char a, char b) { return a == ' ' && b == ' '; }),
                 result.end());
    return result;
}

/// Split at the first @p delimiter into trimmed halves; std::nullopt if the delimiter is missing.
inline std::optional<std::pair<std::string, std::string>> split_once(std::string_view text, char delimiter) {
    const auto position = text.find(delimiter);
    if (position == std::string_view::npos) {
        return std::nullopt;
    }
    return std::pair{trim(text.substr(0, position)), trim(text.substr(position + 1))};
}

/// Count non-overlapping occurrences of @p needle (case-insensitive); an empty needle counts 0.
inline std::size_t count_occurrences(std::string_view haystack, std::string_view needle) {
    if (needle.empty()) {
        return 0;
    }
    const std::string text = to_lower(std::string(haystack));
    const std::string word = to_lower(std::string(needle));
    std::size_t count = 0;
    for (auto at = text.find(word); at != std::string::npos; at = text.find(word, at + word.size())) {
        ++count;
    }
    return count;
}

/// Centre @p text in @p width columns, shortening it with "..." when it does not fit.
inline std::string centre(std::string_view text, std::size_t width) {
    std::string line(text);
    if (line.size() > width) {
        line = width > 3 ? line.substr(0, width - 3) + "..." : line.substr(0, width);
    }
    const std::size_t padding = width - line.size();
    const std::size_t left = padding / 2;
    return std::string(left, ' ') + line + std::string(padding - left, ' ');
}

/// Build a framed badge: name line, company line, border – all exactly @p width + 4 characters wide.
inline std::string make_badge(std::string_view raw_row, std::size_t width = 28) {
    std::string name;
    std::string company;
    if (auto parts = split_once(raw_row, ';')) {
        name = to_title_case(parts->first);
        company = to_title_case(parts->second);
    } else {
        name = to_title_case(raw_row);
    }
    if (name.empty()) {
        name = "Guest";
    }
    const std::string border = "+" + std::string(width + 2, '-') + "+\n";
    std::string badge = border;
    badge += "| " + centre(name, width) + " |\n";
    badge.append("| ").append(centre(company.empty() ? "Independent" : company, width)).append(" |\n");
    badge += border;
    return badge;
}

/// True if @p text reads the same backwards, ignoring case, spaces and punctuation (digits count).
inline bool is_palindrome(std::string_view text) {
    std::string letters;
    for (const char c : text) {
        if (std::isalnum(static_cast<unsigned char>(c))) {
            letters += static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
        }
    }
    return std::equal(letters.begin(), letters.begin() + static_cast<std::ptrdiff_t>(letters.size() / 2),
                      letters.rbegin());
}

/// The interactive demo: paste `name ; company` rows, get badges; a blank line ends.
inline int run(std::istream& in, std::ostream& out) {
    out << "Day 02 – String Manipulation\n";
    out << "Paste sign-up rows as 'name ; company' (blank line to finish)\n";
    int printed = 0;
    while (auto line = prompt_line(in, out, "row> ")) {
        if (trim(*line).empty()) {
            break;
        }
        out << make_badge(*line);
        out << "  length " << trim(*line).size() << " chars, "
            << (is_palindrome(to_title_case(*line)) ? "a palindrome!" : "not a palindrome") << '\n';
        ++printed;
    }
    out << printed << " badge(s) printed.\n";
    return 0;
}

}  // namespace cppm::day02
