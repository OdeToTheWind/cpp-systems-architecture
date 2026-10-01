// Tests for Day 16 – Flowchart Programming.
#include <sstream>
#include <string>

#include "cppm/testing.hpp"
#include "day_16_flowchart_programming/lesson.hpp"

using namespace cppm::day16;

TEST_CASE("every flowchart exists and its decisions are countable") {
    CHECK_EQ(count_decisions(flowchart("baggage")), 4);
    CHECK_EQ(count_decisions(flowchart("boarding")), 4);
    CHECK_EQ(count_decisions(flowchart("queue")), 1);
    CHECK(flowchart("unknown").empty());
    CHECK(flowchart("queue").find("D --> B") != std::string_view::npos);  // the loop arrow
}

TEST_CASE("baggage_fee_cents follows every path of the baggage flowchart") {
    CHECK_EQ(baggage_fee_cents(33, Cabin::economy, false), -1);
    CHECK_EQ(baggage_fee_cents(33, Cabin::business, true), -1);
    CHECK_EQ(baggage_fee_cents(30, Cabin::business, false), 0);
    CHECK_EQ(baggage_fee_cents(25, Cabin::economy, false), 7'500);
    CHECK_EQ(baggage_fee_cents(25, Cabin::economy, true), 3'750);
    CHECK_EQ(baggage_fee_cents(10, Cabin::economy, false), 3'000);
    CHECK_EQ(baggage_fee_cents(10, Cabin::economy, true), 1'500);
}

TEST_CASE("baggage boundaries are exact") {
    CHECK_EQ(baggage_fee_cents(23, Cabin::economy, false), 3'000);
    CHECK_EQ(baggage_fee_cents(23.1, Cabin::economy, false), 7'500);
    CHECK_EQ(baggage_fee_cents(32, Cabin::economy, false), 7'500);
}

TEST_CASE("boarding_check reaches every terminal of its flowchart") {
    CHECK(boarding_check({"a"}) == Boarding::boarding_pass);
    CHECK(boarding_check({"b", false}) == Boarding::deny_passport);
    CHECK(boarding_check({"c", true, true, false}) == Boarding::deny_visa);
    CHECK(boarding_check({"d", true, true, true}) == Boarding::boarding_pass);
    CHECK(boarding_check({"e", true, false, false, 44}) == Boarding::see_desk);
    CHECK(boarding_check({"f", true, false, false, 45}) == Boarding::boarding_pass);
}

TEST_CASE("process_queue visits every passenger and handles an empty queue") {
    const auto summary =
        process_queue({{"Ada"}, {"Bo", false}, {"Cy", true, true, false}, {"Di", true, false, false, 30}, {"Ed"}});
    CHECK_EQ(summary.boarding_passes, 2);
    CHECK_EQ(summary.denied, 2);
    CHECK_EQ(summary.sent_to_desk, 1);
    CHECK(summary.desk_names == std::vector<std::string>{"Di"});
    const auto empty = process_queue({});
    CHECK_EQ(empty.boarding_passes + empty.denied + empty.sent_to_desk, 0);
}

TEST_CASE("run prices bags and summarises the queue") {
    std::istringstream in("25 e 1\n40 b 0\noops\n10 b 0\n\n");
    std::ostringstream out;
    CHECK_EQ(run(in, out), 0);
    const auto text = out.str();
    CHECK(text.find("flowchart TD") != std::string::npos);
    CHECK(text.find("fee 37.50") != std::string::npos);
    CHECK(text.find("refused: ship it as cargo") != std::string::npos);
    CHECK(text.find("fee 0.00") != std::string::npos);
    CHECK(text.find("e.g. 25 e 1") != std::string::npos);
    CHECK(text.find("Queue: 1 boarded, 2 denied, 1 sent to the desk") != std::string::npos);
}
