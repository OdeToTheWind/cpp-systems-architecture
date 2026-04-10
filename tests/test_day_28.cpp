// tests/test_day_28.cpp
#include <cassert>
#include <iostream>

class TestAccount {
private:
    double balance = 0;
public:
    void deposit(double a) { if (a > 0) balance += a; }
    double get_balance() const { return balance; }
};

int main() {
    TestAccount acc;
    acc.deposit(500);
    assert(acc.get_balance() == 500);
    std::cout << "Day 28 class creation tests passed.\n";
    return 0;
}