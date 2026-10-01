// Tests for Day 14 – Code Blocks and Indentation.
#include <sstream>
#include <string>
#include <vector>

#include "cppm/testing.hpp"
#include "day_14_code_blocks/lesson.hpp"

using namespace cppm::day14;

TEST_CASE("objects die at the closing brace of their block") {
    const std::vector<std::string> expected{"enter outer",      "enter loop 0", "leave loop 0",  "enter loop 1",
                                            "leave loop 1",     "enter inner",  "inner block runs", "leave inner",
                                            "back in outer",    "leave outer"};
    CHECK(block_scope_demo() == expected);
}

TEST_CASE("check_brackets accepts balanced code and ignores literals and comments") {
    CHECK(!check_brackets("int main() {\n  return f(a[0]);\n}\n").has_value());
    CHECK(!check_brackets("auto s = \"}{\"; // ) unmatched in a comment\nchar c = '(';\n").has_value());
}

TEST_CASE("check_brackets reports the line and kind of every mismatch") {
    const auto unclosed = check_brackets("int main() {\n  if (x) {\n}\n");
    REQUIRE(unclosed.has_value());
    CHECK_EQ(unclosed->line, 1);
    CHECK_EQ(unclosed->message, "'{' is never closed");
    const auto crossed = check_brackets("f(a[1)]\n");
    REQUIRE(crossed.has_value());
    CHECK_EQ(crossed->message, "')' closes '[' from line 1");
    CHECK_EQ(check_brackets("}\n").value_or(BracketError{0, ""}).message, "unexpected '}'");
}

TEST_CASE("find_unbraced flags control statements without a block") {
    const std::string code =
        "if (ready)\n"
        "    go();\n"
        "for (int i = 0; i < 3; ++i) {\n"
        "}\n"
        "} else if (x) {\n"
        "while (busy) wait();\n"
        "else\n";
    CHECK(find_unbraced(code) == std::vector<int>{1, 6, 7});
}

TEST_CASE("an unbraced else binds to the nearest if") {
    CHECK_EQ(dangling_else_unbraced(true, false), "not a member?");
    CHECK_EQ(dangling_else_unbraced(false, false), "full price");
    CHECK_EQ(dangling_else_braced(true, false), "full price");
    CHECK_EQ(dangling_else_braced(false, false), "not a member");
    CHECK_EQ(dangling_else_braced(true, true), "member + coupon");
}

TEST_CASE("indentation_report counts tabs and odd widths") {
    const auto report = indentation_report("int x;\n\tint y;\n   int z;\n    int w;\n\n");
    CHECK_EQ(report.lines_with_tabs, 1);
    CHECK_EQ(report.lines_not_multiple_of_four, 1);
    CHECK(!report.consistent());
    CHECK(indentation_report("a\n    b\n").consistent());
}

TEST_CASE("reindent uses four spaces per brace level") {
    const std::string messy = "int f() {\n  if (x) {\nreturn 1;\n      }\n return 0;\n}\n";
    const std::string clean = "int f() {\n    if (x) {\n        return 1;\n    }\n    return 0;\n}\n";
    CHECK_EQ(reindent(messy), clean);
    CHECK(indentation_report(reindent(messy)).consistent());
}

TEST_CASE("run checks a snippet and prints a re-indented version") {
    std::istringstream in("if (a)\n\tb();\nEND\n");
    std::ostringstream out;
    CHECK_EQ(run(in, out), 0);
    const auto text = out.str();
    CHECK(text.find("leave inner") != std::string::npos);
    CHECK(text.find("unbraced says 'not a member?'") != std::string::npos);
    CHECK(text.find("Line 1: add braces") != std::string::npos);
    CHECK(text.find("1 line(s) with tabs") != std::string::npos);
    std::istringstream broken("f(\nEND\n");
    std::ostringstream out2;
    run(broken, out2);
    CHECK(out2.str().find("Line 1: '(' is never closed") != std::string::npos);
}
