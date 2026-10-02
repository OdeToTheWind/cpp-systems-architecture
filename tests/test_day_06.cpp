// Tests for Day 06 – Data Types & Fixed-width Integers.
#include <cstdint>
#include <sstream>

#include "cppm/testing.hpp"
#include "day_06_data_types/lesson.hpp"

using namespace cppm::day06;

TEST_CASE("character types print their limits as numbers, not raw bytes") {
    const auto info = describe_type<unsigned char>("unsigned char");
    CHECK_EQ(info.min, "0");
    CHECK_EQ(info.max, "255");
    CHECK_EQ(describe_type<signed char>("signed char").min, "-128");
    CHECK(!info.is_signed);
}

TEST_CASE("the type table covers every modifier and the guaranteed sizes hold") {
    const auto table = type_table();
    CHECK_EQ(table.size(), 23u);
    for (const auto& info : table) {
        if (info.name == "char" || info.name == "std::int8_t" || info.name == "std::uint8_t") {
            CHECK_EQ(info.bytes, 1u);
        }
        if (info.name == "std::int64_t") {
            CHECK_EQ(info.bytes, 8u);
            CHECK_EQ(info.max, "9223372036854775807");
        }
    }
    CHECK(sizeof(short) <= sizeof(int));
    CHECK(sizeof(int) <= sizeof(long));
    CHECK(sizeof(long) <= sizeof(long long));
}

TEST_CASE("smallest_fitting_type picks the narrowest fixed-width type") {
    CHECK_EQ(smallest_fitting_type(0, 100), "std::uint8_t");
    CHECK_EQ(smallest_fitting_type(0, 255), "std::uint8_t");
    CHECK_EQ(smallest_fitting_type(0, 256), "std::uint16_t");
    CHECK_EQ(smallest_fitting_type(-40, 85), "std::int8_t");
    CHECK_EQ(smallest_fitting_type(-500, 500), "std::int16_t");
    CHECK_EQ(smallest_fitting_type(0, 5'000'000'000), "std::uint64_t");
    CHECK_EQ(smallest_fitting_type(-40, -10), "std::int8_t");
    CHECK_EQ(smallest_fitting_type(5, 1), "invalid range");
}

TEST_CASE("an unsigned 8-bit counter wraps around modulo 256") {
    CHECK_EQ(+counter_after(250, 10), 4);
    CHECK_EQ(+counter_after(255, 1), 0);
    CHECK_EQ(+counter_after(0, 256), 0);
}

TEST_CASE("floats cannot represent every integer above 2^24") {
    CHECK(!float_loses_integer(16'777'216));
    CHECK(float_loses_integer(16'777'217));
    CHECK(!float_loses_integer(-42));
}

TEST_CASE("memory_plan totals bytes per reading times readings") {
    std::size_t total = 0;
    const auto plan =
        memory_plan({{"temperature", -40, 85}, {"humidity", 0, 100}, {"pressure", 30'000, 110'000}}, total, 1000);
    REQUIRE_EQ(plan.size(), 3u);
    CHECK_EQ(plan[2].type, "std::uint32_t");
    CHECK_EQ(total, 6000u);
}

TEST_CASE("run prints the table and plans the typed fields") {
    std::istringstream in("temp -40 85\nbroken\nrain 0 500\n\n");
    std::ostringstream out;
    CHECK_EQ(run(in, out), 0);
    const auto text = out.str();
    CHECK(text.find("unsigned char       1      0 .. 255") != std::string::npos);
    CHECK(text.find("= 4 (wrapped)") != std::string::npos);
    CHECK(text.find("expected: name low high") != std::string::npos);
    CHECK(text.find("1000 readings need 3000 bytes") != std::string::npos);
}
