// Tests for Day 05 – Mathematical Operations.
#include <limits>
#include <numeric>
#include <sstream>
#include <stdexcept>

#include "cppm/testing.hpp"
#include "day_05_math_operations/lesson.hpp"

using namespace cppm::day05;

TEST_CASE("precedence examples evaluate as C++ defines them") {
    const auto rows = precedence_examples();
    CHECK_EQ(rows[0].second, 14);
    CHECK_EQ(rows[1].second, 20);
    CHECK_EQ(rows[2].second, 3);
    CHECK_EQ(rows[3].second, 2);
    CHECK_EQ(rows[4].second, 2);
    CHECK_EQ(rows[5].second, -3);
}

TEST_CASE("integer division truncates toward zero; floor division rounds down") {
    const auto facts = division_facts(-7, 2);
    CHECK_EQ(facts.quotient, -3);
    CHECK_EQ(facts.remainder, -1);
    CHECK_EQ(facts.floor_quotient, -4);
    CHECK_NEAR(facts.real, -3.5, 1e-12);
    for (long long a : {7LL, -7LL}) {
        for (long long b : {2LL, -2LL}) {
            CHECK(division_facts(a, b).identity_holds);
        }
    }
}

TEST_CASE("division_facts rejects division by zero and the one overflowing case") {
    CHECK_THROWS_AS(division_facts(1, 0), std::domain_error);
    CHECK_THROWS_AS(division_facts(std::numeric_limits<long long>::min(), -1), std::overflow_error);
}

TEST_CASE("checked_multiply detects overflow in every sign combination") {
    const long long big = std::numeric_limits<long long>::max() / 2 + 1;
    CHECK_EQ(checked_multiply(6, 7).value_or(0), 42);
    CHECK_EQ(checked_multiply(-6, 7).value_or(0), -42);
    CHECK(!checked_multiply(big, 2).has_value());
    CHECK(!checked_multiply(-big, 3).has_value());
    CHECK(!checked_multiply(big, -3).has_value());
    CHECK(!checked_multiply(-big, -2).has_value());
    CHECK_EQ(checked_multiply(0, big).value_or(-1), 0);
}

TEST_CASE("the discount is subtracted before tax is added") {
    // 100.00 - 25.00 = 75.00, plus 10 % = 82.50 (the old bug added the discount: 137.50)
    CHECK_EQ(apply_discount_and_tax(10'000, 2'500, 10), 8'250);
    CHECK_EQ(apply_discount_and_tax(999, 0, 5), 1'049);  // 10.4895 rounds half up to 10.49
    CHECK_EQ(apply_discount_and_tax(500, 900, 20), 0);   // a discount bigger than the bill
    CHECK_THROWS_AS(apply_discount_and_tax(-1, 0, 0), std::invalid_argument);
}

TEST_CASE("split_bill shares always add up to the total") {
    const auto shares = split_bill(10'000, 3);
    CHECK_EQ(shares[0], 3'334);
    CHECK_EQ(shares[1], 3'333);
    CHECK_EQ(std::accumulate(shares.begin(), shares.end(), 0LL), 10'000);
    for (int people = 1; people <= 9; ++people) {
        const auto s = split_bill(12'345, people);
        CHECK_EQ(std::accumulate(s.begin(), s.end(), 0LL), 12'345);
    }
    CHECK_THROWS_AS(split_bill(100, 0), std::invalid_argument);
}

TEST_CASE("circle_stats uses the cmath functions correctly") {
    const auto stats = circle_stats(10.0);
    CHECK_NEAR(stats.area, 314.1592653589793, 1e-9);
    CHECK_NEAR(stats.circumference, 62.83185307179586, 1e-9);
    CHECK_NEAR(stats.tray_diagonal, 28.284271247461902, 1e-9);
    CHECK_THROWS_AS(circle_stats(0.0), std::domain_error);
    CHECK_THROWS_AS(circle_stats(std::numeric_limits<double>::quiet_NaN()), std::domain_error);
}

TEST_CASE("money formats cents with two digits") {
    CHECK_EQ(money(8'250), "82.50");
    CHECK_EQ(money(5), "0.05");
    CHECK_EQ(money(-120), "-1.20");
}

TEST_CASE("run splits a bill and re-asks after invalid numbers") {
    std::istringstream in("10000\n-5\n2500\n10\n3\n");
    std::ostringstream out;
    CHECK_EQ(run(in, out), 0);
    const auto text = out.str();
    CHECK(text.find("Total: 82.50") != std::string::npos);
    CHECK(text.find("diner 1 pays 27.50") != std::string::npos);
    CHECK(text.find("non-negative") != std::string::npos);
}

TEST_CASE("run reports an impossible split instead of crashing") {
    std::istringstream in("1000\n0\n0\n0\n");
    std::ostringstream out;
    run(in, out);
    CHECK(out.str().find("Cannot split this bill") != std::string::npos);
}
