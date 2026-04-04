// tests/test_day_21.cpp
#include <cassert>
#include <stdexcept>
#include <iostream>

double good_calculator(double a, double b, char op) {
    if (op == '+') return a + b;
    if (op == '-') return a - b;
    if (op == '*') return a * b;
    if (op == '/' && b != 0.0) return a / b;
    throw std::invalid_argument("Invalid");
}

int main() {
    assert(good_calculator(10, 5, '+') == 15);
    assert(good_calculator(10, 5, '-') == 5);
    assert(good_calculator(10, 5, '*') == 50);

    try {
        good_calculator(10, 0, '/');
        assert(false);
    } catch (...) {}

    std::cout << "Day 21 return vs print tests passed.\n";
    return 0;
}