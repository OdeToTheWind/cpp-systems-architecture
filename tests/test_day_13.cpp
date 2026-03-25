// tests/test_day_13.cpp
#include <cassert>
#include <vector>
#include <iostream>

// Helper functions for testing
long long sum_first_n(int n) {
    if (n <= 0) return 0;
    long long sum = 0;
    for (int i = 1; i <= n; ++i) {
        sum += i;
    }
    return sum;
}

long long factorial(int n) {
    if (n < 0) return 0;
    long long fact = 1;
    for (int i = 1; i <= n; ++i) {
        fact *= i;
    }
    return fact;
}

int main() {
    // Sum tests
    assert(sum_first_n(5) == 15);
    assert(sum_first_n(10) == 55);
    assert(sum_first_n(1) == 1);
    assert(sum_first_n(0) == 0);

    // Factorial tests
    assert(factorial(0) == 1);
    assert(factorial(1) == 1);
    assert(factorial(5) == 120);
    assert(factorial(6) == 720);

    // Vector + range-based loop simulation
    std::vector<int> nums = {5, 15, 25, 35};
    long long vec_sum = 0;
    for (int val : nums) {
        vec_sum += val;
    }
    assert(vec_sum == 80);

    std::cout << "Day 13 for loops tests passed.\n";
    std::cout << "(Run main.cpp for interactive loop examples)\n";

    return 0;
}