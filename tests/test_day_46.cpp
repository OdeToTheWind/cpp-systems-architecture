// Tests for Day 46 – Variadic Templates.
#include <sstream>
#include <string>
#include <tuple>

#include "cppm/testing.hpp"
#include "day_46_variadic_templates/lesson.hpp"

using namespace cppm::day46;

TEST_CASE("sizeof... counts arguments at compile time") {
    static_assert(count_args() == 0);
    static_assert(count_args(1, "two", 3.0) == 3);
    CHECK_EQ(count_args('a', 'b'), 2u);
}

TEST_CASE("a fold expression adds any number of numbers") {
    static_assert(sum_all() == 0);
    static_assert(sum_all(1, 2, 3) == 6);
    CHECK_NEAR(sum_all(1, 2.5, 3.5f), 7.0, 1e-9);
    CHECK_EQ(sum_all(1'000'000'000LL, 2'000'000'000LL), 3'000'000'000LL);
}

TEST_CASE("format_fields pairs keys with values of any type") {
    CHECK_EQ(format_fields("player", "ada", "hp", 42), "player=ada hp=42");
    CHECK_EQ(format_fields("ok", true), "ok=1");
    CHECK_EQ(format_fields(), "");
}

TEST_CASE("log_line adds a field section only when fields are given") {
    CHECK_EQ(log_line("WARN", "lag spike"), "[WARN] lag spike");
    CHECK_EQ(log_line("INFO", "joined", "id", 7, "ping", 31.5), "[INFO] joined | id=7 ping=31.5");
}

TEST_CASE("perfect forwarding preserves lvalues and rvalues") {
    std::string name = "kept";
    const auto copied = make_tracked<Tracked>(name, 1);
    CHECK(!copied->moved_in);
    CHECK_EQ(name, "kept");
    const auto moved = make_tracked<Tracked>(std::string("temporary"), 1);
    CHECK(moved->moved_in);
    CHECK_EQ(moved->name, "temporary");
}

TEST_CASE("std::apply unpacks a tuple into function arguments") {
    CHECK_NEAR(distance_from_origin({3, 4, 0}), 5.0, 1e-12);
    CHECK_NEAR(distance_from_origin({1, 2, 2}), 3.0, 1e-12);
    CHECK_NEAR(std::apply([](auto... v) { return sum_all(v...); }, std::make_tuple(1, 2, 3)), 6.0, 1e-12);
}

TEST_CASE("several results come back as a tuple") {
    const auto [low, high, mean] = min_max_mean(4, -2, 10, 0);
    CHECK_NEAR(low, -2.0, 1e-12);
    CHECK_NEAR(high, 10.0, 1e-12);
    CHECK_NEAR(mean, 3.0, 1e-12);
    CHECK_NEAR(std::get<2>(min_max_mean(7)), 7.0, 1e-12);
}

TEST_CASE("run logs every player with structured fields") {
    std::istringstream in("3 4 0\n1 2 2\nstop\n");
    std::ostringstream out;
    CHECK_EQ(run(in, out), 0);
    const auto text = out.str();
    CHECK(text.find("[INFO] server started | port=7777 tick_ms=16 debug=0") != std::string::npos);
    CHECK(text.find("[INFO] player joined | id=1 distance=5") != std::string::npos);
    CHECK(text.find("id=2 distance=3") != std::string::npos);
    CHECK(text.find("min 3, max 9.5, mean 5.5") != std::string::npos);
    CHECK(text.find("2 player(s) joined; 3 arguments") != std::string::npos);
}
