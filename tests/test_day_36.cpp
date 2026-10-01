// Tests for Day 36 – Instances and State.
#include <sstream>
#include <stdexcept>

#include "cppm/testing.hpp"
#include "day_36_instances_state/lesson.hpp"

using namespace cppm::day36;

TEST_CASE("the transition table allows only the documented moves") {
    CHECK(allowed(State::registered, State::in_transit));
    CHECK(allowed(State::in_locker, State::collected));
    CHECK(!allowed(State::registered, State::collected));
    CHECK(!allowed(State::collected, State::returned));
    CHECK(!allowed(State::in_locker, State::in_transit));
}

TEST_CASE("a parcel walks the happy path and records its history") {
    Parcel parcel("P1", "4711");
    parcel.advance(State::in_transit, 1);
    parcel.advance(State::in_locker, 2);
    CHECK(parcel.collect("4711", 3));
    CHECK(parcel.state() == State::collected);
    CHECK(parcel.finished());
    REQUIRE_EQ(parcel.history().size(), 4u);
    CHECK_EQ(parcel.history()[2].first, 2);
    CHECK(parcel.history()[3].second == State::collected);
}

TEST_CASE("illegal transitions throw and leave the state unchanged") {
    Parcel parcel("P2", "0000");
    CHECK_THROWS_AS(parcel.advance(State::collected, 1), std::logic_error);
    CHECK(parcel.state() == State::registered);
    parcel.advance(State::in_transit, 5);
    CHECK_THROWS_AS(parcel.advance(State::in_locker, 4), std::logic_error);
    CHECK_EQ(parcel.history().size(), 2u);
}

TEST_CASE("collect depends on the state and on the parcel's own code") {
    Parcel parcel("P3", "1234");
    CHECK(!parcel.collect("1234", 1));  // not in a locker yet
    parcel.advance(State::in_transit, 1);
    parcel.advance(State::in_locker, 2);
    CHECK(!parcel.collect("9999", 2));
    CHECK(parcel.state() == State::in_locker);
}

TEST_CASE("uncollected parcels expire after three days") {
    Parcel parcel("P4", "1");
    parcel.advance(State::in_transit, 0);
    parcel.advance(State::in_locker, 1);
    CHECK(!parcel.expire_if_uncollected(4));
    CHECK(parcel.expire_if_uncollected(5));
    CHECK(parcel.state() == State::returned);
}

TEST_CASE("instances in a network keep independent state") {
    LockerNetwork network;
    network.add("A", "1").advance(State::in_transit, 0);
    network.add("B", "2");
    network.at("A").advance(State::in_locker, 1);
    CHECK(network.at("B").state() == State::registered);
    CHECK_THROWS_AS(network.add("A", "3"), std::invalid_argument);
    CHECK_THROWS_AS(network.at("Z"), std::out_of_range);
    CHECK(network.nightly_expiry(10) == std::vector<std::string>{"A"});
    const auto census = network.census();
    CHECK_EQ(census.at(State::returned), 1);
    CHECK_EQ(census.at(State::registered), 1);
}

TEST_CASE("run drives several parcels through their lifecycles") {
    std::istringstream in("add P1 11\nadd P2 22\nship P1 1\ndeliver P1 2\ncollect P1 99 2\ncollect P1 11 3\n"
                          "ship P2 1\ndeliver P2 1\nnight 9\ncollect P9 1 1\nship P1 4\nfly\n\n");
    std::ostringstream out;
    CHECK_EQ(run(in, out), 0);
    const auto text = out.str();
    CHECK(text.find("wrong code or not in a locker") != std::string::npos);
    CHECK(text.find("P1 is collected") != std::string::npos);
    CHECK(text.find("P2 returned to sender") != std::string::npos);
    CHECK(text.find("no parcel P9") != std::string::npos);
    CHECK(text.find("cannot go from collected to in transit") != std::string::npos);
    CHECK(text.find("collected: 1") != std::string::npos);
    CHECK(text.find("returned: 1") != std::string::npos);
}
