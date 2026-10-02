/**
 * @file
 * Day 14 – Code Blocks and Indentation.
 *
 * Scenario: a *snippet checker for a coding bootcamp*. Students paste C++ snippets; the
 * checker finds unbalanced brackets with line numbers, flags `if`/`else`/loops without braces
 * (the dangling-else trap), reports mixed or odd indentation, and re-indents the code.
 *
 * Deliverables (syllabus):
 * - Block scope
 * - Braces for every branch
 * - The dangling-else trap
 * - Consistent indentation style
 */
#pragma once

#include <cstddef>
#include <istream>
#include <optional>
#include <ostream>
#include <sstream>
#include <string>
#include <string_view>
#include <vector>

#include "cppm/lesson.hpp"

namespace cppm::day14 {

inline constexpr Deliverable DELIVERABLES[] = {
    {"block scope ends an object's lifetime at the closing brace", "block_scope_demo"},
    {"matching brackets with a stack", "check_brackets"},
    {"flagging control statements without braces", "find_unbraced"},
    {"else binds to the nearest if (dangling else)", "dangling_else_unbraced"},
    {"measuring indentation consistency", "indentation_report"},
    {"re-indenting by brace depth", "reindent"},
};

/// Logs its construction and destruction so the end of a block becomes visible.
class ScopeProbe {
  public:
    ScopeProbe(std::vector<std::string>& log, std::string name) : log_(log), name_(std::move(name)) {
        log_.push_back("enter " + name_);
    }
    ~ScopeProbe() { log_.push_back("leave " + name_); }
    ScopeProbe(const ScopeProbe&) = delete;
    ScopeProbe& operator=(const ScopeProbe&) = delete;

  private:
    std::vector<std::string>& log_;
    std::string name_;
};

/// Each `{ }` opens a scope; objects declared inside are destroyed at its closing brace.
inline std::vector<std::string> block_scope_demo() {
    std::vector<std::string> log;
    {
        ScopeProbe outer(log, "outer");
        for (int i = 0; i < 2; ++i) {
            ScopeProbe loop_body(log, "loop " + std::to_string(i));
        }  // loop_body is destroyed at the end of every iteration
        {
            ScopeProbe inner(log, "inner");
            log.emplace_back("inner block runs");
        }  // inner ends here, before outer
        log.emplace_back("back in outer");
    }
    return log;
}

/// Where and why a snippet's brackets do not match.
struct BracketError {
    int line;
    std::string message;
};

/// Match (), [] and {} with a stack, ignoring string/char literals and // comments.
inline std::optional<BracketError> check_brackets(std::string_view code) {
    struct Open {
        char bracket;
        int line;
    };
    std::vector<Open> stack;
    int line = 1;
    char quote = 0;
    for (std::size_t i = 0; i < code.size(); ++i) {
        const char c = code[i];
        if (c == '\n') {
            ++line;
            continue;
        }
        if (quote != 0) {
            if (c == '\\') {
                ++i;  // skip the escaped character
            } else if (c == quote) {
                quote = 0;
            }
            continue;
        }
        if (c == '"' || c == '\'') {
            quote = c;
        } else if (c == '/' && i + 1 < code.size() && code[i + 1] == '/') {
            while (i + 1 < code.size() && code[i + 1] != '\n') {
                ++i;
            }
        } else if (c == '(' || c == '[' || c == '{') {
            stack.push_back({c, line});
        } else if (c == ')' || c == ']' || c == '}') {
            const char expected = c == ')' ? '(' : c == ']' ? '[' : '{';
            if (stack.empty()) {
                return BracketError{line, std::string("unexpected '") + c + "'"};
            }
            if (stack.back().bracket != expected) {
                return BracketError{line, std::string("'") + c + "' closes '" + stack.back().bracket + "' from line " +
                                              std::to_string(stack.back().line)};
            }
            stack.pop_back();
        }
    }
    if (!stack.empty()) {
        return BracketError{stack.back().line, std::string("'") + stack.back().bracket + "' is never closed"};
    }
    return std::nullopt;
}

inline std::string trimmed(std::string_view text) {
    const auto first = text.find_first_not_of(" \t");
    if (first == std::string_view::npos) {
        return {};
    }
    const auto last = text.find_last_not_of(" \t\r");
    return std::string(text.substr(first, last - first + 1));
}

/// Line numbers of if/else/for/while statements whose body is not a braced block.
inline std::vector<int> find_unbraced(std::string_view code) {
    std::vector<int> lines;
    std::istringstream in{std::string(code)};
    std::string raw;
    int number = 0;
    while (std::getline(in, raw)) {
        ++number;
        const std::string line = trimmed(raw);
        const bool control = line.rfind("if (", 0) == 0 || line.rfind("if(", 0) == 0 || line.rfind("for (", 0) == 0 ||
                             line.rfind("while (", 0) == 0 || line == "else" || line.rfind("else ", 0) == 0 ||
                             line.rfind("} else", 0) == 0;
        const bool else_if_braced = line.find("else if") != std::string::npos && !line.empty() && line.back() == '{';
        if (control && !else_if_braced && (line.empty() || line.back() != '{')) {
            lines.push_back(number);
        }
    }
    return lines;
}

// The compiler rightly warns about the next function; silencing it here (and only here) keeps the
// trap visible for study while every other line of the project still builds with -Werror.
#if defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdangling-else"
#pragma GCC diagnostic ignored "-Wmisleading-indentation"
#endif

/// Unbraced: the `else` belongs to the *inner* if, whatever the indentation suggests.
inline std::string dangling_else_unbraced(bool member, bool has_coupon) {
    std::string result = "full price";
    // clang-format off
    if (member)
        if (has_coupon) result = "member + coupon";
    else result = "not a member?";  // actually runs for members WITHOUT a coupon
    // clang-format on
    return result;
}

#if defined(__GNUC__)
#pragma GCC diagnostic pop
#endif

/// Braced: the code means exactly what its layout says.
inline std::string dangling_else_braced(bool member, bool has_coupon) {
    std::string result = "full price";
    if (member) {
        if (has_coupon) {
            result = "member + coupon";
        }
    } else {
        result = "not a member";
    }
    return result;
}

/// Summary of how a snippet is indented.
struct IndentReport {
    int lines_with_tabs{0};
    int lines_not_multiple_of_four{0};
    bool consistent() const { return lines_with_tabs == 0 && lines_not_multiple_of_four == 0; }
};

inline IndentReport indentation_report(std::string_view code) {
    IndentReport report;
    std::istringstream in{std::string(code)};
    std::string line;
    while (std::getline(in, line)) {
        const auto width = line.find_first_not_of(" \t");
        if (width == std::string::npos) {
            continue;  // blank lines do not count
        }
        const std::string indent = line.substr(0, width);
        if (indent.find('\t') != std::string::npos) {
            ++report.lines_with_tabs;
        } else if (width % 4 != 0) {
            ++report.lines_not_multiple_of_four;
        }
    }
    return report;
}

/// Re-indent every line by 4 spaces per open brace; closing braces dedent their own line.
inline std::string reindent(std::string_view code) {
    std::istringstream in{std::string(code)};
    std::string raw;
    std::string out;
    int depth = 0;
    while (std::getline(in, raw)) {
        const std::string line = trimmed(raw);
        if (line.empty()) {
            out += '\n';
            continue;
        }
        int line_depth = depth;
        if (line.front() == '}') {
            line_depth = depth > 0 ? depth - 1 : 0;
        }
        out += std::string(static_cast<std::size_t>(line_depth) * 4, ' ') + line + '\n';
        for (const char c : line) {
            if (c == '{') {
                ++depth;
            } else if (c == '}' && depth > 0) {
                --depth;
            }
        }
    }
    return out;
}

/// The interactive demo: paste a snippet, end it with a line containing only "END".
inline int run(std::istream& in, std::ostream& out) {
    out << "Day 14 – Code Blocks and Indentation\n";
    for (const auto& entry : block_scope_demo()) {
        out << "  " << entry << '\n';
    }
    out << "Dangling else, member without coupon: unbraced says '" << dangling_else_unbraced(true, false)
        << "', braced says '" << dangling_else_braced(true, false) << "'\n";
    out << "Paste a snippet and finish with END:\n";
    std::string snippet;
    while (auto line = prompt_line(in, out, "")) {
        if (*line == "END") {
            break;
        }
        snippet += *line + '\n';
    }
    if (auto error = check_brackets(snippet)) {
        out << "Line " << error->line << ": " << error->message << '\n';
        return 0;
    }
    for (const int line : find_unbraced(snippet)) {
        out << "Line " << line << ": add braces to this block\n";
    }
    const auto report = indentation_report(snippet);
    if (!report.consistent()) {
        out << "Indentation: " << report.lines_with_tabs << " line(s) with tabs, " << report.lines_not_multiple_of_four
            << " not a multiple of 4. Re-indented:\n"
            << reindent(snippet);
    } else {
        out << "Indentation is consistent.\n";
    }
    return 0;
}

}  // namespace cppm::day14
