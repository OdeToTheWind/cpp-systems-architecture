/**
 * @file
 * Day 26 – IDE Tips and Tricks.
 *
 * Scenario: a *pocket IDE coach*: a searchable shortcut cheat-sheet for VS Code, CLion and
 * Visual Studio on each operating system, a live-template expander, and the refactorings an
 * IDE performs – find references, go to definition and a token-aware rename that never
 * touches strings, comments or longer names that merely contain the old one.
 *
 * Deliverables (syllabus):
 * - Navigation and refactoring workflows
 * - Keyboard shortcuts
 * - Token-aware rename
 * - Code templates
 */
#pragma once

#include <algorithm>
#include <cctype>
#include <cstddef>
#include <istream>
#include <map>
#include <optional>
#include <ostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

#include "cppm/lesson.hpp"

namespace cppm::day26 {

inline constexpr Deliverable DELIVERABLES[] = {
    {"a shortcut cheat-sheet per IDE and operating system", "search_shortcuts"},
    {"splitting code into tokens like an IDE does", "tokenize"},
    {"find all references", "find_references"},
    {"go to definition", "go_to_definition"},
    {"a rename that only changes whole identifier tokens", "rename_symbol"},
    {"expanding live templates with placeholders", "expand_template"},
};

struct Shortcut {
    std::string_view ide;
    std::string_view os;
    std::string_view action;
    std::string_view keys;
};

inline constexpr Shortcut cheat_sheet[] = {
    {"vscode", "windows", "go to definition", "F12"},
    {"vscode", "macos", "go to definition", "F12"},
    {"vscode", "windows", "find all references", "Shift+F12"},
    {"vscode", "macos", "find all references", "Shift+F12"},
    {"vscode", "windows", "rename symbol", "F2"},
    {"vscode", "macos", "rename symbol", "F2"},
    {"vscode", "windows", "command palette", "Ctrl+Shift+P"},
    {"vscode", "macos", "command palette", "Cmd+Shift+P"},
    {"clion", "windows", "go to definition", "Ctrl+B"},
    {"clion", "macos", "go to definition", "Cmd+B"},
    {"clion", "windows", "find all references", "Alt+F7"},
    {"clion", "macos", "find all references", "Option+F7"},
    {"clion", "windows", "rename symbol", "Shift+F6"},
    {"clion", "macos", "rename symbol", "Shift+F6"},
    {"clion", "windows", "toggle breakpoint", "Ctrl+F8"},
    {"clion", "macos", "toggle breakpoint", "Cmd+F8"},
    {"visualstudio", "windows", "go to definition", "F12"},
    {"visualstudio", "windows", "find all references", "Shift+F12"},
    {"visualstudio", "windows", "rename symbol", "Ctrl+R, Ctrl+R"},
    {"visualstudio", "windows", "toggle breakpoint", "F9"},
};

inline std::string lower(std::string_view text) {
    std::string out(text);
    std::transform(out.begin(), out.end(), out.begin(),
                   [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    return out;
}

/// Shortcuts whose action contains @p query (case-insensitive), optionally for one IDE and OS.
inline std::vector<Shortcut> search_shortcuts(std::string_view query, std::string_view ide = "",
                                              std::string_view os = "") {
    std::vector<Shortcut> found;
    const std::string needle = lower(query);
    for (const auto& shortcut : cheat_sheet) {
        if ((ide.empty() || shortcut.ide == ide) && (os.empty() || shortcut.os == os) &&
            std::string(shortcut.action).find(needle) != std::string::npos) {
            found.push_back(shortcut);
        }
    }
    return found;
}

enum class TokenKind { identifier, number, string_literal, comment, symbol };

struct Token {
    TokenKind kind;
    std::string text;
    int line;
    int column;
};

/// A small C++ tokenizer: identifiers, numbers, string/char literals, // and /* */ comments, symbols.
inline std::vector<Token> tokenize(std::string_view code) {
    std::vector<Token> tokens;
    int line = 1;
    int column = 1;
    std::size_t i = 0;
    const auto advance = [&](std::size_t count) {
        for (std::size_t k = 0; k < count && i < code.size(); ++k, ++i) {
            if (code[i] == '\n') {
                ++line;
                column = 1;
            } else {
                ++column;
            }
        }
    };
    while (i < code.size()) {
        const char c = code[i];
        const int start_line = line;
        const int start_column = column;
        const std::size_t start = i;
        if (std::isspace(static_cast<unsigned char>(c))) {
            advance(1);
            continue;
        }
        TokenKind kind = TokenKind::symbol;
        if (std::isalpha(static_cast<unsigned char>(c)) || c == '_') {
            kind = TokenKind::identifier;
            while (i < code.size() && (std::isalnum(static_cast<unsigned char>(code[i])) || code[i] == '_')) advance(1);
        } else if (std::isdigit(static_cast<unsigned char>(c))) {
            kind = TokenKind::number;
            while (i < code.size() && (std::isalnum(static_cast<unsigned char>(code[i])) || code[i] == '.' || code[i] == '\'')) advance(1);
        } else if (c == '"' || c == '\'') {
            kind = TokenKind::string_literal;
            advance(1);
            while (i < code.size() && code[i] != c) advance(code[i] == '\\' ? 2 : 1);
            advance(1);
        } else if (code.substr(i, 2) == "//") {
            kind = TokenKind::comment;
            while (i < code.size() && code[i] != '\n') advance(1);
        } else if (code.substr(i, 2) == "/*") {
            kind = TokenKind::comment;
            const auto end = code.find("*/", i + 2);
            advance((end == std::string_view::npos ? code.size() : end + 2) - i);
        } else {
            advance(1);
        }
        tokens.push_back({kind, std::string(code.substr(start, i - start)), start_line, start_column});
    }
    return tokens;
}

/// (line, column) of every identifier token spelled exactly @p name.
inline std::vector<std::pair<int, int>> find_references(std::string_view code, std::string_view name) {
    std::vector<std::pair<int, int>> places;
    for (const auto& token : tokenize(code)) {
        if (token.kind == TokenKind::identifier && token.text == name) {
            places.emplace_back(token.line, token.column);
        }
    }
    return places;
}

/// The line of the first reference that follows a type or `class`/`struct` keyword – a heuristic definition.
inline std::optional<int> go_to_definition(std::string_view code, std::string_view name) {
    const auto tokens = tokenize(code);
    for (std::size_t i = 1; i < tokens.size(); ++i) {
        if (tokens[i].kind == TokenKind::identifier && tokens[i].text == name &&
            tokens[i - 1].kind == TokenKind::identifier && tokens[i - 1].text != "return") {
            return tokens[i].line;
        }
    }
    return std::nullopt;
}

/// Rename identifier tokens only: strings, comments and longer names such as `total_count` are untouched.
inline std::string rename_symbol(std::string_view code, std::string_view old_name, std::string_view new_name) {
    const auto tokens = tokenize(new_name);
    if (tokens.size() != 1 || tokens[0].kind != TokenKind::identifier) {
        throw std::invalid_argument("the new name must be a single identifier");
    }
    std::string result;
    std::size_t copied = 0;
    std::size_t offset = 0;
    // walk the original text again so whitespace and layout survive exactly
    for (const auto& token : tokenize(code)) {
        const std::size_t at = code.find(token.text, offset);
        result.append(code.substr(copied, at - copied));
        result.append(token.kind == TokenKind::identifier && token.text == old_name ? std::string(new_name) : token.text);
        copied = at + token.text.size();
        offset = copied;
    }
    result.append(code.substr(copied));
    return result;
}

/// Live templates in the style of IDE snippets; $name$ placeholders are filled from @p values.
inline std::string expand_template(std::string_view abbreviation, const std::map<std::string, std::string>& values) {
    static const std::map<std::string, std::string, std::less<>> templates{
        {"fori", "for (std::size_t $i$ = 0; $i$ < $n$; ++$i$) {\n}\n"},
        {"forr", "for (const auto& $item$ : $range$) {\n}\n"},
        {"guard", "if (!($cond$)) {\n    return $value$;\n}\n"},
        {"cls", "class $Name$ {\npublic:\n    explicit $Name$();\n};\n"},
    };
    const auto found = templates.find(abbreviation);
    if (found == templates.end()) {
        throw std::invalid_argument("unknown template");
    }
    std::string text = found->second;
    for (std::size_t start = text.find('$'); start != std::string::npos; start = text.find('$', start)) {
        const std::size_t end = text.find('$', start + 1);
        const std::string key = text.substr(start + 1, end - start - 1);
        const auto value = values.find(key);
        const std::string replacement = value == values.end() ? key : value->second;  // default: the placeholder name
        text.replace(start, end - start + 1, replacement);
        start += replacement.size();
    }
    return text;
}

/// The interactive demo: "find <action>", "rename <old> <new>" on a sample, "tpl <abbrev>".
inline int run(std::istream& in, std::ostream& out) {
    out << "Day 26 – IDE Tips and Tricks\n";
    const std::string sample =
        "int total = 0;  // the total so far\nint total_count = 1;\nprint(\"total\");\ntotal += total_count;\n";
    while (auto line = prompt_line(in, out, "find <action> | rename <old> <new> | tpl <abbrev>> ")) {
        std::istringstream words(*line);
        std::string command;
        if (!(words >> command)) {
            break;
        }
        try {
            if (command == "find") {
                std::string query;
                std::getline(words >> std::ws, query);
                for (const auto& s : search_shortcuts(query)) {
                    out << "  " << s.ide << " (" << s.os << "): " << s.action << " = " << s.keys << '\n';
                }
            } else if (command == "rename") {
                std::string from;
                std::string to;
                words >> from >> to;
                out << rename_symbol(sample, from, to);
                out << "  " << find_references(sample, from).size() << " reference(s) renamed\n";
            } else if (command == "tpl") {
                std::string abbreviation;
                words >> abbreviation;
                out << expand_template(abbreviation, {});
            } else {
                out << "  unknown command\n";
            }
        } catch (const std::invalid_argument& error) {
            out << "  " << error.what() << '\n';
        }
    }
    return 0;
}

}  // namespace cppm::day26
