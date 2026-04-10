// tests/test_day_24.cpp
#include <cassert>
#include <iostream>

long long sum_to_n(int n) {
    long long sum = 0;
    for (int i = 1; i <= n; ++i) sum += i;
    return sum;
}

int main() {
    assert(sum_to_n(5) == 15);
    assert(sum_to_n(10) == 55);
    assert(sum_to_n(1) == 1);
    assert(sum_to_n(0) == 0);

    std::cout << "Day 24 debugging techniques tests passed.\n";
    return 0;
}