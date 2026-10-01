// Tests for Day 28 – Creating Classes.
#include <sstream>
#include <stdexcept>
#include <string>
#include <type_traits>

#include "cppm/testing.hpp"
#include "day_28_classes/lesson.hpp"

using namespace cppm::day28;

TEST_CASE("TimeSlot validates and prints with leading zeros") {
    std::ostringstream text;
    text << TimeSlot(7 * 60 + 5);
    CHECK_EQ(text.str(), "07:05");
    CHECK_THROWS_AS(TimeSlot(-1), std::out_of_range);
    CHECK_THROWS_AS(TimeSlot(24 * 60), std::out_of_range);
    CHECK(TimeSlot(60) == TimeSlot(60));
}

TEST_CASE("explicit constructors forbid silent conversions") {
    static_assert(!std::is_convertible_v<int, TimeSlot>);
    static_assert(std::is_constructible_v<TimeSlot, int>);
    const TimeSlot slot(90);  // direct initialisation is still allowed
    CHECK_EQ(slot.hour(), 1);
    CHECK_EQ(slot.minute(), 30);
}

TEST_CASE("the constructor rejects objects that would break the invariant") {
    CHECK_THROWS_AS(FitnessClass("Yoga", TimeSlot(600), 0), std::invalid_argument);
    CHECK_THROWS_AS(FitnessClass("", TimeSlot(600), 5), std::invalid_argument);
    const FitnessClass yoga("Yoga", TimeSlot(600), 5);
    CHECK_EQ(yoga.places_left(), 5);
    CHECK(yoga.invariant_holds());
}

TEST_CASE("book fills places, then the waiting list, never double-books") {
    FitnessClass spin("Spin", TimeSlot(18 * 60), 2);
    CHECK(spin.book("Ada") == BookingResult::booked);
    CHECK(spin.book("Ada") == BookingResult::already_booked);
    CHECK(spin.book("Bo") == BookingResult::booked);
    CHECK(spin.is_full());
    CHECK(spin.book("Cy") == BookingResult::waitlisted);
    CHECK(spin.book("Cy") == BookingResult::already_booked);
    CHECK(spin.invariant_holds());
}

TEST_CASE("cancel promotes the first person on the waiting list") {
    FitnessClass spin("Spin", TimeSlot(18 * 60), 1);
    spin.book("Ada");
    spin.book("Bo");
    spin.book("Cy");
    CHECK_EQ(spin.cancel("Ada").value_or(""), "Bo");
    CHECK(spin.attendees() == std::vector<std::string>{"Bo"});
    CHECK(!spin.cancel("Cy").has_value());  // leaving the waiting list frees no place
    CHECK(spin.waitlist().empty());
    CHECK_THROWS_AS(spin.cancel("Zed"), std::invalid_argument);
    CHECK(spin.invariant_holds());
}

TEST_CASE("operator<< summarises the class") {
    FitnessClass spin("Spin", TimeSlot(18 * 60), 3);
    spin.book("Ada");
    std::ostringstream text;
    text << spin;
    CHECK_EQ(text.str(), "Spin 18:00 (1/3 booked, 0 waiting)");
}

TEST_CASE("run books and cancels through the public interface only") {
    std::istringstream in("book Ada\nbook Bo\nbook Cy\nbook Di\nbook Ada\ncancel Bo\ncancel Zed\nfly Ada\n\n");
    std::ostringstream out;
    CHECK_EQ(run(in, out), 0);
    const auto text = out.str();
    CHECK(text.find("class full – waiting list") != std::string::npos);
    CHECK(text.find("already booked") != std::string::npos);
    CHECK(text.find("cancelled, Di gets the place") != std::string::npos);
    CHECK(text.find("Zed is not booked on Spin") != std::string::npos);
    CHECK(text.find("Spin 18:00 (3/3 booked, 0 waiting)") != std::string::npos);
}
