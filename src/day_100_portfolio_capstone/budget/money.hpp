// budget/money.hpp – amounts as integer cents (Days 6, 88).
#pragma once

#include <cstdint>
#include <stdexcept>
#include <string>

namespace cppm::day100 {

using Cents = std::int64_t;

/// "12", "12.5", "-3.99" -> cents. At most two decimals; anything else is an error.
inline Cents parse_money(const std::string& text) {
    std::string t = text;
    const bool negative = !t.empty() && t[0] == '-';
    if (negative) t.erase(0, 1);
    const auto dot = t.find('.');
    const std::string whole = t.substr(0, dot);
    std::string frac = dot == std::string::npos ? "" : t.substr(dot + 1);
    if (whole.empty() || whole.size() > 12 || whole.find_first_not_of("0123456789") != std::string::npos || frac.size() > 2 ||
        frac.find_first_not_of("0123456789") != std::string::npos || (dot != std::string::npos && frac.empty())) {
        throw std::invalid_argument("not an amount: '" + text + "'");
    }
    frac.resize(2, '0');
    const Cents value = std::stoll(whole) * 100 + std::stoll(frac);
    return negative ? -value : value;
}

inline std::string format_money(Cents cents, const std::string& symbol) {
    const Cents abs = cents < 0 ? -cents : cents;
    std::string whole = std::to_string(abs / 100);
    for (auto i = static_cast<std::ptrdiff_t>(whole.size()) - 3; i > 0; i -= 3) whole.insert(static_cast<std::size_t>(i), ",");
    return (cents < 0 ? "-" : "") + symbol + whole + "." + (abs % 100 < 10 ? "0" : "") + std::to_string(abs % 100);
}

}  // namespace cppm::day100
