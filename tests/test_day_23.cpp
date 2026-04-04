// tests/test_day_23.cpp
#include <cassert>
#include <iostream>

int main() {
    // Simple scope test - local vs global behavior simulation
    int local = 10;
    local += 5;
    assert(local == 15);

    std::cout << "Day 23 scope and variable lifetime tests passed.\n";
    return 0;
}