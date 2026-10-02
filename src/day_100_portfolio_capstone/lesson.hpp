/**
 * @file
 * Day 100 – Capstone: Portfolio Project.
 *
 * Scenario: `budget`, a *personal-finance command-line tool* – the portfolio piece that pulls the
 * course together. It records income and spending in an append-only, checksummed ledger file,
 * reads limits per category from an INI file with environment overrides, prints monthly reports
 * with budget warnings and a CSV export, logs what it does, carries a single-sourced version from
 * the build, and is covered module by module and end to end by tests.
 *
 * Deliverables (syllabus):
 * - A CLI with subcommands and exit codes
 * - Configuration
 * - Persistent storage
 * - Reports
 * - Logging, packaging and a full test suite
 */
#pragma once

#include <filesystem>
#include <fstream>
#include <istream>
#include <map>
#include <ostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

#include "budget/config.hpp"
#include "budget/money.hpp"
#include "budget/report.hpp"
#include "budget/store.hpp"
#include "budget_version.hpp"
#include "cppm/lesson.hpp"

namespace cppm::day100 {

inline constexpr Deliverable DELIVERABLES[] = {
    {"money parsing that refuses ambiguous input", "parse_money"},
    {"INI settings with environment overrides", "load_settings"},
    {"an append-only ledger that survives a torn write", "Ledger"},
    {"monthly totals with budget warnings", "build_month"},
    {"the command line: subcommands, logging and exit codes", "run_cli"},
};

inline const char* const USAGE =
    "usage: budget [--config FILE] <command>\n"
    "  earn  <YYYY-MM-DD> <amount> <source> [note]\n"
    "  spend <YYYY-MM-DD> <amount> <category> [note]\n"
    "  list  [YYYY-MM]\n"
    "  report <YYYY-MM> [--csv]\n"
    "  --version\n";

struct UsageError : std::runtime_error {
    using std::runtime_error::runtime_error;
};

/// Appends "LEVEL message" lines to the configured log file, if any.
class Log {
  public:
    explicit Log(std::string path) : path_(std::move(path)) {}
    void info(const std::string& m) const { write("INFO", m); }
    void error(const std::string& m) const { write("ERROR", m); }

  private:
    void write(const char* level, const std::string& m) const {
        if (!path_.empty()) std::ofstream(path_, std::ios::app) << level << ' ' << m << '\n';
    }
    std::string path_;
};

inline std::string read_text(const std::string& path) {
    std::ifstream in(path, std::ios::binary);
    if (!in) throw std::runtime_error("cannot read config " + path);
    std::ostringstream s;
    s << in.rdbuf();
    return s.str();
}

/// The whole program. Exit codes: 0 ok, 1 error (bad data, I/O), 2 usage.
inline int run_cli(std::vector<std::string> args, const std::map<std::string, std::string>& env, std::ostream& out,
                   std::ostream& err) {
    try {
        if (!args.empty() && args[0] == "--version") {
            out << "budget " << BUDGET_VERSION << '\n';
            return 0;
        }
        std::string config_text;
        if (args.size() >= 2 && args[0] == "--config") {
            config_text = read_text(args[1]);
            args.erase(args.begin(), args.begin() + 2);
        }
        if (args.empty()) throw UsageError("missing command");
        const Settings settings = load_settings(config_text, env);
        const Log log(settings.log_file);
        const std::string command = args[0];
        if (command == "earn" || command == "spend") {
            if (args.size() < 4) throw UsageError(command + " needs <date> <amount> <category>");
            Cents cents = parse_money(args[2]);
            if (cents <= 0) throw std::invalid_argument("amount must be positive; use earn or spend for the direction");
            if (command == "spend") cents = -cents;
            std::string note;
            for (std::size_t k = 4; k < args.size(); ++k) note += (note.empty() ? "" : " ") + args[k];
            Ledger ledger(settings.data_file);
            if (ledger.recovered_bytes() > 0)
                log.error("discarded " + std::to_string(ledger.recovered_bytes()) +
                          " damaged byte(s) at the end of the ledger");
            const Entry e = ledger.add({0, args[1], cents, args[3], note});
            log.info(command + " #" + std::to_string(e.id) + " " + format_money(cents, settings.symbol) + " " +
                     e.category);
            out << "#" << e.id << ' ' << e.date << ' ' << format_money(e.cents, settings.symbol) << ' ' << e.category
                << '\n';
            return 0;
        }
        if (command == "list") {
            const std::string month = args.size() > 1 ? args[1] : "";
            // Named on purpose: `for (… : Ledger(file).entries())` would iterate a member of a temporary
            // that is destroyed before the loop body runs (fixed only in C++23).
            const Ledger ledger(settings.data_file);
            for (const auto& e : ledger.entries()) {
                if (month.empty() || e.month() == month) {
                    out << "#" << e.id << ' ' << e.date << ' ' << format_money(e.cents, settings.symbol) << ' '
                        << e.category << (e.note.empty() ? "" : " – " + e.note) << '\n';
                }
            }
            return 0;
        }
        if (command == "report") {
            if (args.size() < 2 || args[1].size() != 7 || !valid_date(args[1] + "-01"))
                throw UsageError("report needs <YYYY-MM>");
            const Ledger ledger(settings.data_file);
            const bool csv = args.size() > 2 && args[2] == "--csv";
            out << (csv ? render_csv(ledger.entries(), args[1])
                        : render_text(build_month(ledger.entries(), args[1], settings), settings));
            log.info("report " + args[1] + (csv ? " (csv)" : ""));
            return 0;
        }
        throw UsageError("unknown command '" + command + "'");
    } catch (const UsageError& e) {
        err << "budget: " << e.what() << '\n' << USAGE;
        return 2;
    } catch (const std::exception& e) {
        err << "budget: " << e.what() << '\n';
        return 1;
    }
}

inline std::vector<std::string> split_words(const std::string& line) {
    std::istringstream in(line);
    std::vector<std::string> words;
    for (std::string w; in >> w;) words.push_back(w);
    return words;
}

/// The interactive demo: type budget command lines; data lives in @p dir.
inline int run(std::istream& in, std::ostream& out,
               const std::filesystem::path& dir = std::filesystem::temp_directory_path() / "cppm-budget") {
    out << "Day 100 – Capstone: Portfolio Project (budget " << BUDGET_VERSION << ")\n" << USAGE;
    std::filesystem::create_directories(dir);
    const auto config = dir / "budget.ini";
    if (!std::filesystem::exists(config)) std::ofstream(config) << "[limits]\ngroceries = 400\nfun = 100\n";
    const std::map<std::string, std::string> env{{"BUDGET_DATA_FILE", (dir / "budget.ledger").string()},
                                                 {"BUDGET_LOG_FILE", (dir / "budget.log").string()}};
    while (auto line = prompt_line(in, out, "budget ")) {
        auto args = split_words(*line);
        if (args.empty()) break;
        args.insert(args.begin(), {"--config", config.string()});
        const int code = run_cli(args, env, out, out);
        if (code != 0) out << "  (exit " << code << ")\n";
    }
    return 0;
}

}  // namespace cppm::day100
