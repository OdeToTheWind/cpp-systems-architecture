#include <iostream>
#include <string>

int main() {
    std::cout << "=== Day 16: Flowchart Programming ===\n\n";

    // ── Flowchart 1: Eligibility Checker ───────────────────────────────────────
    int age;
    bool has_license;

    std::cout << "=== Driving License Eligibility ===\n";
    std::cout << "Enter your age: ";
    std::cin >> age;
    std::cout << "Do you have a learner's permit? (1 = Yes, 0 = No): ";
    std::cin >> has_license;

    std::cout << "Result: ";
    if (age >= 18) {
        if (has_license) {
            std::cout << "You are eligible for a full driving license.\n";
        } else {
            std::cout << "You need a learner's permit first.\n";
        }
    } else {
        std::cout << "You are too young to drive.\n";
    }
    std::cout << "\n";

    // ── Flowchart 2: Grading System ─────────────────────────────────────────────
    double score;
    std::cout << "=== Student Grading System ===\n";
    std::cout << "Enter your score (0-100): ";
    std::cin >> score;

    std::cout << "Grade: ";
    if (score >= 90 && score <= 100) {
        std::cout << "A - Outstanding\n";
    } else if (score >= 80) {
        std::cout << "B - Very Good\n";
    } else if (score >= 70) {
        std::cout << "C - Good\n";
    } else if (score >= 60) {
        std::cout << "D - Pass\n";
    } else if (score >= 0) {
        std::cout << "F - Fail\n";
    } else {
        std::cout << "Invalid score!\n";
    }
    std::cout << "\n";

    // ── Flowchart 3: Number Classification ──────────────────────────────────────
    int number;
    std::cout << "=== Number Classifier ===\n";
    std::cout << "Enter any integer: ";
    std::cin >> number;

    std::cout << number << " is ";
    if (number > 0) {
        std::cout << "Positive\n";
    } else if (number < 0) {
        std::cout << "Negative\n";
    } else {
        std::cout << "Zero\n";
    }
    std::cout << "\n";

    // ── Flowchart 4: Simple ATM Withdrawal Simulation ───────────────────────────
    int balance = 5000.0;
    int withdraw;
    std::cout << "=== Simple ATM ===\n";
    std::cout << "Current balance: Rupees " << balance << "\n";
    std::cout << "Enter amount to withdraw: Rupees ";
    std::cin >> withdraw;

    if (withdraw <= 0) {
        std::cout << "Invalid amount.\n";
    } else if (withdraw > balance) {
        std::cout << "Insufficient balance!\n";
    } else if (withdraw % 100 != 0) {
        std::cout << "Amount must be multiple of 100.\n";
    } else {
        balance -= withdraw;
        std::cout << "Withdrawal successful! Remaining balance: Rupees " << balance << "\n";
    }

    std::cout << "\nKey takeaway:\n"
              << "  * Always design logic using flowcharts before coding\n"
              << "  * Each diamond (decision) becomes if/else\n"
              << "  * Clear flowcharts lead to cleaner, bug-free code\n"
              << "  * Good structure prevents spaghetti code\n";

    return 0;
}