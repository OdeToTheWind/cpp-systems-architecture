// tests/test_day_08.cpp
#include <cassert>
#include <string>
#include <iostream>

// Simulated grading function (extracted logic for testing)
std::string get_grade(double score) {
    if (score >= 90) return "A";
    if (score >= 80) return "B";
    if (score >= 70) return "C";
    if (score >= 60) return "D";
    return "F";
}

// Simulated age category
std::string get_age_category(int age) {
    if (age < 0) return "Invalid";
    if (age < 13) return "Child";
    if (age < 20) return "Teenager";
    if (age < 65) return "Adult";
    return "Senior";
}

int main() {
    // Grading tests
    assert(get_grade(95.0) == "A");
    assert(get_grade(85.0) == "B");
    assert(get_grade(72.5) == "C");
    assert(get_grade(65.0) == "D");
    assert(get_grade(55.0) == "F");
    assert(get_grade(100.0) == "A");
    assert(get_grade(0.0) == "F");

    // Age category tests
    assert(get_age_category(10) == "Child");
    assert(get_age_category(16) == "Teenager");
    assert(get_age_category(30) == "Adult");
    assert(get_age_category(70) == "Senior");
    assert(get_age_category(-5) == "Invalid");

    std::cout << "Day 08 conditional logic tests passed.\n";
    std::cout << "(Real behavior best seen by running main.cpp with different inputs)\n";

    return 0;
}