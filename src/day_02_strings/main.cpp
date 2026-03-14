#include <iostream>
#include <string>
#include "string_utils.hpp"   // ← new

int main() {
    std::string text;

    std::cout << "=== Day 02: std::string Manipulation ===\n\n";
    std::cout << "Enter a sentence or word: ";
    std::getline(std::cin, text);

    if (text.empty()) {
        std::cout << "No input provided.\n";
        return 0;
    }

    std::cout << "Original         : " << text << "\n";
    std::cout << "Length           : " << text.length() << "\n";
    std::cout << "First character  : '" << text.front() << "'\n";
    std::cout << "Last character   : '" << text.back() << "'\n";

    size_t spacePos = text.find(' ');
    if (spacePos != std::string::npos) {
        std::cout << "First word       : " << text.substr(0, spacePos) << "\n";
    } else {
        std::cout << "First word       : (whole input is one word)\n";
    }

    std::cout << "Is palindrome?   : " << (isPalindrome(text) ? "YES" : "NO") << "\n";

    std::string mutated = text;
    for (char& c : mutated) {
        if (c == 'a' || c == 'A') c = '@';
    }
    std::cout << "After a→@ replace : " << mutated << "\n";

    return 0;
}