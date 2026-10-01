// Tests for Day 54 – Date and Time with chrono.
#include <chrono>
#include <set>
#include <sstream>
#include <stdexcept>

#include "cppm/testing.hpp"
#include "day_54_date_time/lesson.hpp"

using namespace cppm::day54;
using namespace std::chrono_literals;
namespace chr = std::chrono;

namespace {
const std::set<chr::sys_days> no_holidays;
}

TEST_CASE("parse_date validates month lengths and leap years") {
    CHECK(parse_date("2026-05-29").has_value());
    CHECK(parse_date("2024-02-29").has_value());
    CHECK(!parse_date("2026-02-29").has_value());
    CHECK(!parse_date("2026-13-01").has_value());
    CHECK(!parse_date("29/05/2026").has_value());
    CHECK(!parse_date("2026-05-29x").has_value());
    CHECK_EQ(format_date(*parse_date("2026-5-9")), "2026-05-09");
}

TEST_CASE("weekday names come from the calendar") {
    CHECK_EQ(weekday_name(2026y / chr::May / 29), "Friday");
    CHECK_EQ(weekday_name(2026y / chr::May / 31), "Sunday");
    CHECK_EQ(weekday_name(2000y / chr::January / 1), "Saturday");
}

TEST_CASE("business days skip weekends") {
    CHECK(add_business_days(2026y / chr::May / 29, 1, no_holidays) == 2026y / chr::June / 1);  // Fri -> Mon
    CHECK(add_business_days(2026y / chr::May / 30, 0, no_holidays) == 2026y / chr::June / 1);  // Sat rolls forward
    CHECK(add_business_days(2026y / chr::May / 27, 0, no_holidays) == 2026y / chr::May / 27);
    CHECK(add_business_days(2026y / chr::May / 25, 10, no_holidays) == 2026y / chr::June / 8);
    CHECK_THROWS_AS(add_business_days(2026y / chr::May / 25, -1, no_holidays), std::invalid_argument);
}

TEST_CASE("holidays are skipped like weekends") {
    const std::set<chr::sys_days> holidays{chr::sys_days{2026y / chr::June / 1}};
    CHECK(add_business_days(2026y / chr::May / 29, 1, holidays) == 2026y / chr::June / 2);
}

TEST_CASE("durations are split into days, hours and minutes") {
    CHECK_EQ(format_duration(chr::minutes{0}), "0d 00h 00m");
    CHECK_EQ(format_duration(26h + 5min), "1d 02h 05m");
    CHECK_EQ(format_duration(chr::duration_cast<chr::minutes>(chr::days{3})), "3d 00h 00m");
    CHECK_THROWS_AS(format_duration(chr::minutes{-1}), std::invalid_argument);
}

TEST_CASE("UTC offsets can move the time across midnight") {
    const auto utc = at(2026y / chr::May / 29, 22, 15);
    CHECK_EQ(local_time(utc, 0min), "2026-05-29 22:15");
    CHECK_EQ(local_time(utc, 5h + 30min), "2026-05-30 03:45");
    CHECK_EQ(local_time(utc, -10h), "2026-05-29 12:15");
    CHECK_EQ(local_time(at(2026y / chr::May / 29, 1, 0), -4h), "2026-05-28 21:00");
}

TEST_CASE("orders after the 15:00 cut-off ship the next business day") {
    CHECK(ship_date(at(2026y / chr::May / 27, 14, 59), no_holidays) == 2026y / chr::May / 27);
    CHECK(ship_date(at(2026y / chr::May / 27, 15, 0), no_holidays) == 2026y / chr::May / 28);
    CHECK(ship_date(at(2026y / chr::May / 29, 16, 0), no_holidays) == 2026y / chr::June / 1);
}

TEST_CASE("run estimates delivery across a holiday and shows local times") {
    std::istringstream in("2026-05-22 16:30 2\n2026-02-30 10:00 1\nstop\n");
    std::ostringstream out;
    CHECK_EQ(run(in, out), 0);
    const auto text = out.str();
    CHECK(text.find("ships 2026-05-26 (Tuesday), arrives 2026-05-28 (Thursday)") != std::string::npos);
    CHECK(text.find("New York 2026-05-28 05:00, Mumbai 2026-05-28 14:30, Tokyo 2026-05-28 18:00") != std::string::npos);
    CHECK(text.find("door to door: 5d 16h 30m") != std::string::npos);
    CHECK(text.find("invalid order") != std::string::npos);
}
