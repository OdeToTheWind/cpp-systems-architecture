// Tests for Day 69 – Operator Overloading. Many checks run at compile time: the operators are constexpr.
#include <compare>
#include <sstream>
#include <stdexcept>
#include <string>

#include "cppm/testing.hpp"
#include "day_69_operator_overloading/lesson.hpp"

using namespace cppm::day69;

namespace {
std::string show(const Rational& r) {
    std::ostringstream out;
    out << r;
    return out.str();
}
Rational parse(const std::string& text) {
    std::istringstream in(text);
    Rational r(99);
    in >> r;
    if (in.fail()) throw std::runtime_error("parse failed: " + text);
    return r;
}
}  // namespace

TEST_CASE("the invariant keeps fractions reduced with a positive denominator") {
    constexpr Rational r(6, -8);
    static_assert(r.num() == -3 && r.den() == 4);
    CHECK_EQ(Rational(0, 5).den(), 1);
    CHECK_THROWS_AS(Rational(1, 0), std::invalid_argument);
    CHECK(Rational(2, 4) == Rational(1, 2));  // defaulted == works because members are canonical
}

TEST_CASE("arithmetic is exact and mixes with integers") {
    static_assert(Rational(1, 3) + Rational(1, 6) == Rational(1, 2));
    static_assert(2 * Rational(3, 4) == Rational(3, 2));
    static_assert(Rational(3, 4) - 1 == Rational(-1, 4));
    static_assert(Rational(3, 4) / Rational(3, 8) == 2);
    CHECK_THROWS_AS(Rational(1, 2) / 0, std::domain_error);
    Rational total;
    for (int i = 0; i < 10; ++i) total += Rational(1, 10);
    CHECK(total == 1);  // 0.1 added ten times is exactly one, unlike with doubles
}

TEST_CASE("the spaceship operator gives all six comparisons") {
    static_assert(Rational(1, 3) < Rational(1, 2));
    static_assert(Rational(-1, 2) < Rational(1, 3));
    static_assert((Rational(2, 3) <=> Rational(4, 6)) == std::strong_ordering::equal);
    CHECK(Rational(5, 4) >= 1);
    CHECK(Rational(7, 8) != Rational(8, 9));
}

TEST_CASE("streams write mixed numbers") {
    CHECK_EQ(show(Rational(3, 2)), "1 1/2");
    CHECK_EQ(show(Rational(-7, 4)), "-1 3/4");
    CHECK_EQ(show(Rational(-1, 3)), "-1/3");
    CHECK_EQ(show(Rational(4, 2)), "2");
    CHECK_NEAR(static_cast<double>(Rational(1, 8)), 0.125, 1e-12);
}

TEST_CASE("streams read whole numbers, fractions and mixed numbers") {
    CHECK(parse("3/4") == Rational(3, 4));
    CHECK(parse("1 1/2") == Rational(3, 2));
    CHECK(parse("-1 3/4") == Rational(-7, 4));
    CHECK(parse("2") == 2);
    std::istringstream in("2 cups");
    Rational r;
    std::string unit;
    in >> r >> unit;
    CHECK(r == 2);
    CHECK_EQ(unit, "cups");  // the look-ahead was rolled back
}

TEST_CASE("bad input fails the stream and leaves the value alone") {
    std::istringstream bad("3/0");
    Rational r(5);
    bad >> r;
    CHECK(bad.fail());
    CHECK(r == 5);
    CHECK_THROWS_AS(parse("cups"), std::runtime_error);
}

TEST_CASE("run scales the recipe exactly") {
    std::istringstream in("2/3\n0\n1 1/2\nend\n");
    std::ostringstream out;
    CHECK_EQ(run(in, out), 0);
    const auto text = out.str();
    CHECK(text.find("1 cups flour") != std::string::npos);
    CHECK(text.find("1/2 cup butter") != std::string::npos);
    CHECK(text.find("2/9 cup sugar") != std::string::npos);
    CHECK(text.find("the factor must be positive") != std::string::npos);
    CHECK(text.find("2 1/4 cups flour") != std::string::npos);
}
