// Tests for Day 56 – Command-Line Arguments. Files are in-memory streams injected into execute().
#include <map>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

#include "cppm/testing.hpp"
#include "day_56_command_line/lesson.hpp"

using namespace cppm::day56;

namespace {
const std::map<std::string, std::string> files{
    {"app.log", "INFO start\nERROR disk full\nINFO retry\nerror timeout\nERROR disk full again\n"},
    {"db.log", "ERROR deadlock\n"},
};
std::unique_ptr<std::istream> open_fake(const std::string& name) {
    const auto it = files.find(name);
    if (it == files.end()) return nullptr;
    return std::make_unique<std::istringstream>(it->second);
}
int scan(const std::vector<std::string>& args, std::string& out_text, std::string& err_text) {
    std::istringstream in("stdin ERROR line\n");
    std::ostringstream out;
    std::ostringstream err;
    int code = 0;
    try {
        code = execute(parse_args(args), in, out, err, open_fake);
    } catch (const std::invalid_argument& e) {
        err << e.what();
        code = usage_error;
    }
    out_text = out.str();
    err_text = err.str();
    return code;
}
}  // namespace

TEST_CASE("argv[0] is skipped when converting arguments") {
    const char* argv[] = {"logscan", "-n", "ERROR", "app.log"};
    CHECK(arguments_from(4, argv) == std::vector<std::string>{"-n", "ERROR", "app.log"});
    CHECK(arguments_from(1, argv).empty());
}

TEST_CASE("flags, combined flags and options are parsed") {
    const Options a = parse_args({"-in", "--max=3", "error", "app.log", "db.log"});
    CHECK(a.ignore_case);
    CHECK(a.line_numbers);
    CHECK(!a.count_only);
    CHECK_EQ(a.max_matches.value_or(0), 3);
    CHECK_EQ(a.pattern, "error");
    CHECK(a.files == std::vector<std::string>{"app.log", "db.log"});
    CHECK_EQ(parse_args({"-m", "7", "x"}).max_matches.value_or(0), 7);
    CHECK_EQ(parse_args({"--", "-weird-pattern"}).pattern, "-weird-pattern");
}

TEST_CASE("mistakes produce clear messages") {
    CHECK_THROWS_AS(parse_args({}), std::invalid_argument);
    CHECK_THROWS_AS(parse_args({"-x", "a"}), std::invalid_argument);
    CHECK_THROWS_AS(parse_args({"--max=0", "a"}), std::invalid_argument);
    CHECK_THROWS_AS(parse_args({"-m"}), std::invalid_argument);
    CHECK_THROWS_AS(parse_args({"--colour", "a"}), std::invalid_argument);
    CHECK(parse_args({"--help"}).help);
}

TEST_CASE("matching lines are printed with optional line numbers and file names") {
    std::string out;
    std::string err;
    CHECK_EQ(scan({"-n", "ERROR", "app.log"}, out, err), matched);
    CHECK_EQ(out, "2:ERROR disk full\n5:ERROR disk full again\n");
    CHECK_EQ(scan({"ERROR", "app.log", "db.log"}, out, err), matched);
    CHECK(out.find("db.log:ERROR deadlock") != std::string::npos);
}

TEST_CASE("ignore-case, count and max change the output") {
    std::string out;
    std::string err;
    scan({"-ic", "error", "app.log"}, out, err);
    CHECK_EQ(out, "3\n");
    scan({"-i", "--max=1", "error", "app.log"}, out, err);
    CHECK_EQ(out, "ERROR disk full\n");
}

TEST_CASE("exit codes follow the grep convention") {
    std::string out;
    std::string err;
    CHECK_EQ(scan({"panic", "app.log"}, out, err), no_match);
    CHECK_EQ(scan({"ERROR", "missing.log"}, out, err), usage_error);
    CHECK(err.find("cannot open missing.log") != std::string::npos);
    CHECK_EQ(scan({"ERROR"}, out, err), matched);  // no file: standard input
    CHECK_EQ(out, "stdin ERROR line\n");
    CHECK_EQ(scan({"--help"}, out, err), matched);
    CHECK(out.find("usage: logscan") == 0u);
}

TEST_CASE("run reports usage errors on stderr with status 2") {
    std::istringstream in("");
    std::ostringstream out;
    std::ostringstream err;
    CHECK_EQ(run({"-q", "x"}, in, out, err), usage_error);
    CHECK(err.str().find("logscan: unknown flag -q") != std::string::npos);
    CHECK(err.str().find("usage:") != std::string::npos);
}

TEST_CASE("the interactive demo runs one command over typed lines") {
    std::istringstream in("-c ERROR\nERROR a\nok\nERROR b\nEND\n");
    std::ostringstream out;
    CHECK_EQ(run(in, out), 0);
    CHECK(out.str().find("2\nexit status 0") != std::string::npos);
}
