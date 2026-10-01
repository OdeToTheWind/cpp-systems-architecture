// Tests for Day 24 – Debugging Techniques.
#include <sstream>
#include <stdexcept>
#include <string>

#include "cppm/testing.hpp"
#include "day_24_debugging/lesson.hpp"

using namespace cppm::day24;

TEST_CASE("the minimal reproduction shows the shipped bug") {
    CHECK_EQ(reproduce(3), "input days_late=3 expected=0 actual=75 MISMATCH");
    CHECK_EQ(reproduce(2), "input days_late=2 expected=0 actual=0 OK");
}

TEST_CASE("the fixed fee respects the grace period and the cap") {
    CHECK_EQ(late_fee(0), 0);
    CHECK_EQ(late_fee(3), 0);
    CHECK_EQ(late_fee(4), 25);
    CHECK_EQ(late_fee(10), 175);
    CHECK_EQ(late_fee(43), 1'000);
    CHECK_EQ(late_fee(400), 1'000);
}

TEST_CASE("contract checks reject impossible input instead of computing garbage") {
    CHECK_THROWS_AS(late_fee(-1), std::logic_error);
    CHECK_THROWS_AS(expects(false, "x"), std::logic_error);
    CHECK_NOTHROW(expects(true, "x"));
}

TEST_CASE("the tracer writes only when enabled") {
    std::ostringstream sink;
    late_fee(10, Tracer(true, sink));
    CHECK(sink.str().find("[trace] late_fee: chargeable_days = 7") != std::string::npos);
    CHECK(sink.str().find("uncapped = 175") != std::string::npos);
    std::ostringstream silent;
    late_fee(10, Tracer(false, silent));
    CHECK(silent.str().empty());
}

TEST_CASE("bisecting finds the first failing day in a handful of checks") {
    int checks = 0;
    const auto first = first_failing_day([](int day) { return day >= 137; }, 0, 1'000, &checks);
    CHECK_EQ(first.value_or(-1), 137);
    CHECK(checks <= 11);
    CHECK(!first_failing_day([](int) { return false; }, 0, 100).has_value());
    CHECK_EQ(first_failing_day([](int) { return true; }, 5, 100).value_or(-1), 5);
}

TEST_CASE("bisecting the real bug points at the grace-day boundary") {
    const auto first = first_failing_day([](int day) { return late_fee(day) != late_fee_buggy(day); }, 0, 40);
    CHECK_EQ(first.value_or(-1), 3);
}

TEST_CASE("run reports the boundary, fees and traces") {
    std::istringstream in("10 trace\n-2\nmany\n\n");
    std::ostringstream out;
    CHECK_EQ(run(in, out), 0);
    const auto text = out.str();
    CHECK(text.find("first mismatch at day") != std::string::npos);
    CHECK(text.find("[trace] late_fee: chargeable_days = 7") != std::string::npos);
    CHECK(text.find("fee 175 cents") != std::string::npos);
    CHECK(text.find("contract violated: days_late >= 0") != std::string::npos);
    CHECK(text.find("type a number of days") != std::string::npos);
}
