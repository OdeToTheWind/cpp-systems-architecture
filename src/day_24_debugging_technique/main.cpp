#include <iostream>
#include <string>

int main() {
    std::cout << "=== Day 24: Debugging Techniques ===\n\n";

    std::cout << "Let's practice debugging with interactive examples.\n\n";

    // Buggy function example 1: Sum 1 to N (has off-by-one bug)
    int n;
    std::cout << "Enter N to calculate sum 1 to N: ";
    std::cin >> n;

    long long sum = 0;
    for (int i = 1; i <= n; ++i) {   // correct version
        sum += i;
    }
    std::cout << "Correct sum 1 to " << n << " = " << sum << "\n\n";

    // Interactive debugging simulation
    std::string bug_report;
    std::cout << "Describe a bug you encountered recently (or type 'skip'): ";
    std::cin.ignore();
    std::getline(std::cin, bug_report);

    if (bug_report != "skip" && !bug_report.empty()) {
        std::cout << "Debugging tip: Reproduce → Isolate → Fix → Test\n";
        std::cout << "Your bug: " << bug_report << "\n";
    }

    std::cout << "\nKey takeaway:\n"
              << "  * Use std::cerr for debug output (separate from normal output)\n"
              << "  * Add temporary print statements to trace variable values\n"
              << "  * Always reproduce the bug before trying to fix it\n"
              << "  * Good debugging skills are as important as writing code\n";

    return 0;
}