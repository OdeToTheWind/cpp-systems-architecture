#ifndef DAY_02_STRING_UTILS_HPP
#define DAY_02_STRING_UTILS_HPP

#include <string>
#include <cctype>
#include <algorithm>

bool isPalindrome(const std::string& s) {
    std::string cleaned;
    cleaned.reserve(s.size());

    for (char c : s) {
        if (std::isalpha(static_cast<unsigned char>(c))) {
            cleaned += static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
        }
    }

    std::string rev = cleaned;
    std::reverse(rev.begin(), rev.end());
    return cleaned == rev;
}

#endif