// Tests for Day 83 – Capstone: A Robust CLI Application. Each test uses its own store file.
#include <sstream>
#include <string>
#include <vector>

#include "cppm/temp_dir.hpp"
#include "cppm/testing.hpp"
#include "day_83_robust_cli/lesson.hpp"

using namespace cppm::day83;

namespace {
struct Cli {
    cppm::TempDir dir{"day83"};
    std::string out;
    std::string err;
    int operator()(std::vector<std::string> args) {
        args.insert(args.begin(), {"-c", "store.path=" + (dir.path() / "shelf.tsv").string()});
        std::ostringstream o;
        std::ostringstream e;
        const int code = run(args, o, e);
        out = o.str();
        err = e.str();
        return code;
    }
};
}  // namespace

TEST_CASE("global options are parsed before the subcommand") {
    const auto inv = parse_command_line({"-vv", "--json", "-c", "list.limit=5", "list", "-x"});
    CHECK_EQ(inv.verbosity, 3);
    CHECK(inv.json);
    CHECK_EQ(inv.overrides.at("list.limit"), "5");
    CHECK_EQ(inv.command, "list");
    CHECK(inv.args == std::vector<std::string>{"-x"});  // after the command: the subcommand's
    CHECK_EQ(parse_command_line({"-q", "stats"}).verbosity, 0);
    CHECK_THROWS_AS(parse_command_line({"--json"}), UsageError);
    CHECK_THROWS_AS(parse_command_line({"-c", "novalue", "list"}), UsageError);
    CHECK_THROWS_AS(parse_command_line({"--colour", "list"}), UsageError);
}

TEST_CASE("settings overrides are validated") {
    Settings s;
    s.apply({{"list.limit", "3"}, {"list.sort", "title"}});
    CHECK_EQ(s.list_limit, 3);
    CHECK_EQ(s.list_sort, "title");
    CHECK_THROWS_AS(s.apply({{"list.limit", "-1"}}), UsageError);
    CHECK_THROWS_AS(s.apply({{"list.sort", "author"}}), UsageError);
    CHECK_THROWS_AS(s.apply({{"colour", "red"}}), UsageError);
}

TEST_CASE("add, list and done work against the store") {
    Cli shelf;
    CHECK_EQ(shelf({"add", "Dune", "Frank Herbert"}), 0);
    CHECK_EQ(shelf.out, "added #1 Dune\n");
    shelf({"add", "Beloved", "Toni Morrison"});
    CHECK_EQ(shelf({"done", "1"}), 0);
    shelf({"list"});
    CHECK_EQ(shelf.out, "[x] #1 Dune – Frank Herbert\n[ ] #2 Beloved – Toni Morrison\n");
    shelf({"-c", "list.sort=title", "-c", "list.limit=1", "list"});
    CHECK_EQ(shelf.out, "[ ] #2 Beloved – Toni Morrison\n");
}

TEST_CASE("JSON output is valid and escaped") {
    Cli shelf;
    shelf({"--json", "add", "The \"Real\" Thing", ""});
    CHECK_EQ(shelf.out, "{\"id\":1,\"title\":\"The \\\"Real\\\" Thing\",\"author\":\"\",\"done\":false}\n");
    shelf({"--json", "list"});
    CHECK(shelf.out.front() == '[');
    shelf({"--json", "stats"});
    CHECK_EQ(shelf.out, "{\"books\":1,\"done\":0}\n");
}

TEST_CASE("exit codes distinguish usage errors, missing items and success") {
    Cli shelf;
    CHECK_EQ(shelf({"fly"}), 2);
    CHECK(shelf.err.find("unknown command 'fly'") != std::string::npos);
    CHECK(shelf.err.find("usage: shelf") != std::string::npos);
    CHECK(shelf.out.empty());  // errors never pollute standard output
    CHECK_EQ(shelf({"done", "42"}), 3);
    CHECK_EQ(shelf.err, "shelf: no book #42\n");
    CHECK_EQ(shelf({"done", "abc"}), 2);
    CHECK_EQ(shelf({"add"}), 2);
    CHECK_EQ(shelf({"-c", "list.limit=x", "list"}), 2);
}

TEST_CASE("verbosity controls chatter, not results") {
    Cli shelf;
    CHECK_EQ(shelf({"-q", "add", "Emma"}), 0);
    CHECK(shelf.out.empty());
    shelf({"-vv", "list"});
    CHECK_EQ(shelf.out, "[ ] #1 Emma\n");
    CHECK(shelf.err.find("[debug] store") != std::string::npos);
    CHECK_EQ(shelf({"-c", "store.path=/nonexistent-dir/x/shelf.tsv", "add", "X"}), 1);
}

TEST_CASE("the interactive demo splits quoted words and shows exit codes") {
    CHECK(split_words(R"(add "War and Peace" Tolstoy)") == std::vector<std::string>{"add", "War and Peace", "Tolstoy"});
    CHECK(split_words(R"(add "")") == std::vector<std::string>{"add", ""});
    cppm::TempDir dir("day83");
    std::istringstream in("add \"Middlemarch\" Eliot\nstats\ndone 9\n\n");
    std::ostringstream out;
    CHECK_EQ(run(in, out, dir.path() / "s.tsv"), 0);
    CHECK(out.str().find("added #1 Middlemarch") != std::string::npos);
    CHECK(out.str().find("1 book(s), 0 finished") != std::string::npos);
    CHECK(out.str().find("(exit 3)") != std::string::npos);
}
