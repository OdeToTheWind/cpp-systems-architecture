// tests/test_day_07.cpp
#include <cassert>
#include <cstdint>
#include <iostream>
#include <limits>

bool approx_equal(double a, double b, double epsilon = 1e-9) {
    return std::abs(a - b) < epsilon;
}

int main() {
    // Implicit conversion checks
    char c = 'A';
    assert(static_cast<int>(c) == 65);

    int i = 42;
    assert(approx_equal(static_cast<double>(i), 42.0));

    double d = 123.456;
    assert(static_cast<int>(d) == 123);  // truncates

    // Explicit cast behavior
    int s32 = -100;
    unsigned int u32 = static_cast<unsigned int>(s32);
    assert(u32 == 4294967196U);  // 2^32 - 100

    // Safe range check simulation
    int positive = 500;
    unsigned int safe_u = (positive >= 0) ? static_cast<unsigned int>(positive) : 0;
    assert(safe_u == 500);

    std::cout << "Day 07 type conversion & casting tests passed.\n";
    std::cout << "(Most insights come from running main.cpp with different inputs)\n";

    return 0;
}