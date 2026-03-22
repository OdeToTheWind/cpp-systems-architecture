// tests/test_day_06.cpp
#include <cassert>
#include <cstdint>
#include <iostream>
#include <limits>
#include <cstddef>

int main() {
    // Basic size checks (platform-dependent but reasonable minimums)
    assert(sizeof(char) == 1);
    assert(sizeof(int) >= 2);
    assert(sizeof(double) >= 4);
    assert(sizeof(long long) >= 8);

    // Fixed-width types must be exact
    assert(sizeof(std::int8_t)  == 1);
    assert(sizeof(std::uint16_t) == 2);
    assert(sizeof(std::int32_t) == 4);
    assert(sizeof(std::int64_t) == 8);

    // Range checks via numeric_limits
    assert(std::numeric_limits<std::uint8_t>::max() == 255);
    assert(std::numeric_limits<std::int16_t>::min() == -32768);
    assert(std::numeric_limits<std::int16_t>::max() == 32767);

    // size_t is unsigned and large enough for array indices
    assert(sizeof(std::size_t) >= sizeof(void*));
    assert(std::numeric_limits<std::size_t>::max() > 0xFFFF'FFFF); // at least 32-bit

    std::cout << "Day 06 data types size & range checks passed.\n";
    std::cout << "(Most value is in observing output of main.cpp on your platform)\n";

    return 0;
}