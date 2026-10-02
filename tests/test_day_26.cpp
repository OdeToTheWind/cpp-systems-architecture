// Tests for Day 26 – IDE Tips and Tricks.
#include <sstream>
#include <stdexcept>
#include <string>

#include "cppm/testing.hpp"
#include "day_26_ide_tips/lesson.hpp"

using namespace cppm::day26;

TEST_CASE("search_shortcuts filters by action, IDE and operating system") {
    const auto renames = search_shortcuts("RENAME");
    CHECK_EQ(renames.size(), 5u);
    const auto clion_mac = search_shortcuts("definition", "clion", "macos");
    REQUIRE_EQ(clion_mac.size(), 1u);
    CHECK_EQ(clion_mac[0].keys, "Cmd+B");
    CHECK(search_shortcuts("teleport").empty());
}

TEST_CASE("tokenize separates identifiers, literals and comments with positions") {
    const auto tokens = tokenize("int x = 42; // note\nauto s = \"a b\";");
    REQUIRE(tokens.size() >= 10);
    CHECK(tokens[0].kind == TokenKind::identifier);
    CHECK(tokens[3].kind == TokenKind::number);
    CHECK_EQ(tokens[5].text, "// note");
    CHECK(tokens[5].kind == TokenKind::comment);
    CHECK_EQ(tokens[6].line, 2);
    CHECK_EQ(tokens[6].column, 1);
    CHECK_EQ(tokens[9].text, "\"a b\"");
}

TEST_CASE("find_references reports only whole identifiers") {
    const std::string code = "int total = 0;\nint total_count = total + 1; // total\n";
    const auto places = find_references(code, "total");
    REQUIRE_EQ(places.size(), 2u);
    CHECK(places[0] == std::pair<int, int>{1, 5});
    CHECK(places[1] == std::pair<int, int>{2, 19});
}

TEST_CASE("go_to_definition finds the declaring line") {
    const std::string code = "void start();\nint total = 0;\nvoid f() { return total; }\n";
    CHECK_EQ(go_to_definition(code, "total").value_or(-1), 2);
    CHECK(!go_to_definition(code, "missing").has_value());
}

TEST_CASE("rename_symbol leaves strings, comments and longer names alone") {
    const std::string code = "int total = 0;  // total so far\nint total_count = 1;\nprint(\"total\");\ntotal += 1;\n";
    const std::string expected = "int sum = 0;  // total so far\nint total_count = 1;\nprint(\"total\");\nsum += 1;\n";
    CHECK_EQ(rename_symbol(code, "total", "sum"), expected);
    CHECK_THROWS_AS(rename_symbol(code, "total", "two words"), std::invalid_argument);
    CHECK_THROWS_AS(rename_symbol(code, "total", "9lives"), std::invalid_argument);
}

TEST_CASE("expand_template fills placeholders and keeps defaults") {
    CHECK_EQ(expand_template("fori", {{"i", "row"}, {"n", "rows"}}),
             "for (std::size_t row = 0; row < rows; ++row) {\n}\n");
    CHECK_EQ(expand_template("guard", {}), "if (!(cond)) {\n    return value;\n}\n");
    CHECK_THROWS_AS(expand_template("nope", {}), std::invalid_argument);
}

TEST_CASE("run searches shortcuts, renames and expands templates") {
    std::istringstream in("find rename symbol\nrename total sum\ntpl cls\ntpl xyz\nhelp\n\n");
    std::ostringstream out;
    CHECK_EQ(run(in, out), 0);
    const auto text = out.str();
    CHECK(text.find("clion (windows): rename symbol = Shift+F6") != std::string::npos);
    CHECK(text.find("int sum = 0;") != std::string::npos);
    CHECK(text.find("2 reference(s) renamed") != std::string::npos);
    CHECK(text.find("class Name {") != std::string::npos);
    CHECK(text.find("unknown template") != std::string::npos);
    CHECK(text.find("unknown command") != std::string::npos);
}
