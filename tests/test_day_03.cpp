// tests/test_day_03.cpp
#include <cassert>
#include <string>
#include <sstream>
#include <iostream>

// Simulated BMI calculation (same logic as main)
bool calculate_bmi(double height_cm, double weight_kg, double& out_bmi) {
    if (height_cm <= 0 || weight_kg <= 0) {
        return false;
    }
    out_bmi = weight_kg / ((height_cm / 100.0) * (height_cm / 100.0));
    return true;
}

// Simulated gender category string
std::string get_gender_string(char input) {
    char lower = std::tolower(input);
    if (lower == 'm') return "Male";
    if (lower == 'f') return "Female";
    if (lower == 'o') return "Other";
    return "Invalid";
}

int main() {
    double bmi = 0.0;

    // Test BMI calculation
    assert(calculate_bmi(170.0, 65.0, bmi) == true);
    assert(bmi >= 22.4 && bmi <= 22.6);  // ≈22.49

    assert(calculate_bmi(160.0, 55.0, bmi) == true);
    assert(bmi >= 21.4 && bmi <= 21.6);  // ≈21.48

    assert(calculate_bmi(180.0, 90.0, bmi) == true);
    assert(bmi >= 27.7 && bmi <= 27.9);  // ≈27.78

    // Invalid inputs
    assert(calculate_bmi(0, 70.0, bmi) == false);
    assert(calculate_bmi(175.0, -5.0, bmi) == false);

    // Test gender mapping
    assert(get_gender_string('M') == "Male");
    assert(get_gender_string('f') == "Female");
    assert(get_gender_string('O') == "Other");
    assert(get_gender_string('x') == "Invalid");

    std::cout << "Day 03 tests passed (BMI calculation + gender mapping).\n";
    return 0;
}