// tests/test_day_12.cpp
#include <cassert>
#include <string>
#include <stdexcept>
#include <iostream>

// Helper functions from main.cpp (for testing)
double power(double base, int exponent) {
    if (exponent < 0) throw std::invalid_argument("Negative exponent");
    double result = 1.0;
    for (int i = 0; i < exponent; ++i) result *= base;
    return result;
}

double calculate_bmi(double weight_kg, double height_cm) {
    if (weight_kg <= 0 || height_cm <= 0) {
        throw std::invalid_argument("Invalid dimensions");
    }
    double height_m = height_cm / 100.0;
    return weight_kg / (height_m * height_m);
}

std::string describe_number(int n) {
    return (n % 2 == 0) ? "Even" : "Odd";
}

int main() {
    // Power function tests
    assert(power(2.0, 3) == 8.0);
    assert(power(5.0, 0) == 1.0);

    // BMI calculation
    assert(calculate_bmi(70.0, 170.0) > 24.0);
    assert(calculate_bmi(55.0, 160.0) > 21.0);

    // Overloading
    assert(describe_number(4) == "Even");
    assert(describe_number(7) == "Odd");

    // Exception test
    try {
        power(2.0, -1);
        assert(false);
    } catch (const std::invalid_argument&) {
        // expected
    }

    std::cout << "Day 12 functions tests passed.\n";
    std::cout << "(Run main.cpp to see interactive function usage)\n";

    return 0;
}