// tests/test_day_18.cpp
#include <cassert>
#include <string>
#include <iostream>

// Functions from main.cpp
double calculate_rectangle_area(double length, double width) {
    return length * width;
}

double calculate_rectangle_perimeter(double length, double width = 0.0) {
    if (width == 0.0) width = length;
    return 2 * (length + width);
}

std::string describe_person(const std::string& name, int age) {
    return name + " (" + std::to_string(age) + ")";
}

int main() {
    // Positional arguments
    assert(calculate_rectangle_area(5.0, 3.0) == 15.0);

    // Default arguments
    assert(calculate_rectangle_perimeter(10.0) == 40.0);      // square
    assert(calculate_rectangle_perimeter(10.0, 5.0) == 30.0); // rectangle

    // Overloading simulation
    assert(describe_person("Alice", 25) == "Alice (25)");

    std::cout << "Day 18 positional & simulated keyword arguments tests passed.\n";
    std::cout << "(Run main.cpp for interactive demonstration)\n";

    return 0;
}