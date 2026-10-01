// Tests for Day 07 – Type Conversion & Casting.
#include <cstdint>
#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>

#include "cppm/testing.hpp"
#include "day_07_type_conversion/lesson.hpp"

using namespace cppm::day07;

TEST_CASE("small integers are promoted to int before arithmetic") {
    const auto report = promotion_report();
    REQUIRE_EQ(report.size(), 3u);
    CHECK_EQ(report[0], "uint8_t + uint8_t is int = 300");
    CHECK_EQ(report[1], "short * double is double");
    CHECK(report[2].find("true") != std::string::npos);
    CHECK(!(static_cast<unsigned>(-1) < 1u));  // what `-1 < 1u` really compares
    CHECK(std::cmp_less(-1, 1u));               // the value-preserving comparison
}

TEST_CASE("average_ticket divides in floating point") {
    CHECK_NEAR(average_ticket(1'000, 3), 3.3333333333, 1e-9);
    CHECK_NEAR(average_ticket(250, 2), 1.25, 1e-12);
    CHECK_THROWS_AS(average_ticket(100, 0), std::invalid_argument);
}

TEST_CASE("narrow accepts values that fit and rejects lossy conversions") {
    CHECK_EQ(narrow<std::int16_t>(32'767LL), 32'767);
    CHECK_EQ(narrow<std::int16_t>(-32'768LL), -32'768);
    CHECK_THROWS_AS(narrow<std::int16_t>(40'000LL), std::range_error);
    CHECK_THROWS_AS(narrow<unsigned>(-1), std::range_error);
    CHECK_THROWS_AS(narrow<int>(2.5), std::range_error);
    CHECK_EQ(narrow<int>(2.0), 2);
}

TEST_CASE("checksum_of calls the legacy API without modifying the string") {
    const std::string message = "PAY 1999";
    const std::string copy = message;
    const unsigned first = checksum_of(message);
    CHECK_EQ(checksum_of(message), first);
    CHECK_EQ(message, copy);
    CHECK(checksum_of("PAY 1999") != checksum_of("PAY 1998"));
}

TEST_CASE("wire_bytes exposes the object representation") {
    const auto bytes = wire_bytes(0x11223344u);
    if (is_little_endian()) {
        CHECK_EQ(+bytes[0], 0x44);
        CHECK_EQ(+bytes[3], 0x11);
    } else {
        CHECK_EQ(+bytes[0], 0x11);
        CHECK_EQ(+bytes[3], 0x44);
    }
}

TEST_CASE("dynamic_cast distinguishes payments from refunds") {
    const Payment payment(1'999);
    const Refund refund(500, "damaged");
    const Message& as_base = refund;
    CHECK_EQ(describe(payment), "payment of 1999 cents");
    CHECK_EQ(describe(as_base), "refund of 500 cents (damaged)");
    CHECK(dynamic_cast<const Payment*>(&as_base) == nullptr);
    CHECK_EQ(describe(Message(7)), "unknown message");
}

TEST_CASE("run narrows each amount and averages the accepted ones") {
    std::istringstream in("1999\n-500\n70000\nabc\n\n");
    std::ostringstream out;
    CHECK_EQ(run(in, out), 0);
    const auto text = out.str();
    CHECK(text.find("payment of 1999 cents") != std::string::npos);
    CHECK(text.find("refund of 500 cents") != std::string::npos);
    CHECK(text.find("70000 does not fit") != std::string::npos);
    CHECK(text.find("not a number") != std::string::npos);
    CHECK(text.find("Average ticket: 12.495") != std::string::npos);
}
