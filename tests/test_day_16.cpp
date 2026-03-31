// tests/test_day_16.cpp
#include <cassert>
#include <string>
#include <iostream>

// Simulated flowchart logic functions
std::string get_grade(double score) {
    if (score >= 90 && score <= 100) return "A";
    if (score >= 80) return "B";
    if (score >= 70) return "C";
    if (score >= 60) return "D";
    if (score >= 0) return "F";
    return "Invalid";
}

std::string classify_number(int n) {
    if (n > 0) return "Positive";
    if (n < 0) return "Negative";
    return "Zero";
}

bool can_withdraw(int amount, int balance) {
    return (amount > 0 && amount <= balance && amount % 100 == 0);
}

int main() {
    // Grading flowchart logic
    assert(get_grade(95) == "A");
    assert(get_grade(82) == "B");
    assert(get_grade(65) == "D");
    assert(get_grade(45) == "F");
    assert(get_grade(101) == "Invalid");

    // Number classification
    assert(classify_number(42) == "Positive");
    assert(classify_number(-7) == "Negative");
    assert(classify_number(0) == "Zero");

    // ATM withdrawal logic
    assert(can_withdraw(500, 5000) == true);
    assert(can_withdraw(1500, 1000) == false);
    assert(can_withdraw(-100, 5000) == false);
    assert(can_withdraw(250, 5000) == false);  // not multiple of 100

    std::cout << "Day 16 flowchart programming tests passed.\n";
    std::cout << "(Run main.cpp to see multiple flowchart implementations)\n";

    return 0;
}