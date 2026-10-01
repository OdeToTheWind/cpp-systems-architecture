// Tests for Day 23 – Scope, Lifetime & Global Variables.
#include <sstream>
#include <string>
#include <vector>

#include "cppm/testing.hpp"
#include "day_23_scope_lifetime/lesson.hpp"

using namespace cppm::day23;

TEST_CASE("namespace constants describe the dispenser") {
    CHECK_EQ(shop::first_ticket, 1);
    CHECK_EQ(shop::last_ticket, 99);
    CHECK_EQ(std::string(shop::counter_name), "Deli");
}

TEST_CASE("the static counter keeps its value between calls") {
    const int before = tickets_issued();
    const int a = next_ticket();
    const int b = next_ticket();
    CHECK_EQ(b, a == shop::last_ticket ? shop::first_ticket : a + 1);
    CHECK_EQ(tickets_issued(), before + 2);
}

TEST_CASE("the counter wraps from 99 back to 1") {
    int previous = next_ticket();
    bool wrapped = false;
    for (int i = 0; i < 120; ++i) {
        const int current = next_ticket();
        CHECK(current >= shop::first_ticket && current <= shop::last_ticket);
        if (previous == shop::last_ticket) {
            CHECK_EQ(current, shop::first_ticket);
            wrapped = true;
        }
        previous = current;
    }
    CHECK(wrapped);
}

TEST_CASE("format_ticket pads to the two-digit display") {
    CHECK_EQ(format_ticket(7), "Deli #07");
    CHECK_EQ(format_ticket(42), "Deli #42");
}

TEST_CASE("an inner declaration hides the outer one only inside its block") {
    const auto [outer, inner] = shadowing_demo();
    CHECK_EQ(outer, 3);
    CHECK_EQ(inner, 10);
}

TEST_CASE("automatic, dynamic and static objects die at different times") {
    const auto first = lifetime_demo();
    const std::vector<std::string> expected_first{
        "construct automatic", "construct dynamic", "construct static (lives until exit)", "touch_static called",
        "touch_static called", "destroy dynamic",   "end of block",                        "destroy automatic"};
    CHECK(first == expected_first);
    const auto second = lifetime_demo();
    CHECK_EQ(second.size(), expected_first.size() - 1);  // the static is not constructed again
    CHECK_EQ(second[2], "touch_static called");
}

TEST_CASE("run issues tickets until q") {
    std::istringstream in("\n\nq\n\n");
    std::ostringstream out;
    CHECK_EQ(run(in, out), 0);
    const auto text = out.str();
    CHECK(text.find("inner block saw 10, outer still 3") != std::string::npos);
    CHECK(text.find("Deli #") != std::string::npos);
    CHECK(text.find("ticket(s) issued since start-up") != std::string::npos);
}
