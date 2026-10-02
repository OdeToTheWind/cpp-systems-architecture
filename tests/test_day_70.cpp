// Tests for Day 70 – Compile-time Programming. static_asserts are tests that run inside the compiler.
#include <array>
#include <cstdint>
#include <sstream>
#include <string>
#include <string_view>
#include <vector>

#include "cppm/testing.hpp"
#include "day_70_compile_time/lesson.hpp"

using namespace cppm::day70;

namespace {
template <typename T>
concept Serialisable = requires(std::vector<std::uint8_t>& out, const T& v) { serialise(out, v); };
}  // namespace

TEST_CASE("the CRC table is a compile-time constant") {
    static_assert(CRC8_TABLE[0] == 0x00 && CRC8_TABLE[1] == 0x07 && CRC8_TABLE[255] == 0xF3);
    constexpr auto crc = crc8(std::array<std::uint8_t, 2>{0x01, 0x02});
    static_assert(crc == crc8(std::string_view("\x01\x02")));
    CHECK_EQ(crc8(std::string("123456789")), 0xF4);  // and it still works at run time
}

TEST_CASE("consteval validates constants during compilation") {
    static_assert(RADIO_BAUD == 38400);
    constexpr auto fast = checked_baud(115200);
    CHECK_EQ(fast, 115200u);
}

TEST_CASE("the custom trait classifies field types") {
    static_assert(is_fixed_size_v<int> && is_fixed_size_v<float> && is_fixed_size_v<Status>);
    static_assert(is_fixed_size_v<std::array<std::uint16_t, 4>>);
    static_assert(!is_fixed_size_v<std::string> && !is_fixed_size_v<std::array<std::string, 2>>);
    static_assert(encoded_size<std::array<std::uint16_t, 4>>() == 8);
    static_assert(encoded_size<double>() == 2);
    CHECK_EQ(READING_BYTES, 10u);
}

TEST_CASE("integers and enums are written little-endian") {
    std::vector<std::uint8_t> out;
    serialise(out, std::uint16_t{0x1234});
    serialise(out, std::int32_t{-2});
    serialise(out, Status::sensor_fault);
    serialise(out, true);
    CHECK(out == std::vector<std::uint8_t>{0x34, 0x12, 0xFE, 0xFF, 0xFF, 0xFF, 0x02, 0x01});
}

TEST_CASE("floats become rounded centi-units and strings are length-prefixed") {
    std::vector<std::uint8_t> out;
    serialise(out, 21.456f);   // 2146 = 0x0862
    serialise(out, -0.0151);   // -1.51 rounds to -2
    serialise(out, std::string("ok"));
    CHECK(out == std::vector<std::uint8_t>{0x62, 0x08, 0xFE, 0xFF, 0x02, 'o', 'k'});
    static_assert(Serialisable<std::array<float, 2>> && Serialisable<const char*>);
}

TEST_CASE("a packet is the fields plus a CRC that detects corruption") {
    auto bytes = packet({7, 31.5f, 18.25f, Status::ok, {10, 20, 40}});
    CHECK_EQ(bytes.size(), READING_BYTES + 1);
    CHECK_EQ(hex({bytes.begin(), bytes.begin() + 4}), "07 00 4E 0C");
    const auto crc = bytes.back();
    bytes.pop_back();
    CHECK_EQ(crc8(bytes), crc);
    bytes[3] ^= 0x10;  // one flipped bit
    CHECK(crc8(bytes) != crc);
}

TEST_CASE("run prints a hex packet per reading") {
    std::istringstream in("7 31.5 18.25\n2 9 20\nstop\n");
    std::ostringstream out;
    CHECK_EQ(run(in, out), 0);
    CHECK(out.str().find("radio at 38400 baud, 10-byte readings + 1 CRC byte") != std::string::npos);
    CHECK(out.str().find("07 00 4E 0C 21 07 00 0A 14 28") != std::string::npos);
    CHECK(out.str().find("02 00 84 03 D0 07 01 0A 14 28") != std::string::npos);  // dry status
}
