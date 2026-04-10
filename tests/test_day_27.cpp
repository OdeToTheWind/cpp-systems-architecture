// tests/test_day_27.cpp
#include <cassert>
#include <iostream>

class TestPlayer {
private:
    int health = 100;
public:
    void take_damage(int d) {
        if (d > 0) health -= d;
        if (health < 0) health = 0;
    }
    int get_health() const { return health; }
};

int main() {
    TestPlayer p;
    p.take_damage(40);
    assert(p.get_health() == 60);

    p.take_damage(100);  // should not go below 0
    assert(p.get_health() == 0);

    std::cout << "Day 27 OOP Basics (Encapsulation) tests passed.\n";
    return 0;
}