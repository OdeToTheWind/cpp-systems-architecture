// tests/test_day_04.cpp
// Day 4 tests are mostly compile-time + readability focused.
// No heavy logic yet — just ensuring names are used consistently.

#include <cassert>
#include <iostream>
#include <string>

// These names should compile and be intention-revealing
int number_of_students_enrolled = 0;
double average_temperature_celsius = 0.0;
bool has_user_accepted_terms = false;

constexpr int MAXIMUM_LOGIN_ATTEMPTS = 5;

// Simulated function using good names
bool can_user_log_in(int attempts_used, bool account_locked) {
    return (attempts_used < MAXIMUM_LOGIN_ATTEMPTS) && !account_locked;
}

int main() {
    // Compile-time checks (these should all succeed)
    assert(MAXIMUM_LOGIN_ATTEMPTS == 5);

    // Logic check with intention-revealing names
    assert(can_user_log_in(3, false) == true);
    assert(can_user_log_in(5, false) == false);
    assert(can_user_log_in(2, true) == false);

    std::cout << "Day 04 naming convention checks passed.\n";
    std::cout << "(Most value here is in reading & understanding the main.cpp names)\n";

    return 0;
}