// tests/test_day_10.cpp
#include <cassert>
#include <random>
#include <limits>
#include <iostream>

int main() {
    std::random_device rd;
    std::mt19937 gen(rd());

    // Test uniform int distribution range
    std::uniform_int_distribution<int> dist(1, 10);
    for (int i = 0; i < 100; ++i) {
        int val = dist(gen);
        assert(val >= 1 && val <= 10);
    }

    // Test uniform real distribution
    std::uniform_real_distribution<double> real_dist(0.0, 1.0);
    for (int i = 0; i < 50; ++i) {
        double val = real_dist(gen);
        assert(val >= 0.0 && val <= 1.0);
    }

    // Test bernoulli distribution roughly
    std::bernoulli_distribution coin(0.5);
    int heads = 0;
    for (int i = 0; i < 1000; ++i) {
        if (coin(gen)) heads++;
    }
    // Should be roughly 50% (within reasonable tolerance)
    assert(heads >= 400 && heads <= 600);

    std::cout << "Day 10 randomisation tests passed.\n";
    std::cout << "(Run main.cpp for interactive random examples)\n";

    return 0;
}