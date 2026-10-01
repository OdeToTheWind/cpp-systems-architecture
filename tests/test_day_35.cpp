// Tests for Day 35 – Event Listeners & Callbacks.
#include <sstream>
#include <string>
#include <vector>

#include "cppm/testing.hpp"
#include "day_35_event_listeners/lesson.hpp"

using namespace cppm::day35;

TEST_CASE("listeners receive only the topics they subscribed to") {
    EventBus bus;
    std::vector<std::string> log;
    bus.subscribe("doorbell", [&](const Event& e) { log.push_back("door:" + e.detail); });
    bus.subscribe("motion", [&](const Event& e) { log.push_back("motion:" + e.detail); });
    CHECK_EQ(bus.publish({"doorbell", "front"}), 1);
    CHECK_EQ(bus.publish({"smoke", "kitchen"}), 0);
    CHECK(log == std::vector<std::string>{"door:front"});
}

TEST_CASE("wildcard listeners see every event, in subscription order") {
    EventBus bus;
    std::string order;
    bus.subscribe("*", [&](const Event&) { order += "A"; });
    bus.subscribe("motion", [&](const Event&) { order += "B"; });
    bus.subscribe("*", [&](const Event&) { order += "C"; });
    CHECK_EQ(bus.publish({"motion", "hall"}), 3);
    CHECK_EQ(order, "ABC");
}

TEST_CASE("unsubscribe by handle stops delivery") {
    EventBus bus;
    int calls = 0;
    const auto handle = bus.subscribe("motion", [&](const Event&) { ++calls; });
    bus.publish({"motion", "a"});
    CHECK(bus.unsubscribe(handle));
    CHECK(!bus.unsubscribe(handle));
    bus.publish({"motion", "b"});
    CHECK_EQ(calls, 1);
    CHECK_EQ(bus.listener_count(), 0u);
}

TEST_CASE("a listener may unsubscribe itself or others during delivery") {
    EventBus bus;
    std::string order;
    EventBus::Handle second = 0;
    EventBus::Handle self = 0;
    self = bus.subscribe("x", [&](const Event&) {
        order += "1";
        bus.unsubscribe(self);
        bus.unsubscribe(second);
    });
    second = bus.subscribe("x", [&](const Event&) { order += "2"; });
    bus.subscribe("x", [&](const Event&) { order += "3"; });
    CHECK_EQ(bus.publish({"x", ""}), 2);
    CHECK_EQ(order, "13");
    CHECK_EQ(bus.listener_count(), 1u);
}

TEST_CASE("listeners added during delivery start with the next event") {
    EventBus bus;
    int late_calls = 0;
    bus.subscribe("x", [&](const Event&) { bus.subscribe("x", [&](const Event&) { ++late_calls; }); });
    bus.publish({"x", ""});
    CHECK_EQ(late_calls, 0);
    bus.publish({"x", ""});
    CHECK_EQ(late_calls, 1);
}

TEST_CASE("a scoped subscription unsubscribes when it goes out of scope") {
    EventBus bus;
    int calls = 0;
    {
        ScopedSubscription subscription(bus, "smoke", [&](const Event&) { ++calls; });
        bus.publish({"smoke", "kitchen"});
        CHECK_EQ(bus.listener_count(), 1u);
    }
    bus.publish({"smoke", "kitchen"});
    CHECK_EQ(calls, 1);
    CHECK_EQ(bus.listener_count(), 0u);
}

TEST_CASE("run reacts to events and counts them per topic") {
    std::istringstream in("doorbell front door\nmotion garden\nmotion garden\nsmoke kitchen\n\n");
    std::ostringstream out;
    CHECK_EQ(run(in, out), 0);
    const auto text = out.str();
    CHECK(text.find("someone is at the front door") != std::string::npos);
    CHECK(text.find("garden sensor disabled") != std::string::npos);
    CHECK(text.find("calling the fire service (kitchen)") != std::string::npos);
    CHECK(text.find("motion: 2 event(s)") != std::string::npos);
    CHECK(text.find("-> 1 listener(s)") != std::string::npos);
}
