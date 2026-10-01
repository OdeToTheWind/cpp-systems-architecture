// Tests for Day 01 – Variables, Types & Basic I/O.
#include <sstream>
#include <stdexcept>
#include <string>

#include "cppm/testing.hpp"
#include "day_01_variables/lesson.hpp"

using namespace cppm::day01;

TEST_CASE("type tour shows one initialised value per declaration") {
    const auto rows = type_tour();
    REQUIRE_EQ(rows.size(), 6u);
    CHECK_EQ(rows[0].second, "42");
    CHECK_EQ(rows[1].second, "12.50");
    CHECK_EQ(rows[3].second, "true");
    CHECK_EQ(rows[5].second, "1250000");
}

TEST_CASE("a default member card has every field initialised") {
    const MemberCard card;
    CHECK(card.name.empty());
    CHECK_EQ(card.age, 0);
    CHECK_EQ(card.tier, 'B');
    CHECK_EQ(card.newsletter, false);
}

TEST_CASE("parse_number accepts whole numbers and rejects trailing junk") {
    CHECK_EQ(parse_number<int>("42").value_or(-1), 42);
    CHECK_EQ(parse_number<int>("  7  ").value_or(-1), 7);
    CHECK_NEAR(parse_number<double>("1.75").value_or(0.0), 1.75, 1e-12);
    CHECK(!parse_number<int>("12abc").has_value());
    CHECK(!parse_number<int>("3 4").has_value());
    CHECK(!parse_number<int>("").has_value());
}

TEST_CASE("read_number asks again after invalid input and stops at end of input") {
    std::istringstream in("abc\n19\n");
    std::ostringstream out;
    CHECK_EQ(read_number<int>(in, out, "Age: ").value_or(-1), 19);
    CHECK(out.str().find("'abc' is not a valid number") != std::string::npos);
    CHECK(!read_number<int>(in, out, "Age: ").has_value());
}

TEST_CASE("read_member keeps spaces in names and parses every field") {
    std::istringstream in("\nAda Lovelace\n36\n1.65\np\nyes\n");
    std::ostringstream out;
    const auto card = read_member(in, out);
    REQUIRE(card.has_value());
    CHECK_EQ(card->name, "Ada Lovelace");
    CHECK_EQ(card->age, 36);
    CHECK_NEAR(card->height_m, 1.65, 1e-12);
    CHECK_EQ(card->tier, 'P');
    CHECK(card->newsletter);
    CHECK(out.str().find("A name is required") != std::string::npos);
}

TEST_CASE("format_card aligns labels and spells out the tier") {
    const MemberCard card{"Linus", 21, 1.8, 'B', false};
    const auto text = format_card(card);
    CHECK(text.find("Name        Linus") != std::string::npos);
    CHECK(text.find("Height      1.80 m") != std::string::npos);
    CHECK(text.find("Basic") != std::string::npos);
    CHECK(text.find("Newsletter  no") != std::string::npos);
}

TEST_CASE("calculate handles the four operators") {
    CHECK_NEAR(calculate(120, '/', 4), 30.0, 1e-12);
    CHECK_NEAR(calculate(2.5, '*', 4), 10.0, 1e-12);
    CHECK_NEAR(calculate(7, '-', 10), -3.0, 1e-12);
    CHECK_NEAR(calculate(0.1, '+', 0.2), 0.3, 1e-12);
}

TEST_CASE("calculate refuses division by zero and unknown operators") {
    CHECK_THROWS_AS(calculate(1, '/', 0), std::domain_error);
    CHECK_THROWS_AS(calculate(1, '%', 2), std::invalid_argument);
}

TEST_CASE("run drives the sign-up form and the calculator from scripted input") {
    std::istringstream in("Grace Hopper\n85\n1.6\nB\nn\n120 / 4\n1 / 0\nnonsense\n\n");
    std::ostringstream out;
    CHECK_EQ(run(in, out), 0);
    const auto text = out.str();
    CHECK(text.find("Grace Hopper") != std::string::npos);
    CHECK(text.find("= 30") != std::string::npos);
    CHECK(text.find("error: division by zero") != std::string::npos);
    CHECK(text.find("please type a number") != std::string::npos);
}

TEST_CASE("run stops politely when input ends during sign-up") {
    std::istringstream in("Barbara\n");
    std::ostringstream out;
    CHECK_EQ(run(in, out), 0);
    CHECK(out.str().find("Sign-up cancelled") != std::string::npos);
}
