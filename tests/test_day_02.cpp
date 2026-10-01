// Tests for Day 02 – String Manipulation.
#include <sstream>
#include <string>

#include "cppm/testing.hpp"
#include "day_02_strings/lesson.hpp"

using namespace cppm::day02;

TEST_CASE("trim removes surrounding whitespace only") {
    CHECK_EQ(trim("  ada  "), "ada");
    CHECK_EQ(trim("\tline\r\n"), "line");
    CHECK_EQ(trim("a b"), "a b");
    CHECK_EQ(trim("   "), "");
    CHECK_EQ(trim(""), "");
}

TEST_CASE("to_title_case normalises case and inner spacing") {
    CHECK_EQ(to_title_case("  ada   LOVELACE "), "Ada Lovelace");
    CHECK_EQ(to_title_case("jean-luc o'neil"), "Jean-Luc O'Neil");
    CHECK_EQ(to_title_case(""), "");
}

TEST_CASE("split_once splits at the first delimiter and trims both halves") {
    const auto parts = split_once(" grace ; navy ; extra ", ';');
    REQUIRE(parts.has_value());
    CHECK_EQ(parts->first, "grace");
    CHECK_EQ(parts->second, "navy ; extra");
    CHECK(!split_once("no delimiter", ';').has_value());
}

TEST_CASE("count_occurrences is case-insensitive and non-overlapping") {
    CHECK_EQ(count_occurrences("C++ and c++ and C", "c++"), 2u);
    CHECK_EQ(count_occurrences("aaaa", "aa"), 2u);
    CHECK_EQ(count_occurrences("anything", ""), 0u);
}

TEST_CASE("centre pads evenly and shortens text that does not fit") {
    CHECK_EQ(centre("ab", 6), "  ab  ");
    CHECK_EQ(centre("abc", 6), " abc  ");
    CHECK_EQ(centre("abcdefghij", 6), "abc...");
    CHECK_EQ(centre("abcdef", 2), "ab");
}

TEST_CASE("make_badge produces a fixed-width frame with cleaned text") {
    const auto badge = make_badge("  ada LOVELACE ;  analytical engines ltd ", 20);
    std::istringstream lines(badge);
    std::string line;
    int count = 0;
    while (std::getline(lines, line)) {
        CHECK_EQ(line.size(), 24u);
        ++count;
    }
    CHECK_EQ(count, 4);
    CHECK(badge.find("Ada Lovelace") != std::string::npos);
    CHECK(badge.find("Analytical Engine...") != std::string::npos);
}

TEST_CASE("make_badge falls back to sensible defaults") {
    CHECK(make_badge("linus").find("Independent") != std::string::npos);
    CHECK(make_badge(" ; acme").find("Guest") != std::string::npos);
}

TEST_CASE("is_palindrome ignores case and punctuation but not digits") {
    CHECK(is_palindrome("A man, a plan, a canal: Panama"));
    CHECK(is_palindrome("No 'x' in Nixon"));
    CHECK(is_palindrome(""));
    CHECK(!is_palindrome("1a2"));
    CHECK(!is_palindrome("race a car"));
}

TEST_CASE("run prints one badge per row until a blank line") {
    std::istringstream in("ada ; acme\nbob\n\nignored ; row\n");
    std::ostringstream out;
    CHECK_EQ(run(in, out), 0);
    CHECK(out.str().find("2 badge(s) printed") != std::string::npos);
    CHECK(out.str().find("Ignored") == std::string::npos);
}
