#include <iostream>
#include <string>
#include <limits>

int main() {
    std::cout << "=== Day 08: Conditional Statements (if / else) ===\n\n";

    // ── Example 1: Age-based category ───────────────────────────────────────────
    int age;
    std::cout << "Enter your age: ";
    if (!(std::cin >> age) || age < 0) {
        std::cout << "Invalid age entered.\n";
        return 1;
    }

    std::cout << "You are: ";
    if (age < 13) {
        std::cout << "Child\n";
    } else if (age < 20) {
        std::cout << "Teenager\n";
    } else if (age < 65) {
        std::cout << "Adult\n";
    } else {
        std::cout << "Senior\n";
    }

    std::cout << "\n";

    // ── Example 2: Simple grading system ────────────────────────────────────────
    double score;
    std::cout << "Enter your exam score (0-100): ";
    if (!(std::cin >> score) || score < 0 || score > 100) {
        std::cout << "Score must be between 0 and 100.\n";
        return 1;
    }

    std::cout << "Grade: ";
    if (score >= 90) {
        std::cout << "A (Excellent)\n";
    } else if (score >= 80) {
        std::cout << "B (Good)\n";
    } else if (score >= 70) {
        std::cout << "C (Average)\n";
    } else if (score >= 60) {
        std::cout << "D (Pass)\n";
    } else {
        std::cout << "F (Fail)\n";
    }

    std::cout << "\n";

    // ── Example 3: Temperature advice with nested conditions ────────────────────
    double temperature_c;
    std::cout << "Enter current temperature (degree Celsius): ";
    if (!(std::cin >> temperature_c)) {
        std::cout << "Invalid temperature input.\n";
        return 1;
    }

    std::cout << "Weather advice: ";
    if (temperature_c >= 35) {
        std::cout << "Extreme heat - stay indoors, hydrate!\n";
    } else if (temperature_c >= 25) {
        std::cout << "Hot - light clothing, drink water.\n";
    } else if (temperature_c >= 15) {
        std::cout << "Pleasant - enjoy the day!\n";
        if (temperature_c >= 20) {
            std::cout << "   → Perfect for a walk.\n";
        }
    } else if (temperature_c >= 0) {
        std::cout << "Cold - wear a jacket.\n";
    } else {
        std::cout << "Freezing - bundle up heavily!\n";
    }

    std::cout << "\nKey takeaway:\n"
              << "  * Use if/else if chains for multiple conditions\n"
              << "  * Validate input early\n"
              << "  * Keep nesting shallow when possible\n";

    return 0;
}