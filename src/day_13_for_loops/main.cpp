#include <iostream>
#include <vector>
#include <iomanip>

int main() {
    std::cout << "=== Day 13: For Loops in C++ ===\n\n";

    // ── Example 1: Sum of first N natural numbers ───────────────────────────────
    int n;
    std::cout << "Enter a positive integer N: ";
    std::cin >> n;

    if (n <= 0) {
        std::cout << "Please enter a positive number.\n";
        return 1;
    }

    long long sum = 0;
    for (int i = 1; i <= n; ++i) {
        sum += i;
    }
    std::cout << "Sum of first " << n << " numbers = " << sum << "\n\n";

    // ── Example 2: Multiplication Table ─────────────────────────────────────────
    int num;
    std::cout << "Enter a number for multiplication table: ";
    std::cin >> num;

    int terms;
    std::cout << "How many terms? ";
    std::cin >> terms;

    std::cout << "\nMultiplication Table of " << num << ":\n";
    for (int i = 1; i <= terms; ++i) {
        std::cout << num << " x " << std::setw(2) << i 
                  << " = " << (num * i) << "\n";
    }
    std::cout << "\n";

    // ── Example 3: Star Pattern Printer ─────────────────────────────────────────
    int rows;
    std::cout << "Enter number of rows for star pattern: ";
    std::cin >> rows;

    if (rows > 0) {
        std::cout << "Star Pattern:\n";
        for (int i = 1; i <= rows; ++i) {
            for (int j = 1; j <= i; ++j) {
                std::cout << "* ";
            }
            std::cout << "\n";
        }
    }
    std::cout << "\n";

    // ── Example 4: Range-based for loop with vector ─────────────────────────────
    int count;
    std::cout << "How many random numbers to generate and sum? ";
    std::cin >> count;

    if (count <= 0) count = 5;

    std::vector<int> numbers;
    numbers.reserve(count);

    // Fill vector using traditional for loop
    for (int i = 0; i < count; ++i) {
        numbers.push_back(i * 10 + 5);  // 5, 15, 25, ...
    }

    std::cout << "Numbers in vector: ";
    long long vector_sum = 0;

    // Range-based for loop (modern style)
    for (int val : numbers) {
        std::cout << val << " ";
        vector_sum += val;
    }
    std::cout << "\nSum using range-based for loop: " << vector_sum << "\n\n";

    std::cout << "Key takeaway:\n"
              << "  * Use for loops when you know the number of iterations\n"
              << "  * Range-based for is cleaner when iterating over containers\n"
              << "  * Always validate input before starting a loop\n";

    return 0;
}