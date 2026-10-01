// Tests for Day 04 – Variable Naming Rules.
#include <sstream>
#include <string>

#include "cppm/testing.hpp"
#include "day_04_variable_names/lesson.hpp"

using namespace cppm::day04;

TEST_CASE("keywords are recognised, including C++20 additions") {
    CHECK(is_keyword("class"));
    CHECK(is_keyword("co_await"));
    CHECK(is_keyword("requires"));
    CHECK(!is_keyword("classic"));
}

TEST_CASE("legal identifiers follow the grammar and avoid keywords") {
    CHECK(is_legal_identifier("user_age"));
    CHECK(is_legal_identifier("_counter"));
    CHECK(is_legal_identifier("x2"));
    CHECK(!is_legal_identifier("2x"));
    CHECK(!is_legal_identifier("user-age"));
    CHECK(!is_legal_identifier("int"));
    CHECK(!is_legal_identifier(""));
}

TEST_CASE("reserved names are flagged") {
    CHECK(is_reserved_name("__count"));
    CHECK(is_reserved_name("my__value"));
    CHECK(is_reserved_name("_Config"));
    CHECK(!is_reserved_name("_config"));
    CHECK(!is_reserved_name("config_"));
}

TEST_CASE("detect_style tells the conventions apart") {
    CHECK(detect_style("user_age") == Style::snake_case);
    CHECK(detect_style("UserAccount") == Style::pascal_case);
    CHECK(detect_style("userAge") == Style::camel_case);
    CHECK(detect_style("MAX_USERS") == Style::upper_snake_case);
    CHECK(detect_style("Mixed_Case") == Style::other);
}

TEST_CASE("to_snake_case handles camelCase and acronyms") {
    CHECK_EQ(to_snake_case("userAgeInYears"), "user_age_in_years");
    CHECK_EQ(to_snake_case("HTTPServer"), "http_server");
    CHECK_EQ(to_snake_case("parseURL"), "parse_url");
    CHECK_EQ(to_snake_case("already_snake"), "already_snake");
}

TEST_CASE("vague and Hungarian names are reported") {
    CHECK(is_vague_name("data"));
    CHECK(is_vague_name("x"));
    CHECK(is_vague_name("strName"));
    CHECK(is_vague_name("bDone"));
    CHECK(!is_vague_name("i"));
    CHECK(!is_vague_name("retry_count"));
    CHECK(!is_vague_name("string_builder"));
}

TEST_CASE("review_name reports errors before style advice") {
    const auto keyword = review_name("class", Kind::variable);
    CHECK_EQ(keyword.errors.size(), 1u);
    CHECK(keyword.warnings.empty());
    const auto camel = review_name("userAge", Kind::variable);
    REQUIRE_EQ(camel.warnings.size(), 1u);
    CHECK_EQ(camel.warnings[0], "use snake_case: user_age");
}

TEST_CASE("review_name applies the convention of each kind") {
    CHECK(review_name("max_retries", Kind::variable).clean());
    CHECK(review_name("BankAccount", Kind::type).clean());
    CHECK(review_name("MAX_USERS", Kind::constant).clean());
    CHECK(review_name("kMaxUsers", Kind::constant).clean());
    CHECK(!review_name("bank_account", Kind::type).clean());
    CHECK(!review_name("maxUsers", Kind::constant).clean());
}

TEST_CASE("run reviews each line and explains usage errors") {
    std::istringstream in("variable userAge\nfunction 9lives\nnonsense\ntype Parser\n\n");
    std::ostringstream out;
    CHECK_EQ(run(in, out), 0);
    const auto text = out.str();
    CHECK(text.find("use snake_case: user_age") != std::string::npos);
    CHECK(text.find("not a legal identifier") != std::string::npos);
    CHECK(text.find("usage:") != std::string::npos);
    CHECK(text.find("'Parser' follows the conventions") != std::string::npos);
}
