// tests/test_day_02.cpp
#include <cassert>
#include <string>
#include "../src/day_02_strings/main.cpp"   // include the source (for isPalindrome)

// Only the testable function is exposed — in real projects we'd move it to .hpp
// For learning phase this is acceptable

int main() {
    // Test 1: Basic palindrome
    assert(isPalindrome("radar") == true);
    assert(isPalindrome("A man a plan a canal Panama") == true);

    // Test 2: Case insensitive + non-letters ignored
    assert(isPalindrome("Able was I ere I saw Elba") == true);
    assert(isPalindrome("No 'x' in Nixon") == true);

    // Test 3: Not palindrome
    assert(isPalindrome("hello") == false);
    assert(isPalindrome("A man a plan a canal") == false);

    // Test 4: Empty / single char / only non-letters
    assert(isPalindrome("") == true);                // empty is palindrome by convention
    assert(isPalindrome("a") == true);
    assert(isPalindrome("!!! 123 !!!") == true);     // no letters → palindrome
    assert(isPalindrome("ab ba") == true);           // spaces ignored

    // Test 5: Mutation example (we test logic manually here)
    std::string s = "banana Apple";
    std::string expected = "b@n@n@ @pple";
    for (char& c : s) {
        if (c == 'a' || c == 'A') c = '@';
    }
    assert(s == expected);

    std::cout << "All Day 02 string tests passed!\n";
    return 0;
}