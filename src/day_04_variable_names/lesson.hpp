/**
 * @file
 * Day 04 – Variable Naming Rules.
 *
 * Scenario: a *naming review bot* for pull requests: it inspects every proposed identifier
 * and reports errors (the name will not compile), warnings about reserved names, and
 * style advice so that variables, functions, types and constants each follow one convention.
 *
 * Deliverables (syllabus):
 * - Legal identifiers
 * - Reserved words and reserved names
 * - snake_case and PascalCase conventions
 * - Intention-revealing names
 */
#pragma once

#include <algorithm>
#include <array>
#include <cctype>
#include <istream>
#include <ostream>
#include <sstream>
#include <string>
#include <string_view>
#include <vector>

#include "cppm/lesson.hpp"

namespace cppm::day04 {

inline constexpr Deliverable DELIVERABLES[] = {
    {"C++ keywords cannot be identifiers", "is_keyword"},
    {"the grammar of a legal identifier", "is_legal_identifier"},
    {"names reserved for the implementation", "is_reserved_name"},
    {"recognising snake_case, PascalCase and UPPER_SNAKE_CASE", "detect_style"},
    {"converting camelCase to snake_case", "to_snake_case"},
    {"spotting names that hide intent", "is_vague_name"},
    {"one review combining errors and advice", "review_name"},
};

/// What an identifier names; each kind has its own convention in this project.
enum class Kind { variable, function, type, constant };

/// The naming style an identifier is written in.
enum class Style { snake_case, pascal_case, camel_case, upper_snake_case, other };

inline constexpr std::array<std::string_view, 92> keywords = {
    "alignas", "alignof", "and", "and_eq", "asm", "auto", "bitand", "bitor", "bool", "break", "case",
    "catch", "char", "char8_t", "char16_t", "char32_t", "class", "compl", "concept", "const", "consteval",
    "constexpr", "constinit", "const_cast", "continue", "co_await", "co_return", "co_yield", "decltype",
    "default", "delete", "do", "double", "dynamic_cast", "else", "enum", "explicit", "export", "extern",
    "false", "float", "for", "friend", "goto", "if", "inline", "int", "long", "mutable", "namespace", "new",
    "noexcept", "not", "not_eq", "nullptr", "operator", "or", "or_eq", "private", "protected", "public",
    "register", "reinterpret_cast", "requires", "return", "short", "signed", "sizeof", "static",
    "static_assert", "static_cast", "struct", "switch", "template", "this", "thread_local", "throw", "true",
    "try", "typedef", "typeid", "typename", "union", "unsigned", "using", "virtual", "void", "volatile",
    "wchar_t", "while", "xor", "xor_eq"};

/// True for every C++20 keyword and alternative operator token.
inline bool is_keyword(std::string_view name) {
    return std::find(keywords.begin(), keywords.end(), name) != keywords.end();
}

/// A legal identifier: a letter or '_' first, then letters, digits or '_', and not a keyword.
inline bool is_legal_identifier(std::string_view name) {
    if (name.empty() || std::isdigit(static_cast<unsigned char>(name.front()))) {
        return false;
    }
    const bool valid_chars = std::all_of(name.begin(), name.end(), [](char c) {
        return std::isalnum(static_cast<unsigned char>(c)) || c == '_';
    });
    return valid_chars && !is_keyword(name);
}

/// Reserved for the compiler and standard library: any "__", or '_' followed by an upper-case letter.
inline bool is_reserved_name(std::string_view name) {
    if (name.find("__") != std::string_view::npos) {
        return true;
    }
    return name.size() >= 2 && name[0] == '_' && std::isupper(static_cast<unsigned char>(name[1]));
}

/// Classify the writing style of a legal identifier.
inline Style detect_style(std::string_view name) {
    const auto has = [&](auto predicate) {
        return std::any_of(name.begin(), name.end(), [&](char c) { return predicate(static_cast<unsigned char>(c)); });
    };
    const bool upper = has([](unsigned char c) { return std::isupper(c) != 0; });
    const bool lower = has([](unsigned char c) { return std::islower(c) != 0; });
    const bool underscore = name.find('_') != std::string_view::npos;
    if (name.empty()) {
        return Style::other;
    }
    if (!upper) {
        return Style::snake_case;
    }
    if (!lower) {
        return Style::upper_snake_case;
    }
    if (underscore) {
        return Style::other;
    }
    return std::isupper(static_cast<unsigned char>(name.front())) ? Style::pascal_case : Style::camel_case;
}

/// "userAgeInYears" -> "user_age_in_years", "HTTPServer" -> "http_server".
inline std::string to_snake_case(std::string_view name) {
    std::string result;
    for (std::size_t i = 0; i < name.size(); ++i) {
        const auto c = static_cast<unsigned char>(name[i]);
        if (std::isupper(c)) {
            const bool previous_lower = i > 0 && std::islower(static_cast<unsigned char>(name[i - 1]));
            const bool next_lower = i + 1 < name.size() && std::islower(static_cast<unsigned char>(name[i + 1]));
            const bool previous_upper = i > 0 && std::isupper(static_cast<unsigned char>(name[i - 1]));
            if (!result.empty() && result.back() != '_' && (previous_lower || (previous_upper && next_lower))) {
                result += '_';
            }
            result += static_cast<char>(std::tolower(c));
        } else {
            result += static_cast<char>(c);
        }
    }
    return result;
}

/// Names that say nothing about intent, or encode the type (Hungarian notation).
inline bool is_vague_name(std::string_view name) {
    static constexpr std::array<std::string_view, 9> vague = {"data", "temp", "tmp", "flag", "foo",
                                                              "bar",  "thing", "stuff", "val"};
    if (name.size() == 1 && name != "i" && name != "j" && name != "n") {
        return true;
    }
    if (std::find(vague.begin(), vague.end(), name) != vague.end()) {
        return true;
    }
    for (std::string_view prefix : {"str", "int", "b", "p", "sz"}) {  // strName, intCount, bDone, pNode
        if (name.size() > prefix.size() && name.substr(0, prefix.size()) == prefix &&
            std::isupper(static_cast<unsigned char>(name[prefix.size()]))) {
            return true;
        }
    }
    return false;
}

/// Findings for one identifier: errors make the code fail to compile, warnings are advice.
struct Review {
    std::vector<std::string> errors;
    std::vector<std::string> warnings;
    bool clean() const { return errors.empty() && warnings.empty(); }
};

/// Review @p name for its @p kind: legality first, then reserved names, convention and intent.
inline Review review_name(std::string_view name, Kind kind) {
    Review review;
    if (!is_legal_identifier(name)) {
        review.errors.push_back(is_keyword(name) ? "'" + std::string(name) + "' is a C++ keyword"
                                                 : "'" + std::string(name) + "' is not a legal identifier");
        return review;
    }
    if (is_reserved_name(name)) {
        review.warnings.emplace_back("reserved for the implementation (double underscore or _Capital)");
    }
    const Style style = detect_style(name);
    const bool type_like = kind == Kind::type;
    if (kind == Kind::constant && style != Style::upper_snake_case &&
        !(name.size() > 1 && name[0] == 'k' && std::isupper(static_cast<unsigned char>(name[1])))) {
        review.warnings.emplace_back("constants use UPPER_SNAKE_CASE or kPascalCase");
    } else if (type_like && style != Style::pascal_case) {
        review.warnings.emplace_back("types use PascalCase");
    } else if ((kind == Kind::variable || kind == Kind::function) && style != Style::snake_case) {
        review.warnings.push_back("use snake_case: " + to_snake_case(name));
    }
    if (is_vague_name(name)) {
        review.warnings.emplace_back("the name does not reveal intent");
    }
    return review;
}

/// Parse "variable", "function", "type" or "constant".
inline bool parse_kind(std::string_view word, Kind& kind) {
    if (word == "variable") kind = Kind::variable;
    else if (word == "function") kind = Kind::function;
    else if (word == "type") kind = Kind::type;
    else if (word == "constant") kind = Kind::constant;
    else return false;
    return true;
}

/// The interactive demo: type "<kind> <name>" lines, e.g. "variable userAge".
inline int run(std::istream& in, std::ostream& out) {
    out << "Day 04 – Variable Naming Rules\nType '<variable|function|type|constant> <name>', blank to quit\n";
    while (auto line = prompt_line(in, out, "review> ")) {
        if (line->empty()) {
            break;
        }
        std::istringstream words(*line);
        std::string kind_word;
        std::string name;
        Kind kind{};
        if (!(words >> kind_word >> name) || !parse_kind(kind_word, kind)) {
            out << "  usage: <variable|function|type|constant> <name>\n";
            continue;
        }
        const Review review = review_name(name, kind);
        for (const auto& error : review.errors) out << "  ERROR   " << error << '\n';
        for (const auto& warning : review.warnings) out << "  WARNING " << warning << '\n';
        if (review.clean()) out << "  OK      '" << name << "' follows the conventions\n";
    }
    return 0;
}

}  // namespace cppm::day04
