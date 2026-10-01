// Tests for Day 13 – For Loops.
#include <sstream>

#include "cppm/testing.hpp"
#include "day_13_for_loops/lesson.hpp"

using namespace cppm::day13;

namespace {
const std::vector<Runner> field{
    {"Ann", {3600, 3700, 3500}},  // 10800
    {"Ben", {3000, 3100, 0}},     // DNF (missed a mat)
    {"Cal", {3200, 3300, 3400}},  // 9900
    {"Dee", {3500, 3500}},        // DNF (too few splits)
    {"Eve", {3200, 3300, 3400}},  // 9900, ties with Cal
};
}  // namespace

TEST_CASE("total_seconds sums every split") {
    CHECK_EQ(total_seconds(field[0]), 10'800);
    CHECK_EQ(total_seconds(Runner{"empty", {}}), 0);
}

TEST_CASE("finished requires every checkpoint with a positive split") {
    CHECK(finished(field[0], 3));
    CHECK(!finished(field[1], 3));
    CHECK(!finished(field[3], 3));
}

TEST_CASE("rank_finishers skips DNFs and keeps ties in start order") {
    const auto ranking = rank_finishers(field, 3);
    REQUIRE_EQ(ranking.size(), 3u);
    CHECK_EQ(ranking[0].first, "Cal");
    CHECK_EQ(ranking[1].first, "Eve");
    CHECK_EQ(ranking[2].first, "Ann");
    CHECK(rank_finishers({}, 3).empty());
}

TEST_CASE("first_under stops at the first match in start order") {
    CHECK_EQ(first_under(field, 10'000, 3).value_or(""), "Cal");
    CHECK_EQ(first_under(field, 11'000, 3).value_or(""), "Ann");
    CHECK(!first_under(field, 5'000, 3).has_value());
}

TEST_CASE("pace_chart fills every row and column") {
    const auto chart = pace_chart({10, 42}, {240, 300, 360});
    REQUIRE_EQ(chart.size(), 2u);
    REQUIRE_EQ(chart[1].size(), 3u);
    CHECK_EQ(chart[0][0], 2'400);
    CHECK_EQ(chart[1][2], 15'120);
    CHECK(pace_chart({}, {300}).empty());
}

TEST_CASE("splits_in_reverse handles empty and non-empty runners") {
    CHECK(splits_in_reverse(Runner{"x", {}}).empty());
    CHECK(splits_in_reverse(Runner{"x", {1, 2, 3}}) == std::vector<int>{3, 2, 1});
}

TEST_CASE("format_time pads minutes and seconds") {
    CHECK_EQ(format_time(3'725), "1:02:05");
    CHECK_EQ(format_time(59), "0:00:59");
}

TEST_CASE("run prints the results board and the pace chart") {
    std::istringstream in("Ann 3600 3700 3500\nBen 3000 0 3000\nCal 3200 3300 3400\n\n");
    std::ostringstream out;
    CHECK_EQ(run(in, out), 0);
    const auto text = out.str();
    CHECK(text.find("1. Cal       2:45:00") != std::string::npos);
    CHECK(text.find("2. Ann       3:00:00") != std::string::npos);
    CHECK(text.find("Ben") == std::string::npos);
    CHECK(text.find("First sub-3-hour runner in start order: Cal") != std::string::npos);
    CHECK(text.find("4:12:00") != std::string::npos);
}
