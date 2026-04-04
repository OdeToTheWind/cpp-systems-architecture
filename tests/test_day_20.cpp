// tests/test_day_20.cpp
#include <cassert>
#include <functional>
#include <string>
#include <iostream>

// Functions from main.cpp
std::function<double(double, double)> get_operation(const std::string& op) {
    if (op == "add" || op == "+") return [](double a, double b){ return a + b; };
    if (op == "multiply" || op == "*") return [](double a, double b){ return a * b; };
    return [](double a, double b){ return a + b; }; // default
}

int main() {
    auto add = get_operation("add");
    assert(add(5.0, 3.0) == 8.0);

    auto mul = get_operation("multiply");
    assert(mul(4.0, 5.0) == 20.0);

    // Test lambda returning function
    auto make_double = [](int n) {
        return [n](int x){ return x * n; };
    };

    auto doubler = make_double(2);
    assert(doubler(7) == 14);

    std::cout << "Day 20 returning functions tests passed.\n";
    std::cout << "(Run main.cpp for interactive examples of returned functions)\n";

    return 0;
}