// Tests for Day 08 – Conditional Statements.
#include <limits>
#include <sstream>

#include "cppm/testing.hpp"
#include "day_08_conditionals/lesson.hpp"

using namespace cppm::day08;

TEST_CASE("guard clauses report the first impossible value") {
    CHECK(!validate({10, -5, 1000, 0}).has_value());
    CHECK_EQ(validate({-1, 0, 100, 0}).value_or(""), "wind speed cannot be negative");
    CHECK_EQ(validate({1, 55, 100, 0}).value_or(""), "temperature outside the sensor's range");
    CHECK_EQ(validate({1, 0, 100, -3}).value_or(""), "new snow cannot be negative");
    CHECK(validate({std::numeric_limits<double>::quiet_NaN(), 0, 0, 0}).has_value());
}

TEST_CASE("classify_wind uses half-open ranges with exact boundaries") {
    CHECK(classify_wind(0) == Wind::calm);
    CHECK(classify_wind(19.99) == Wind::calm);
    CHECK(classify_wind(20) == Wind::breezy);
    CHECK(classify_wind(45) == Wind::strong);
    CHECK(classify_wind(70) == Wind::storm);
}

TEST_CASE("each lift type has its own wind tolerance") {
    const Reading breezy{30, -5, 2000, 0};
    CHECK(lift_decision(Lift::drag_lift, breezy).open);
    CHECK(lift_decision(Lift::chairlift, breezy).open);
    CHECK(!lift_decision(Lift::gondola, breezy).open);
    const Reading strong{60, -5, 2000, 0};
    CHECK(lift_decision(Lift::drag_lift, strong).open);
    CHECK_EQ(lift_decision(Lift::chairlift, strong).reason, "chairs swing in strong wind");
    CHECK(!lift_decision(Lift::drag_lift, {90, -5, 2000, 0}).open);
}

TEST_CASE("visibility, cold and invalid readings close lifts") {
    CHECK_EQ(lift_decision(Lift::drag_lift, {5, 0, 30, 0}).reason, "visibility below 50 m");
    CHECK_EQ(lift_decision(Lift::gondola, {5, -30, 2000, 0}).reason, "too cold for the cabin doors");
    CHECK(lift_decision(Lift::gondola, {5, -10, 2000, 0}).open);
    CHECK_EQ(lift_decision(Lift::chairlift, {-3, 0, 100, 0}).reason, "invalid reading: wind speed cannot be negative");
}

TEST_CASE("avalanche warning grows with snow and warmth") {
    CHECK_EQ(avalanche_warning({0, -5, 0, 5}), 1);
    CHECK_EQ(avalanche_warning({0, -5, 0, 20}), 2);
    CHECK_EQ(avalanche_warning({0, 2, 0, 20}), 3);
    CHECK_EQ(avalanche_warning({0, -5, 0, 40}), 4);
}

TEST_CASE("status_label and lift_name pick short display strings") {
    CHECK_EQ(status_label({true, "ok"}), "OPEN");
    CHECK_EQ(status_label({false, "x"}), "CLOSED");
    CHECK_EQ(lift_name(Lift::gondola), "gondola");
}

TEST_CASE("run prints a board per morning and rejects bad lines") {
    std::istringstream in("30 -5 2000 12\nwindy\n-4 0 100 0\n\n");
    std::ostringstream out;
    CHECK_EQ(run(in, out), 0);
    const auto text = out.str();
    CHECK(text.find("chairlift: OPEN") != std::string::npos);
    CHECK(text.find("gondola: CLOSED – gondola runs only in calm wind") != std::string::npos);
    CHECK(text.find("avalanche warning level 2") != std::string::npos);
    CHECK(text.find("please type four numbers") != std::string::npos);
    CHECK(text.find("rejected: wind speed cannot be negative") != std::string::npos);
}
