// Tests for Day 03 – Input & Output Streams.
#include <sstream>
#include <string>

#include "cppm/testing.hpp"
#include "day_03_input_output/lesson.hpp"

using namespace cppm::day03;

TEST_CASE("ask_int accepts a valid number on the first try") {
    std::istringstream in("42\n");
    std::ostringstream out;
    CHECK_EQ(ask_int(in, out, "Age: ", 5, 120).value_or(-1), 42);
}

TEST_CASE("ask_int recovers from text and re-asks for out-of-range values") {
    std::istringstream in("abc\n200\n30\n");
    std::ostringstream out;
    CHECK_EQ(ask_int(in, out, "Age: ", 5, 120).value_or(-1), 30);
    CHECK(out.str().find("not a whole number") != std::string::npos);
    CHECK(out.str().find("from 5 to 120") != std::string::npos);
}

TEST_CASE("ask_int returns nothing at end of input instead of looping forever") {
    std::istringstream in("oops");
    std::ostringstream out;
    CHECK(!ask_int(in, out, "Age: ", 0, 10).has_value());
}

TEST_CASE("recover_stream clears the fail state and the bad line") {
    std::istringstream in("xyz rest of line\n7\n");
    int value{};
    CHECK(!(in >> value));
    CHECK(recover_stream(in));
    CHECK(static_cast<bool>(in >> value));
    CHECK_EQ(value, 7);
}

TEST_CASE("ask_choice is case-insensitive and rejects other answers") {
    std::istringstream in("x\nvip\nv\n");
    std::ostringstream out;
    CHECK_EQ(ask_choice(in, out, "? ", "SVT").value_or('?'), 'V');
    CHECK(out.str().find("choose one of: SVT") != std::string::npos);
}

TEST_CASE("price_cents uses exact integer cents") {
    CHECK_EQ(price_cents({"a", 20, 'S', 1}), 8000);
    CHECK_EQ(price_cents({"a", 20, 'V', 3}), 15000);
    CHECK_EQ(price_cents({"a", 20, 'T', 2}), 6050);
}

TEST_CASE("format_receipt aligns labels and prints two decimals") {
    const auto text = format_receipt({"Ada", 36, 'T', 1});
    CHECK(text.find("Attendee    Ada") != std::string::npos);
    CHECK(text.find("Ticket      Student") != std::string::npos);
    CHECK(text.find("Total                 45.50 EUR") != std::string::npos);
}

TEST_CASE("run walks through a full registration with corrections") {
    std::istringstream in("Grace Hopper\nold\n85\nq\nV\n2\n");
    std::ostringstream out;
    CHECK_EQ(run(in, out), 0);
    CHECK(out.str().find("135.00 EUR") != std::string::npos);
}

TEST_CASE("run reports an incomplete registration at end of input") {
    std::istringstream in("Linus\n21\n");
    std::ostringstream out;
    run(in, out);
    CHECK(out.str().find("Registration incomplete") != std::string::npos);
}
