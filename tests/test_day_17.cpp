// tests/test_day_17.cpp
#include <cassert>
#include <vector>
#include <map>
#include <string>
#include <iostream>

int main() {
    // Vector tests
    std::vector<int> vec;
    vec.push_back(10);
    vec.push_back(20);
    vec.push_back(30);

    assert(vec.size() == 3);
    assert(vec[1] == 20);

    // Map tests
    std::map<std::string, double> scores;
    scores["Alice"] = 95.5;
    scores["Bob"] = 87.0;

    assert(scores.size() == 2);
    assert(scores["Alice"] == 95.5);
    assert(scores.count("Charlie") == 0);

    // Range-based loop simulation
    double total = 0;
    for (const auto& pair : scores) {
        total += pair.second;
    }
    assert(total == 182.5);

    std::cout << "Day 17 vectors and maps tests passed.\n";
    std::cout << "(Run main.cpp for interactive student score and shopping list demo)\n";

    return 0;
}