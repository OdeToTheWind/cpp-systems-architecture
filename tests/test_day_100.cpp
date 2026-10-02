// Tests for Day 100 – Capstone: Portfolio Project. One section per module, then end-to-end runs of
// the CLI against files in a temporary directory.
#include <filesystem>
#include <fstream>
#include <map>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

#include "cppm/temp_dir.hpp"
#include "cppm/testing.hpp"
#include "day_100_portfolio_capstone/lesson.hpp"

using namespace cppm::day100;

namespace {
struct Tool {
    cppm::TempDir dir{"day100"};
    std::string out;
    std::string err;
    std::map<std::string, std::string> env() const {
        return {{"BUDGET_DATA_FILE", (dir.path() / "b.ledger").string()}, {"BUDGET_LOG_FILE", (dir.path() / "b.log").string()}};
    }
    int operator()(std::vector<std::string> args, const std::string& config = "[limits]\ngroceries = 100\nfun = 50\n") {
        const auto ini = dir.write("budget.ini", config);
        args.insert(args.begin(), {"--config", ini.string()});
        std::ostringstream o;
        std::ostringstream e;
        const int code = run_cli(args, env(), o, e);
        out = o.str();
        err = e.str();
        return code;
    }
};
}  // namespace

TEST_CASE("money: strict parsing and formatting") {
    CHECK_EQ(parse_money("12"), 1200);
    CHECK_EQ(parse_money("12.5"), 1250);
    CHECK_EQ(parse_money("-0.99"), -99);
    for (const char* bad : {"", "12.", ".5", "1.234", "1,000", "abc", "--1"}) CHECK_THROWS_AS(parse_money(bad), std::invalid_argument);
    CHECK_EQ(format_money(123456, "$"), "$1,234.56");
    CHECK_EQ(format_money(-5, "€"), "-€0.05");
}

TEST_CASE("config: INI limits, comments and environment overrides") {
    const auto s = load_settings("[general]\nsymbol = $  # dollars\n[limits]\nrent = 1200\n", {{"BUDGET_DATA_FILE", "/tmp/x"}});
    CHECK_EQ(s.symbol, "$");
    CHECK_EQ(s.monthly_limits.at("rent"), 120000);
    CHECK_EQ(s.data_file, "/tmp/x");
    CHECK_THROWS_AS(load_settings("[general]\ncolour = red\n", {}), std::runtime_error);
    CHECK_THROWS_AS(load_settings("[limits]\nrent = lots\n", {}), std::runtime_error);
}

TEST_CASE("store: entries persist and a torn tail is dropped") {
    cppm::TempDir dir("day100");
    const auto file = dir.path() / "l.ledger";
    {
        Ledger ledger(file);
        ledger.add({0, "2026-06-01", 250000, "salary", "June"});
        ledger.add({0, "2026-06-02", -4599, "groceries", "a|b"});
        CHECK_THROWS_AS(ledger.add({0, "2026-02-30", -1, "x", ""}), std::invalid_argument);
        CHECK_THROWS_AS(ledger.add({0, "2026-06-02", 0, "x", ""}), std::invalid_argument);
    }
    std::ofstream(file, std::ios::binary | std::ios::app) << "deadbeef|3|2026-06";  // crash mid-write
    Ledger reopened(file);
    CHECK_EQ(reopened.entries().size(), 2u);
    CHECK_EQ(reopened.entries()[1].note, "a b");
    CHECK(reopened.recovered_bytes() > 0);
    CHECK_EQ(reopened.add({0, "2026-06-03", -100, "fun", ""}).id, 3);
}

TEST_CASE("report: totals, warnings and CSV") {
    Settings s;
    s.monthly_limits = {{"groceries", 10000}, {"fun", 5000}, {"rent", 100000}};
    const std::vector<Entry> entries{{1, "2026-06-01", 200000, "salary", ""}, {2, "2026-06-03", -9500, "groceries", ""},
                                     {3, "2026-06-09", -6000, "fun", "=cinema, popcorn"}, {4, "2026-07-01", -999999, "fun", ""}};
    const auto r = build_month(entries, "2026-06", s);
    CHECK_EQ(r.income, 200000);
    CHECK_EQ(r.spent, 15500);
    CHECK(r.warnings == std::vector<std::string>{"fun is over budget by €10.00", "groceries has used 95% of its budget"});
    CHECK(render_text(r, s).find("  balance  €1,845.00\n") != std::string::npos);
    CHECK_EQ(render_csv(entries, "2026-06"),
             "id,date,amount,category,note\n1,2026-06-01,2000.00,salary,\n2,2026-06-03,-95.00,groceries,\n3,2026-06-09,-60.00,fun,\"'=cinema, popcorn\"\n");
}

TEST_CASE("cli: recording and listing money") {
    Tool budget;
    CHECK_EQ(budget({"earn", "2026-06-01", "2500", "salary"}), 0);
    CHECK_EQ(budget.out, "#1 2026-06-01 €2,500.00 salary\n");
    CHECK_EQ(budget({"spend", "2026-06-02", "45.99", "groceries", "weekly", "shop"}), 0);
    CHECK_EQ(budget({"spend", "2026-07-01", "10", "fun"}), 0);
    CHECK_EQ(budget({"list", "2026-06"}), 0);
    CHECK_EQ(budget.out, "#1 2026-06-01 €2,500.00 salary\n#2 2026-06-02 -€45.99 groceries – weekly shop\n");
    std::ifstream log(budget.dir.path() / "b.log");
    std::string first;
    std::getline(log, first);
    CHECK_EQ(first, "INFO earn #1 €2,500.00 salary");
}

TEST_CASE("cli: reports and exit codes") {
    Tool budget;
    budget({"spend", "2026-06-02", "95", "groceries"});
    CHECK_EQ(budget({"report", "2026-06"}), 0);
    CHECK(budget.out.find("  ! groceries has used 95% of its budget\n") != std::string::npos);
    CHECK_EQ(budget({"report", "2026-06", "--csv"}), 0);
    CHECK_EQ(budget.out, "id,date,amount,category,note\n1,2026-06-02,-95.00,groceries,\n");
    CHECK_EQ(budget({"report", "June"}), 2);
    CHECK_EQ(budget({"fly"}), 2);
    CHECK(budget.err.find("usage: budget") != std::string::npos);
    CHECK_EQ(budget({"spend", "2026-06-31", "5", "fun"}), 1);
    CHECK_EQ(budget.err, "budget: not a date: '2026-06-31' (use YYYY-MM-DD)\n");
    CHECK_EQ(budget({"spend", "2026-06-01", "-5", "fun"}), 1);
    CHECK_EQ(budget({"list"}, "[oops]\nx = 1\n"), 1);
    std::ostringstream version;
    std::ostringstream ignored;
    CHECK_EQ(run_cli({"--version"}, {}, version, ignored), 0);
    CHECK_EQ(version.str(), std::string("budget ") + BUDGET_VERSION + "\n");
}

TEST_CASE("the interactive demo keeps data between commands") {
    cppm::TempDir dir("day100");
    std::istringstream in("earn 2026-06-01 1000 salary\nspend 2026-06-05 120 fun concert\nreport 2026-06\nspend bad 1 x\n\n");
    std::ostringstream out;
    CHECK_EQ(run(in, out, dir.path()), 0);
    const auto text = out.str();
    CHECK(text.find("  ! fun is over budget by €20.00") != std::string::npos);
    CHECK(text.find("  balance  €880.00") != std::string::npos);
    CHECK(text.find("(exit 1)") != std::string::npos);
}
