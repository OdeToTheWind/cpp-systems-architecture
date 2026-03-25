#include <iostream>
#include <stdexcept>    // runtime_error, invalid_argument, out_of_range
#include <string>
#include <limits>

int main() {
    std::cout << "=== Day 11: Error Handling (try / catch) ===\n\n";

    // ── Example 1: Safe Division ───────────────────────────────────────────────
    double num1, num2;
    std::cout << "Enter two numbers for division: ";
    std::cin >> num1 >> num2;

    try {
        if (num2 == 0.0) {
            throw std::runtime_error("Division by zero is not allowed!");
        }
        double result = num1 / num2;
        std::cout << num1 << " / " << num2 << " = " << result << "\n";
    }
    catch (const std::runtime_error& e) {
        std::cout << "Error: " << e.what() << "\n";
    }

    std::cout << "\n";

    // ── Example 2: Age validation with exception ───────────────────────────────
    int age;
    std::cout << "Enter your age: ";
    try {
        if (!(std::cin >> age)) {
            throw std::invalid_argument("Invalid input: Age must be a number.");
        }
        if (age < 0) {
            throw std::out_of_range("Age cannot be negative.");
        }
        if (age > 150) {
            throw std::out_of_range("Age seems unrealistic (>150).");
        }

        std::cout << "Valid age entered: " << age << " years.\n";
    }
    catch (const std::invalid_argument& e) {
        std::cout << "Input Error: " << e.what() << "\n";
    }
    catch (const std::out_of_range& e) {
        std::cout << "Range Error: " << e.what() << "\n";
    }

    // Clear input buffer for next section
    std::cin.clear();
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

    std::cout << "\n";

    // ── Example 3: Custom exception for password validation ─────────────────────
    std::string password;
    std::cout << "Create a password (min 8 characters): ";
    std::getline(std::cin, password);

    try {
        if (password.length() < 8) {
            throw std::runtime_error("Password too short! Minimum 8 characters required.");
        }
        if (password.find(' ') != std::string::npos) {
            throw std::runtime_error("Password cannot contain spaces.");
        }

        std::cout << "Password accepted successfully!\n";
    }
    catch (const std::runtime_error& e) {
        std::cout << "Password Error: " << e.what() << "\n";
    }

    std::cout << "\nKey takeaway:\n"
              << "  * Use exceptions for exceptional/error cases\n"
              << "  * Catch specific exceptions before general ones\n"
              << "  * Provide clear error messages with .what()\n"
              << "  * Prefer std::runtime_error and std::invalid_argument\n";

    return 0;
}