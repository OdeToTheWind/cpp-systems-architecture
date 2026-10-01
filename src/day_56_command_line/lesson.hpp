/**
 * @file
 * Day 56 – Command-Line Arguments.
 *
 * Scenario: `logscan`, a *log-search command-line tool* for an operations team. It takes flags
 * (-i, -n, -c), options with values (-m 5 or --max=5), a pattern and any number of files, prints
 * a usage message on --help or on a mistake, and returns grep-style exit codes so scripts can
 * react: 0 = matches found, 1 = no match, 2 = usage or file error.
 *
 * Deliverables (syllabus):
 * - argc and argv
 * - Flags and options
 * - Positional arguments
 * - Usage messages and exit codes
 */
#pragma once

#include <algorithm>
#include <cctype>
#include <fstream>
#include <functional>
#include <istream>
#include <memory>
#include <optional>
#include <ostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

#include "cppm/lesson.hpp"

namespace cppm::day56 {

inline constexpr Deliverable DELIVERABLES[] = {
    {"turning argc/argv into a vector of strings", "arguments_from"},
    {"parsing flags, options and positionals", "parse_args"},
    {"a usage message", "usage"},
    {"grep-style exit codes", "ExitCode"},
    {"executing the parsed command", "execute"},
};

enum ExitCode : int { matched = 0, no_match = 1, usage_error = 2 };

struct Options {
    bool ignore_case{false};
    bool line_numbers{false};
    bool count_only{false};
    std::optional<int> max_matches;
    std::string pattern;
    std::vector<std::string> files;  // empty or "-" means standard input
    bool help{false};
};

/// argv[0] is the program name; the arguments proper start at argv[1].
inline std::vector<std::string> arguments_from(int argc, const char* const* argv) {
    return argc > 1 ? std::vector<std::string>(argv + 1, argv + argc) : std::vector<std::string>{};
}

inline std::string usage() {
    return "usage: logscan [-i] [-n] [-c] [-m N | --max=N] PATTERN [FILE...]\n"
           "  -i          ignore case\n"
           "  -n          prefix matches with their line number\n"
           "  -c          print only the number of matching lines\n"
           "  -m, --max   stop after N matches\n"
           "  --help      show this message\n"
           "exit status: 0 match found, 1 no match, 2 error\n";
}

/// Parse the arguments; throws std::invalid_argument with a message for the user.
inline Options parse_args(const std::vector<std::string>& args) {
    Options options;
    const auto parse_max = [&](const std::string& text) {
        std::istringstream in(text);
        int value = 0;
        if (!(in >> value) || !(in >> std::ws).eof() || value <= 0) {
            throw std::invalid_argument("--max needs a positive number, got '" + text + "'");
        }
        options.max_matches = value;
    };
    bool options_ended = false;
    std::vector<std::string> positional;
    for (std::size_t i = 0; i < args.size(); ++i) {
        const std::string& arg = args[i];
        if (options_ended || arg == "-" || arg.empty() || arg[0] != '-') {
            positional.push_back(arg);
        } else if (arg == "--") {
            options_ended = true;  // everything after -- is positional, even "-x"
        } else if (arg == "--help" || arg == "-h") {
            options.help = true;
        } else if (arg.rfind("--max=", 0) == 0) {
            parse_max(arg.substr(6));
        } else if (arg == "-m" || arg == "--max") {
            if (i + 1 == args.size()) throw std::invalid_argument(arg + " needs a value");
            parse_max(args[++i]);
        } else if (arg.size() >= 2 && arg[1] != '-') {
            for (std::size_t k = 1; k < arg.size(); ++k) {  // combined short flags: -in
                switch (arg[k]) {
                    case 'i': options.ignore_case = true; break;
                    case 'n': options.line_numbers = true; break;
                    case 'c': options.count_only = true; break;
                    default: throw std::invalid_argument(std::string("unknown flag -") + arg[k]);
                }
            }
        } else {
            throw std::invalid_argument("unknown option " + arg);
        }
    }
    if (options.help) return options;
    if (positional.empty()) throw std::invalid_argument("a PATTERN is required");
    options.pattern = positional.front();
    options.files.assign(positional.begin() + 1, positional.end());
    return options;
}

inline std::string lowered(std::string text) {
    std::transform(text.begin(), text.end(), text.begin(), [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    return text;
}

/// Run the search. @p open turns a file name into a stream (tests inject in-memory files).
inline int execute(const Options& options, std::istream& stdin_stream, std::ostream& out, std::ostream& err,
                   const std::function<std::unique_ptr<std::istream>(const std::string&)>& open) {
    if (options.help) {
        out << usage();
        return matched;
    }
    const std::string needle = options.ignore_case ? lowered(options.pattern) : options.pattern;
    const std::vector<std::string> files = options.files.empty() ? std::vector<std::string>{"-"} : options.files;
    int total = 0;
    bool file_error = false;
    for (const auto& name : files) {
        std::unique_ptr<std::istream> owned;
        std::istream* input = &stdin_stream;
        if (name != "-") {
            owned = open(name);
            if (!owned || !*owned) {
                err << "logscan: cannot open " << name << '\n';
                file_error = true;
                continue;
            }
            input = owned.get();
        }
        std::string line;
        int number = 0;
        int count = 0;
        while (std::getline(*input, line)) {
            ++number;
            if (options.max_matches && total >= *options.max_matches) break;
            const std::string haystack = options.ignore_case ? lowered(line) : line;
            if (haystack.find(needle) == std::string::npos) continue;
            ++count;
            ++total;
            if (!options.count_only) {
                if (files.size() > 1) out << name << ':';
                if (options.line_numbers) out << number << ':';
                out << line << '\n';
            }
        }
        if (options.count_only) out << (files.size() > 1 ? name + ":" : "") << count << '\n';
    }
    if (file_error) return usage_error;
    return total > 0 ? matched : no_match;
}

/// Entry point for real command lines: logscan -n ERROR app.log
inline int run(const std::vector<std::string>& args, std::istream& in, std::ostream& out, std::ostream& err) {
    try {
        return execute(parse_args(args), in, out, err, [](const std::string& name) -> std::unique_ptr<std::istream> {
            return std::make_unique<std::ifstream>(name);
        });
    } catch (const std::invalid_argument& error) {
        err << "logscan: " << error.what() << '\n' << usage();
        return usage_error;
    }
}

/// The interactive demo when no arguments are given: type a command line, then the text to search.
inline int run(std::istream& in, std::ostream& out) {
    out << "Day 56 – Command-Line Arguments\n" << usage();
    auto line = prompt_line(in, out, "logscan arguments> ");
    if (!line) return usage_error;
    std::istringstream words(*line);
    std::vector<std::string> args;
    for (std::string word; words >> word;) args.push_back(word);
    out << "Now type the log lines (end with END):\n";
    std::string text;
    while (auto log_line = prompt_line(in, out, "")) {
        if (*log_line == "END") break;
        text += *log_line + '\n';
    }
    std::istringstream log(text);
    const int code = run(args, log, out, out);
    out << "exit status " << code << '\n';
    return 0;
}

}  // namespace cppm::day56
