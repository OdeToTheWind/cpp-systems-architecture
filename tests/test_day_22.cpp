// tests/test_day_22.cpp
#include <cassert>
#include <string>
#include <iostream>

std::string greet_user(const std::string& name, int age = 0) {
    if (name.empty()) return "Hello, stranger!";
    if (age > 0) return "Hello, " + name + "! You are " + std::to_string(age) + " years old.";
    return "Hello, " + name + "!";
}

int main() {
    assert(greet_user("") == "Hello, stranger!");
    assert(greet_user("Alice", 25) == "Hello, Alice! You are 25 years old.");
    assert(greet_user("Bob") == "Hello, Bob!");

    std::cout << "Day 22 documentation tests passed.\n";
    return 0;
}