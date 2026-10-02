/**
 * @file
 * Day 83 – Capstone: A Robust CLI Application.
 *
 * Scenario: `shelf`, a *reading-list tool* used from the terminal and from scripts. It has
 * subcommands (add, list, done, stats), global options for verbosity and JSON output, `-c key=value`
 * configuration overrides, a file-backed store, and documented exit codes so shell scripts can
 * react to "not found" differently from "bad usage".
 *
 * Deliverables (syllabus):
 * - Subcommands
 * - Configuration overrides
 * - Verbosity levels
 * - JSON output
 * - Exit codes
 */
#pragma once

#include <algorithm>
#include <cctype>
#include <filesystem>
#include <fstream>
#include <istream>
#include <map>
#include <optional>
#include <ostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

#include "cppm/lesson.hpp"

namespace cppm::day83 {

namespace fs = std::filesystem;

inline constexpr Deliverable DELIVERABLES[] = {
    {"documented exit codes for scripts", "ExitCode"},
    {"global options before a subcommand", "parse_command_line"},
    {"validated -c key=value configuration overrides", "Settings::apply"},
    {"one dispatcher for every subcommand", "execute"},
    {"machine-readable JSON output", "to_json"},
};

/// Scripts test these with `$?`; they are part of the tool's public interface.
enum class ExitCode { ok = 0, failure = 1, usage = 2, not_found = 3 };

struct UsageError : std::runtime_error {
    using std::runtime_error::runtime_error;
};

inline const char* const USAGE =
    "usage: shelf [-v|-q] [--json] [-c key=value]... <command> [args]\n"
    "commands: add <title> [author] | list | done <id> | stats\n"
    "settings: list.limit (int), list.sort (id|title), store.path (file)\n";

struct Invocation {
    int verbosity = 1;  // 0 quiet, 1 normal, 2+ verbose
    bool json = false;
    std::map<std::string, std::string> overrides;
    std::string command;
    std::vector<std::string> args;
};

/// Global options come before the subcommand; everything after it belongs to the subcommand.
inline Invocation parse_command_line(const std::vector<std::string>& args) {
    Invocation inv;
    std::size_t i = 0;
    for (; i < args.size() && args[i].size() > 1 && args[i][0] == '-'; ++i) {
        const std::string& a = args[i];
        if (a == "--json") {
            inv.json = true;
        } else if (a == "-q" || a == "--quiet") {
            inv.verbosity = 0;
        } else if (a.find_first_not_of('v', 1) == std::string::npos) {  // -v, -vv, -vvv
            inv.verbosity = 1 + static_cast<int>(a.size() - 1);
        } else if (a == "-c") {
            if (i + 1 == args.size()) throw UsageError("-c needs key=value");
            const std::string& kv = args[++i];
            const auto eq = kv.find('=');
            if (eq == std::string::npos || eq == 0) throw UsageError("-c expects key=value, got '" + kv + "'");
            inv.overrides[kv.substr(0, eq)] = kv.substr(eq + 1);
        } else {
            throw UsageError("unknown option " + a);
        }
    }
    if (i == args.size()) throw UsageError("missing command");
    inv.command = args[i];
    inv.args.assign(args.begin() + static_cast<std::ptrdiff_t>(i) + 1, args.end());
    return inv;
}

/// Defaults first, then overrides; unknown keys and bad values are usage errors, reported before any work.
struct Settings {
    int list_limit = 20;
    std::string list_sort = "id";
    fs::path store_path = "shelf.tsv";

    void apply(const std::map<std::string, std::string>& overrides) {
        for (const auto& [key, value] : overrides) {
            if (key == "list.limit") {
                if (value.empty() || value.size() > 6 || value.find_first_not_of("0123456789") != std::string::npos) {
                    throw UsageError("list.limit must be a non-negative integer");
                }
                list_limit = std::stoi(value);
            } else if (key == "list.sort") {
                if (value != "id" && value != "title") throw UsageError("list.sort must be id or title");
                list_sort = value;
            } else if (key == "store.path") {
                store_path = value;
            } else {
                throw UsageError("unknown setting " + key);
            }
        }
    }
};

struct Book {
    int id = 0;
    std::string title;
    std::string author;
    bool done = false;
};

/// Tab-separated file: id, done flag, title, author. Tabs in fields are replaced on the way in.
inline std::vector<Book> load(const fs::path& path) {
    std::vector<Book> books;
    std::ifstream in(path);
    for (std::string line; std::getline(in, line);) {
        std::istringstream fields(line);
        Book b;
        std::string id;
        std::string done;
        if (std::getline(fields, id, '\t') && std::getline(fields, done, '\t') && std::getline(fields, b.title, '\t')) {
            std::getline(fields, b.author);
            b.id = std::stoi(id);
            b.done = done == "1";
            books.push_back(b);
        }
    }
    return books;
}

/// Write to a temporary file and rename it over the store, so a crash never leaves half a file.
inline void save(const fs::path& path, const std::vector<Book>& books) {
    const fs::path temp = path.string() + ".tmp";
    {
        std::ofstream out(temp, std::ios::trunc);
        for (const auto& b : books)
            out << b.id << '\t' << (b.done ? 1 : 0) << '\t' << b.title << '\t' << b.author << '\n';
        if (!out.flush()) throw std::runtime_error("cannot write " + temp.string());
    }
    fs::rename(temp, path);
}

inline std::string json_string(const std::string& s) {
    std::string out = "\"";
    for (const char c : s) {
        if (c == '"' || c == '\\') out += '\\';
        out += c;
    }
    return out + "\"";
}

inline std::string to_json(const Book& b) {
    return "{\"id\":" + std::to_string(b.id) + ",\"title\":" + json_string(b.title) +
           ",\"author\":" + json_string(b.author) + ",\"done\":" + (b.done ? "true" : "false") + "}";
}

inline std::string to_json(const std::vector<Book>& books) {
    std::string out = "[";
    for (std::size_t i = 0; i < books.size(); ++i) out += (i ? "," : "") + to_json(books[i]);
    return out + "]";
}

inline std::string clean(std::string field) {
    std::replace(field.begin(), field.end(), '\t', ' ');
    std::replace(field.begin(), field.end(), '\n', ' ');
    return field;
}

/// Run one subcommand. Results go to @p out, diagnostics to @p err (so piping JSON stays clean).
inline ExitCode execute(const Invocation& inv, const Settings& settings, std::ostream& out, std::ostream& err) {
    auto books = load(settings.store_path);
    auto verbose = [&](const std::string& message) {
        if (inv.verbosity >= 2) err << "[debug] " << message << '\n';
    };
    verbose("store " + settings.store_path.string() + " has " + std::to_string(books.size()) + " book(s)");
    if (inv.command == "add") {
        if (inv.args.empty() || inv.args.size() > 2 || inv.args[0].empty())
            throw UsageError("add needs <title> [author]");
        int next = 1;
        for (const auto& b : books) next = std::max(next, b.id + 1);
        books.push_back({next, clean(inv.args[0]), inv.args.size() > 1 ? clean(inv.args[1]) : "", false});
        save(settings.store_path, books);
        if (inv.json)
            out << to_json(books.back()) << '\n';
        else if (inv.verbosity > 0)
            out << "added #" << next << ' ' << books.back().title << '\n';
        return ExitCode::ok;
    }
    if (inv.command == "list") {
        if (!inv.args.empty()) throw UsageError("list takes no arguments");
        if (settings.list_sort == "title") {
            std::stable_sort(books.begin(), books.end(),
                             [](const Book& a, const Book& b) { return a.title < b.title; });
        }
        if (books.size() > static_cast<std::size_t>(settings.list_limit))
            books.resize(static_cast<std::size_t>(settings.list_limit));
        if (inv.json) {
            out << to_json(books) << '\n';
        } else {
            for (const auto& b : books) {
                out << (b.done ? "[x] " : "[ ] ") << '#' << b.id << ' ' << b.title
                    << (b.author.empty() ? "" : " – " + b.author) << '\n';
            }
        }
        return ExitCode::ok;
    }
    if (inv.command == "done") {
        if (inv.args.size() != 1 || inv.args[0].empty() ||
            inv.args[0].find_first_not_of("0123456789") != std::string::npos) {
            throw UsageError("done needs a numeric <id>");
        }
        const int id = std::stoi(inv.args[0]);
        const auto it = std::find_if(books.begin(), books.end(), [id](const Book& b) { return b.id == id; });
        if (it == books.end()) {
            err << "shelf: no book #" << id << '\n';
            return ExitCode::not_found;
        }
        it->done = true;
        save(settings.store_path, books);
        if (inv.json)
            out << to_json(*it) << '\n';
        else if (inv.verbosity > 0)
            out << "finished #" << id << ' ' << it->title << '\n';
        return ExitCode::ok;
    }
    if (inv.command == "stats") {
        const auto finished = std::count_if(books.begin(), books.end(), [](const Book& b) { return b.done; });
        if (inv.json)
            out << "{\"books\":" << books.size() << ",\"done\":" << finished << "}\n";
        else
            out << books.size() << " book(s), " << finished << " finished\n";
        return ExitCode::ok;
    }
    throw UsageError("unknown command '" + inv.command + "'");
}

/// The whole tool: parse, configure, execute, and map every failure to an exit code.
inline int run(const std::vector<std::string>& args, std::ostream& out, std::ostream& err) {
    try {
        const Invocation inv = parse_command_line(args);
        Settings settings;
        settings.apply(inv.overrides);
        return static_cast<int>(execute(inv, settings, out, err));
    } catch (const UsageError& e) {
        err << "shelf: " << e.what() << '\n' << USAGE;
        return static_cast<int>(ExitCode::usage);
    } catch (const std::exception& e) {
        err << "shelf: error: " << e.what() << '\n';
        return static_cast<int>(ExitCode::failure);
    }
}

inline std::vector<std::string> arguments_from(int argc, const char* const* argv) {
    return {argv + 1, argv + argc};
}

/// Split a typed command line, honouring "double quotes" for titles with spaces.
inline std::vector<std::string> split_words(const std::string& line) {
    std::vector<std::string> words;
    std::string current;
    bool quoted = false;
    bool have = false;
    for (const char c : line) {
        if (c == '"') {
            quoted = !quoted;
            have = true;
        } else if (std::isspace(static_cast<unsigned char>(c)) && !quoted) {
            if (have) words.push_back(current);
            current.clear();
            have = false;
        } else {
            current += c;
            have = true;
        }
    }
    if (have) words.push_back(current);
    return words;
}

/// The interactive demo: type shelf command lines (store in the temp directory); prints each exit code.
inline int run(std::istream& in, std::ostream& out,
               const fs::path& store = fs::temp_directory_path() / "cppm-shelf.tsv") {
    out << "Day 83 – Capstone: A Robust CLI Application\n" << USAGE;
    while (auto line = prompt_line(in, out, "shelf ")) {
        auto args = split_words(*line);
        if (args.empty()) break;
        args.insert(args.begin(), {"-c", "store.path=" + store.string()});
        const int code = run(args, out, out);
        out << "  (exit " << code << ")\n";
    }
    return 0;
}

}  // namespace cppm::day83
