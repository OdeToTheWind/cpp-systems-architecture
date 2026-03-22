#include <iostream>
#include <string>
#include <limits>

int main() {
    std::cout << "=== Day 09: Logical Operators (&& || !) ===\n\n";

    // ── Input values ────────────────────────────────────────────────────────────
    int age;
    std::cout << "Enter your age: ";
    if (!(std::cin >> age) || age < 0) {
        std::cout << "Invalid age.\n";
        return 1;
    }

    bool has_license;
    std::cout << "Do you have a driving license? (1 = yes, 0 = no): ";
    std::cin >> has_license;

    bool has_insurance;
    std::cout << "Do you have vehicle insurance? (1 = yes, 0 = no): ";
    std::cin >> has_insurance;

    std::cout << "\n";

    // ── Example 1: Driving eligibility (AND) ────────────────────────────────────
    std::cout << "Driving eligibility check:\n";
    if (age >= 18 && has_license && has_insurance) {
        std::cout << " You can drive legally.\n";
    } else {
        std::cout << " You cannot drive legally. Reasons:\n";
        if (age < 18)          std::cout << "  - Under 18\n";
        if (!has_license)      std::cout << "  - No license\n";
        if (!has_insurance)    std::cout << "  - No insurance\n";
    }

    std::cout << "\n";

    // ── Example 2: Discount eligibility (OR + AND) ──────────────────────────────
    double purchase_amount;
    std::cout << "Enter purchase amount (Rupees): ";
    std::cin >> purchase_amount;

    bool is_member;
    std::cout << "Are you a loyalty member? (1 = yes, 0 = no): ";
    std::cin >> is_member;

    bool is_weekend;
    std::cout << "Is it weekend? (1 = yes, 0 = no): ";
    std::cin >> is_weekend;

    std::cout << "Discount eligibility: ";
    if (purchase_amount >= 5000 || (is_member && is_weekend)) {
        std::cout << "Eligible for 10% discount!\n";
    } else {
        std::cout << "No discount this time.\n";
    }

    std::cout << "\n";

    // ── Example 3: Password strength check (NOT + AND) ──────────────────────────
    std::string password;
    std::cout << "Enter a password to check strength: ";
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    std::getline(std::cin, password);

    bool has_upper = false, has_lower = false, has_digit = false;
    for (char c : password) {
        if (c >= 'A' && c <= 'Z') has_upper = true;
        if (c >= 'a' && c <= 'z') has_lower = true;
        if (c >= '0' && c <= '9') has_digit = true;
    }

    std::cout << "Password strength: ";
    if (password.length() >= 8 && has_upper && has_lower && has_digit) {
        std::cout << "Strong\n";
    } else {
        std::cout << "Weak — improve by:\n";
        if (password.length() < 8)        std::cout << "  - Make it at least 8 characters\n";
        if (!has_upper)                   std::cout << "  - Add uppercase letter\n";
        if (!has_lower)                   std::cout << "  - Add lowercase letter\n";
        if (!has_digit)                   std::cout << "  - Add a digit\n";
    }

    std::cout << "\nKey takeaway:\n"
              << "  * && = both must be true\n"
              << "  * || = at least one true\n"
              << "  * ! = reverse logic\n"
              << "  * Use parentheses to control evaluation order\n"
              << "  * Short-circuiting prevents unnecessary work (and crashes)\n";

    return 0;
}