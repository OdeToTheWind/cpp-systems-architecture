#include <iostream>
#include <string>

// Global variable (generally discouraged)
int global_counter = 0;

namespace utils {
    // File-local global using static (better than plain global)
    static int internal_counter = 0;
}

int main() {
    std::cout << "=== Day 23: Scope and Local/Global Variables ===\n\n";

    int local_counter = 0;  // Local to main()

    std::string name;
    std::cout << "Enter your name: ";
    std::getline(std::cin, name);

    int iterations;
    std::cout << "How many times to increment? ";
    std::cin >> iterations;

    for (int i = 0; i < iterations; ++i) {
        local_counter++;           // Local variable
        global_counter++;          // Global variable
        utils::internal_counter++; // Namespace-static variable
    }

    std::cout << "\nResults:\n";
    std::cout << "Local counter     : " << local_counter << "\n";
    std::cout << "Global counter    : " << global_counter << " (shared across functions)\n";
    std::cout << "Internal counter  : " << utils::internal_counter << " (file-local)\n";

    std::cout << "\nKey takeaway:\n"
              << "  * Prefer local variables and explicit parameter passing\n"
              << "  * Global variables make code harder to reason about and test\n"
              << "  * Use namespaces and static for controlled global-like behavior\n";

    return 0;
}