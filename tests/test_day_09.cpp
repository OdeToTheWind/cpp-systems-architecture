// Tests for Day 09 – Logical Operators.
#include <sstream>

#include "cppm/testing.hpp"
#include "day_09_logical_operators/lesson.hpp"

using namespace cppm::day09;

namespace {
const Door server_room{"server room", 3, false};
const Badge engineer{"Ada", 3, true, 100};
}  // namespace

TEST_CASE("an active, cleared badge opens the door during office hours") {
    CHECK(can_enter(&engineer, server_room, {10, 9, false, false}));
    CHECK(!can_enter(&engineer, server_room, {10, 22, false, false}));
    CHECK(!can_enter(&engineer, server_room, {10, 9, true, false}));
    CHECK(!can_enter(&engineer, server_room, {101, 9, false, false}));
}

TEST_CASE("an escort lets a low-clearance visitor through") {
    const Badge visitor{"Guest", 1, true, 365};
    CHECK(!can_enter(&visitor, server_room, {1, 10, false, false}));
    CHECK(can_enter(&visitor, server_room, {1, 10, false, true}));
}

TEST_CASE("short-circuit evaluation protects against a null badge") {
    CHECK(!badge_is_active(nullptr));
    CHECK(!can_enter(nullptr, server_room, {1, 10, false, true}));
    const Badge disabled{"Old", 4, false, 365};
    CHECK(!badge_is_active(&disabled));
}

TEST_CASE("denial_reasons lists every unmet condition") {
    const Badge visitor{"Guest", 1, true, 5};
    const auto reasons = denial_reasons(&visitor, server_room, {9, 23, true, false});
    REQUIRE_EQ(reasons.size(), 4u);
    CHECK_EQ(reasons[0], "building is in lockdown");
    CHECK_EQ(reasons[1], "badge expired");
    CHECK_EQ(reasons[2], "clearance too low and no escort");
    CHECK_EQ(reasons[3], "door closed after hours");
    CHECK(denial_reasons(&engineer, server_room, {1, 9, false, false}).empty());
}

TEST_CASE("&& stops at the first false operand and || at the first true one") {
    EvaluationTrace and_trace;
    CHECK(!(and_trace.check("a", false) && and_trace.check("b", true)));
    CHECK_EQ(and_trace.joined(), "a");
    EvaluationTrace or_trace;
    CHECK(or_trace.check("a", true) || or_trace.check("b", false));
    CHECK_EQ(or_trace.joined(), "a");
    EvaluationTrace full;
    CHECK(full.check("a", true) && full.check("b", true));
    CHECK_EQ(full.evaluated().size(), 2u);
}

TEST_CASE("De Morgan's laws hold and the truth table is complete") {
    CHECK(de_morgan_holds());
    const auto table = truth_table();
    CHECK_EQ(table[0], "F F | F F T");
    CHECK_EQ(table[3], "T T | T T F");
}

TEST_CASE("run evaluates each request against the server room") {
    std::istringstream in("3 10 0 0\n1 22 1 0\nhello\n\n");
    std::ostringstream out;
    CHECK_EQ(run(in, out), 0);
    const auto text = out.str();
    CHECK(text.find("evaluated [lockdown_clear]") != std::string::npos);
    CHECK(text.find("door opens") != std::string::npos);
    CHECK(text.find("denied: building is in lockdown") != std::string::npos);
    CHECK(text.find("denied: door closed after hours") != std::string::npos);
    CHECK(text.find("please type four numbers") != std::string::npos);
}
