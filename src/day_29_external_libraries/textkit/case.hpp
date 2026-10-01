// textkit/case.hpp – the HEADER-ONLY part of textkit: every function is `inline`, nothing to link.
#pragma once

#include <cctype>
#include <string>
#include <string_view>

namespace textkit {

/// "hello world" -> "Hello World".
inline std::string title_case(std::string_view text) {
    std::string result(text);
    bool start = true;
    for (char& c : result) {
        const auto u = static_cast<unsigned char>(c);
        c = static_cast<char>(start ? std::toupper(u) : std::tolower(u));
        start = std::isspace(u) != 0;
    }
    return result;
}

/// "Hello, World!" -> "hello-world".
inline std::string slug(std::string_view text) {
    std::string result;
    for (const char c : text) {
        const auto u = static_cast<unsigned char>(c);
        if (std::isalnum(u)) {
            result += static_cast<char>(std::tolower(u));
        } else if (!result.empty() && result.back() != '-') {
            result += '-';
        }
    }
    if (!result.empty() && result.back() == '-') {
        result.pop_back();
    }
    return result;
}

}  // namespace textkit
