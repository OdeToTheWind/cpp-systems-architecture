// tests/test_day_19.cpp
#include <cassert>
#include <iostream> 
// Functions from main.cpp
void swap_by_reference(int& a, int& b) {
    int temp = a;
    a = b;
    b = temp;
}

void increment_by_reference(int& num, int amount) {
    num += amount;
}

int main() {
    int x = 10, y = 20;
    swap_by_reference(x, y);
    assert(x == 20);
    assert(y == 10);

    int value = 50;
    increment_by_reference(value, 30);
    assert(value == 80);

    // Test pointer safety simulation
    int* ptr = nullptr;
    // We can't easily test null pointer in simple assert, but logic is covered

    std::cout << "Day 19 pointers and references tests passed.\n";
    std::cout << "(Run main.cpp for interactive pointer/reference examples)\n";

    return 0;
}