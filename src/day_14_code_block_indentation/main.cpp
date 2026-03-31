#include <iostream>
#include <string>

int main() {
    std::cout << "=== Day 14: Code Blocks and Indentation ===\n\n";

    // ── Example 1: Proper if-else with blocks ───────────────────────────────────
    int age;
    std::cout << "Enter your age: ";
    std::cin >> age;

    std::cout << "Category: ";
    if (age < 0) {
        std::cout << "Invalid age!\n";
    } 
    else if (age < 13) {
        std::cout << "Child\n";
    } 
    else if (age < 20) {
        std::cout << "Teenager\n";
    } 
    else if (age < 65) {
        std::cout << "Adult\n";
    } 
    else {
        std::cout << "Senior Citizen\n";
    }
    std::cout << "\n";

    // ── Example 2: Nested blocks with proper indentation ───────────────────────
    int score;
    std::cout << "Enter your exam score (0-100): ";
    std::cin >> score;

    std::cout << "Result: ";
    if (score >= 0 && score <= 100) {
        if (score >= 90) {
            std::cout << "Excellent - Grade A\n";
        } 
        else if (score >= 75) {
            std::cout << "Very Good - Grade B\n";
        } 
        else if (score >= 60) {
            std::cout << "Pass - Grade C\n";
        } 
        else {
            std::cout << "Needs Improvement\n";
        }
    } 
    else {
        std::cout << "Invalid score! Must be between 0 and 100.\n";
    }
    std::cout << "\n";

    // ── Example 3: Function-like block with clear scoping ───────────────────────
    std::string name;
    std::cout << "Enter your name: ";
    std::cin.ignore();
    std::getline(std::cin, name);

    // Clear block for name validation
    {
        if (name.empty()) {
            std::cout << "Warning: Name cannot be empty.\n";
        } 
        else if (name.length() > 50) {
            std::cout << "Warning: Name is very long.\n";
        } 
        else {
            std::cout << "Hello, " << name << "!\n";
        }
    }

    std::cout << "\nKey takeaway:\n"
              << "  * Always use braces {} for code blocks — even for single statements\n"
              << "  * Consistent 4-space indentation greatly improves readability\n"
              << "  * Proper scoping prevents bugs and makes code maintainable\n"
              << "  * Clean indentation is a hallmark of professional code\n";

    return 0;
}