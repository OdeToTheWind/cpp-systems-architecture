// tests/test_day_11.cpp
#include <cassert>
#include <stdexcept>
#include <string>
#include <iostream>

// Simulated functions that throw exceptions
double safe_divide(double a, double b) {
    if (b == 0.0) {
        throw std::runtime_error("Division by zero");
    }
    return a / b;
}

void validate_age(int age) {
    if (age < 0) {
        throw std::out_of_range("Age cannot be negative");
    }
    if (age > 150) {
        throw std::out_of_range("Age is unrealistic");
    }
}

bool is_valid_password(const std::string& pwd) {
    if (pwd.length() < 8) {
        throw std::runtime_error("Password too short");
    }
    return true;
}

int main() {
    // Test safe divide
    assert(safe_divide(10.0, 2.0) == 5.0);
    try {
        safe_divide(10.0, 0.0);
        assert(false); // should not reach here
    } catch (const std::runtime_error&) {
        // expected
    }

    // Test age validation
    try {
        validate_age(25);
        validate_age(-5);   // should throw
        assert(false);
    } catch (const std::out_of_range&) {
        // expected
    }

    // Test password
    try {
        assert(is_valid_password("SecurePass123") == true);
    } catch (...) {
        assert(false);
    }

    try {
        is_valid_password("short");
        assert(false);
    } catch (const std::runtime_error&) {
        // expected
    }

    std::cout << "Day 11 error handling (try/catch) tests passed.\n";
    std::cout << "(Run main.cpp to see interactive exception handling)\n";

    return 0;
}