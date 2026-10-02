// Tests for Day 49 – Advanced Error Handling.
#include <sstream>
#include <stdexcept>
#include <string>
#include <system_error>

#include "cppm/testing.hpp"
#include "day_49_advanced_errors/lesson.hpp"

using namespace cppm::day49;

namespace {
const std::string good_card = "4111111111111111";
}

TEST_CASE("parse_seat returns values or specific error codes") {
    const auto seat = parse_seat("c12");
    REQUIRE(seat.ok());
    CHECK_EQ(seat.value().code(), "C12");
    CHECK(parse_seat("").error() == seat_errc::empty);
    CHECK(parse_seat("12").error() == seat_errc::bad_row);
    CHECK(parse_seat("Cx").error() == seat_errc::bad_number);
    CHECK(parse_seat("C").error() == seat_errc::bad_number);
    CHECK(parse_seat("C31").error() == seat_errc::out_of_range);
}

TEST_CASE("the custom error category names and describes its codes") {
    const std::error_code code = seat_errc::out_of_range;
    CHECK_EQ(std::string(code.category().name()), "seat");
    CHECK_EQ(code.message(), "seat number must be 1-30");
    CHECK(code != std::make_error_code(std::errc::invalid_argument));
    CHECK(!std::error_code{});
}

TEST_CASE("Result::value throws std::system_error when there is no value") {
    const Result<Seat> bad = seat_errc::bad_row;
    CHECK(!bad);
    CHECK_THROWS_AS(bad.value(), std::system_error);
    const Result<Seat> good = Seat{'A', 1};
    CHECK(!good.error());
}

TEST_CASE("booking succeeds and refuses a seat that is taken") {
    BoxOffice office(10);
    CHECK_EQ(office.book("a1", "Ada", good_card), "A1");
    try {
        office.book("A1", "Bo", good_card);
        CHECK(false);
    } catch (const SeatTaken& error) {
        CHECK_EQ(error.seat(), "A1");
    }
}

TEST_CASE("every business failure can be caught as BookingError") {
    BoxOffice office(1);
    office.book("B2", "Ada", good_card);
    CHECK_THROWS_AS(office.book("B3", "Bo", good_card), SoldOut);
    CHECK_THROWS_AS(office.book("B3", "Bo", good_card), BookingError);
    BoxOffice other(5);
    CHECK_THROWS_AS(other.book("B3", "Bo", "123"), PaymentDeclined);
    CHECK_THROWS_AS(other.book("B3", "Bo", "4111111111111110"), std::runtime_error);
}

TEST_CASE("a bad seat code becomes a system_error carrying the parser's code") {
    BoxOffice office(5);
    try {
        office.book("#!", "Ada", good_card);
        CHECK(false);
    } catch (const std::system_error& error) {
        CHECK(error.code() == seat_errc::bad_row);
        CHECK(std::string(error.what()).find("cannot book '#!'") != std::string::npos);
    }
    CHECK_EQ(office.sold(), 0u);
}

TEST_CASE("run reports each kind of failure differently") {
    std::istringstream in("A1 Ada " + good_card + "\nA1 Bo " + good_card + "\nZ99 Cy " + good_card +
                          "\nB1 Di 1234\nB2 Ed " + good_card + "\nB3 Fi " + good_card + "\nB4 Gu " + good_card +
                          "\nstop\n");
    std::ostringstream out;
    CHECK_EQ(run(in, out), 0);
    const auto text = out.str();
    CHECK(text.find("booked A1 for Ada") != std::string::npos);
    CHECK(text.find("sorry, A1 is taken") != std::string::npos);
    CHECK(text.find("seat error 4: seat number must be 1-30") != std::string::npos);
    CHECK(text.find("booking failed: payment declined") != std::string::npos);
    CHECK(text.find("booking failed: the concert is sold out") != std::string::npos);
    CHECK(text.find("3 ticket(s) sold") != std::string::npos);
}
