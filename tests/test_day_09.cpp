// tests/test_day_09.cpp
#include <cassert>
#include <string>
#include <iostream>

// Simulated functions for testing logical combinations
bool can_drive(int age, bool has_license, bool has_insurance) {
    return age >= 18 && has_license && has_insurance;
}

bool gets_discount(double amount, bool is_member, bool is_weekend) {
    return amount >= 5000 || (is_member && is_weekend);
}

bool is_strong_password(const std::string& pwd) {
    if (pwd.length() < 8) return false;

    bool has_upper = false, has_lower = false, has_digit = false;
    for (char c : pwd) {
        if (c >= 'A' && c <= 'Z') has_upper = true;
        if (c >= 'a' && c <= 'z') has_lower = true;
        if (c >= '0' && c <= '9') has_digit = true;
    }
    return has_upper && has_lower && has_digit;
}

int main() {
    // Driving eligibility
    assert(can_drive(20, true, true) == true);
    assert(can_drive(17, true, true) == false);
    assert(can_drive(25, false, true) == false);
    assert(can_drive(30, true, false) == false);

    // Discount logic
    assert(gets_discount(6000, false, false) == true);
    assert(gets_discount(3000, true, true) == true);
    assert(gets_discount(4000, false, false) == false);

    // Password strength
    assert(is_strong_password("Passw0rd") == true);
    assert(is_strong_password("password") == false);     // no upper, no digit
    assert(is_strong_password("P4ss") == false);          // too short
    assert(is_strong_password("P4ssword!") == true);

    std::cout << "Day 09 logical operators tests passed.\n";
    std::cout << "(Run main.cpp with different inputs to see combinations in action)\n";

    return 0;
}