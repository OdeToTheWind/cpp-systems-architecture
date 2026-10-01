// Tests for Day 18 – Positional and Named Arguments.
#include <sstream>
#include <stdexcept>

#include "cppm/testing.hpp"
#include "day_18_named_arguments/lesson.hpp"

using namespace cppm::day18;

TEST_CASE("base fares need a valid positional route") {
    CHECK_EQ(base_fare_cents("CDG", "FCO", 1'100), 11'900);
    CHECK_THROWS_AS(base_fare_cents("CDG", "CDG", 10), std::invalid_argument);
    CHECK_THROWS_AS(base_fare_cents("PARIS", "FCO", 10), std::invalid_argument);
    CHECK_THROWS_AS(base_fare_cents("CDG", "FCO", 0), std::invalid_argument);
}

TEST_CASE("default arguments fill in from the right") {
    CHECK_EQ(seat_fee_cents(), 0);
    CHECK_EQ(seat_fee_cents(Cabin::premium), 1'500);
    CHECK_EQ(seat_fee_cents(Cabin::economy, true), 2'500);
    CHECK_EQ(seat_fee_cents(Cabin::economy, true, true), 3'300);
    CHECK_EQ(seat_fee_cents(Cabin::business, true, true), 0);
}

TEST_CASE("make_date overloads accept integers or text and validate both") {
    const Date a = make_date(2026, 7, 14);
    const Date b = make_date("2026-07-14");
    CHECK_EQ(a.month, b.month);
    CHECK_EQ(a.day, b.day);
    CHECK_THROWS_AS(make_date(2026, 13, 1), std::invalid_argument);
    CHECK_THROWS_AS(make_date("14/07/2026"), std::invalid_argument);
    CHECK_THROWS_AS(make_date("2026-02-40"), std::invalid_argument);
}

TEST_CASE("designated initialisers name only the options that change") {
    const Booking plain = book_flight("CDG", "FCO", 1'100);
    CHECK_EQ(plain.total_cents, 11'900);
    CHECK_EQ(plain.summary, "0 bag(s)");
    const Booking premium = book_flight("CDG", "FCO", 1'100, {.cabin = Cabin::premium, .bags = 2, .meal = "vegan"});
    CHECK_EQ(premium.total_cents, 11'900 * 3 / 2 + 1'500 + 7'000);
    CHECK_EQ(premium.summary, "2 bag(s), meal: vegan");
}

TEST_CASE("book_flight validates the options and applies the flexible surcharge") {
    CHECK_THROWS_AS(book_flight("CDG", "FCO", 1'100, {.bags = 4}), std::invalid_argument);
    const Booking flexible = book_flight("CDG", "FCO", 1'100, {.flexible = true});
    CHECK_EQ(flexible.total_cents, 11'900 + 11'900 / 5);
    CHECK_EQ(flexible.route, "CDG-FCO");
}

TEST_CASE("the builder chains named setters to the same result") {
    const Booking built = BookingBuilder("LHR", "JFK", 5'540).cabin(Cabin::premium).bags(1).window().book();
    const Booking direct =
        book_flight("LHR", "JFK", 5'540, {.cabin = Cabin::premium, .bags = 1, .window = true});
    CHECK_EQ(built.total_cents, direct.total_cents);
    CHECK_EQ(BookingBuilder("AMS", "OSL", 900).meal("fish").flexible().book().summary, "0 bag(s), meal: fish, flexible");
}

TEST_CASE("run books flights and explains bad input") {
    std::istringstream in("CDG FCO 1100 1 flex\nCDG CDG 5\nnonsense\n\n");
    std::ostringstream out;
    CHECK_EQ(run(in, out), 0);
    const auto text = out.str();
    CHECK(text.find("Sample: LHR-JFK") != std::string::npos);
    CHECK(text.find("CDG-FCO: 18480 cents (1 bag(s), flexible)") != std::string::npos);
    CHECK(text.find("two different three-letter") != std::string::npos);
    CHECK(text.find("e.g. CDG FCO") != std::string::npos);
}
