// tests/test_day_15.cpp
#include <cassert>
#include <iostream>

// Simulated loop-based functions for testing
long long sum_up_to(int n) {
    if (n <= 0) return 0;
    long long sum = 0;
    int i = 1;
    while (i <= n) {
        sum += i;
        ++i;
    }
    return sum;
}

int count_positive_numbers(const int arr[], int size) {
    int count = 0;
    int i = 0;
    while (i < size) {
        if (arr[i] > 0) count++;
        ++i;
    }
    return count;
}

int main() {
    // Sum test with while loop logic
    assert(sum_up_to(5) == 15);
    assert(sum_up_to(10) == 55);
    assert(sum_up_to(0) == 0);
    assert(sum_up_to(-3) == 0);

    // Count positive numbers
    int numbers[] = { -5, 3, 0, 7, -2, 12, 4 };
    assert(count_positive_numbers(numbers, 7) == 4);

    // Do-while simulation (at least one execution)
    int counter = 0;
    do {
        counter++;
    } while (counter < 3);
    assert(counter == 3);

    std::cout << "Day 15 while & do-while loops tests passed.\n";
    std::cout << "(Run main.cpp for interactive loop examples)\n";

    return 0;
}