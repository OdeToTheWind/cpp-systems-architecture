// tests/test_day_05.cpp
#include <cassert>
#include <cmath>
#include <iostream>
#include <limits>

bool approx_equal(double a, double b, double epsilon = 1e-9) {
    return std::abs(a - b) < epsilon;
}

int main() {
    // Basic arithmetic (integer)
    assert(17 / 5 == 3);
    assert(17 % 5 == 2);
    assert((10 + 5) * 2 == 30);
    assert(-17 / 5 == -3);   // toward zero in C++11+

    // Floating point compound assignment simulation (matching main.cpp logic)
    double val = 100.0;
    val += 25.0;  // discount
    val *= 1.1;   // 10% tax
    assert(approx_equal(val, 137.5));

    // Increment / Decrement behavior
    int n = 5;
    int pre = ++n;   // n=6, pre=6
    int post = n++;  // post=6, n=7
    assert(pre == 6);
    assert(post == 6);
    assert(n == 7);

    --n;             // n=6
    assert(n == 6);

    // <cmath> functions
    assert(approx_equal(std::sqrt(144.0), 12.0));
    assert(approx_equal(std::pow(3.0, 4.0), 81.0));
    assert(approx_equal(std::hypot(5.0, 12.0), 13.0));
    assert(std::round(4.6) == 5.0);
    assert(std::round(-4.6) == -5.0);
    assert(std::abs(-7.8) == 7.8);

    std::cout << "Day 05 mathematical operations tests passed.\n";
    return 0;
}