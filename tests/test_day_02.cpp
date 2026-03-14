// tests/test_day_02.cpp
#include <cassert>
#include <string>
#include <iostream>

#include "../src/day_02_strings/string_utils.hpp"

int main() {
    // Test 1: Classic palindromes
    assert(isPalindrome("radar") == true);
    assert(isPalindrome("A man a plan a canal Panama") == true);
    assert(isPalindrome("Able was I ere I saw Elba") == true);

    // Test 2: Edge cases
    assert(isPalindrome("") == true);                    // empty → true (convention)
    assert(isPalindrome("a") == true);
    assert(isPalindrome("!!! 123 !!!") == true);         // no letters → true
    assert(isPalindrome("ab ba") == true);               // spaces ignored

    // Test 3: Negative cases
    assert(isPalindrome("hello") == false);
    assert(isPalindrome("A man a plan a canal") == false);
    assert(isPalindrome("race a car") == false);

    std::cout << "All Day 02 string tests passed!\n";
    return 0;
}