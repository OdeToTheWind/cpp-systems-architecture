/**
 * @file
 * Day 05 – Mathematical Operations.
 *
 * Scenario: a *restaurant bill splitter* that works in whole cents: it applies a discount
 * before tax, splits the total fairly between diners, and refuses calculations that would
 * overflow or divide by zero.
 *
 * Deliverables (syllabus):
 * - Arithmetic and compound assignment
 * - Operator precedence
 * - Integer vs floating division
 * - Overflow-safe arithmetic
 * - cmath functions
 */
#pragma once

#include <cmath>
#include <cstdint>
#include <iomanip>
#include <istream>
#include <limits>
#include <numbers>
#include <optional>
#include <ostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include "cppm/lesson.hpp"

namespace cppm::day05 {

inline constexpr Deliverable DELIVERABLES[] = {
    {"compound assignment (-=, *=, /=, %=) on money", "apply_discount_and_tax"},
    {"operator precedence and associativity", "precedence_examples"},
    {"integer division truncates, floor division rounds down", "division_facts"},
    {"overflow-safe multiplication", "checked_multiply"},
    {"distributing a remainder fairly", "split_bill"},
    {"cmath: pow, sqrt, hypot and std::numbers::pi", "circle_stats"},
};

/// Expressions whose value changes once parentheses are added.
inline std::vector<std::pair<std::string, long long>> precedence_examples() {
    return {
        {"2 + 3 * 4", 2 + 3 * 4},
        {"(2 + 3) * 4", (2 + 3) * 4},
        {"10 - 4 - 3", 10 - 4 - 3},      // left-associative: (10 - 4) - 3
        {"100 / 10 / 5", 100 / 10 / 5},  // (100 / 10) / 5
        {"7 % 3 * 2", 7 % 3 * 2},        // % and * share a precedence level
        {"-7 / 2", -7 / 2},              // truncates toward zero
    };
}

/// What C++ integer division does, compared with floor division and real division.
struct DivisionFacts {
    long long quotient;        // a / b, truncated toward zero
    long long remainder;       // a % b, takes the sign of a
    long long floor_quotient;  // rounded toward minus infinity
    double real;               // exact-ish floating-point result
    bool identity_holds;       // a == b * quotient + remainder
};

/// Integer and floating division side by side. Throws std::domain_error for b == 0.
inline DivisionFacts division_facts(long long a, long long b) {
    if (b == 0) {
        throw std::domain_error("division by zero");
    }
    if (a == std::numeric_limits<long long>::min() && b == -1) {
        throw std::overflow_error("LLONG_MIN / -1 does not fit in long long");
    }
    const long long q = a / b;
    const long long r = a % b;
    const long long floor_q = (r != 0 && ((r < 0) != (b < 0))) ? q - 1 : q;
    return {q, r, floor_q, static_cast<double>(a) / static_cast<double>(b), a == b * q + r};
}

/// a * b, or std::nullopt if the product does not fit in a long long.
inline std::optional<long long> checked_multiply(long long a, long long b) {
    using limits = std::numeric_limits<long long>;
    if (a == 0 || b == 0) {
        return 0;
    }
    if (a > 0 ? (b > 0 ? a > limits::max() / b : b < limits::min() / a)
              : (b > 0 ? a < limits::min() / b : a != 0 && b < limits::max() / a)) {
        return std::nullopt;
    }
    return a * b;
}

/// Subtract the discount first, then add tax (rounded half up), all in cents.
inline long long apply_discount_and_tax(long long subtotal_cents, long long discount_cents, int tax_percent) {
    if (subtotal_cents < 0 || discount_cents < 0 || tax_percent < 0) {
        throw std::invalid_argument("amounts and tax must not be negative");
    }
    long long total = subtotal_cents;
    total -= discount_cents;  // compound subtraction: the discount comes off first
    if (total < 0) {
        total = 0;
    }
    const auto taxed = checked_multiply(total, 100 + tax_percent);
    if (!taxed) {
        throw std::overflow_error("bill too large");
    }
    total = *taxed;
    total += 50;   // round half up …
    total /= 100;  // … when dividing back to cents
    return total;
}

/// Split @p total_cents between @p people; the first `total % people` diners pay one cent more.
inline std::vector<long long> split_bill(long long total_cents, int people) {
    if (people <= 0) {
        throw std::invalid_argument("at least one person must pay");
    }
    if (total_cents < 0) {
        throw std::invalid_argument("the bill cannot be negative");
    }
    const long long share = total_cents / people;
    long long leftover = total_cents % people;
    std::vector<long long> shares(static_cast<std::size_t>(people), share);
    for (auto& s : shares) {
        if (leftover == 0) {
            break;
        }
        s += 1;
        --leftover;
    }
    return shares;
}

/// Plate geometry for the menu designer: area, circumference and the diagonal of a square tray.
struct CircleStats {
    double area;
    double circumference;
    double tray_diagonal;  // a square tray just big enough for the plate
};

/// Uses std::numbers::pi, std::pow, std::sqrt and std::hypot. Throws for non-positive radii.
inline CircleStats circle_stats(double radius_cm) {
    if (!(radius_cm > 0.0) || !std::isfinite(radius_cm)) {
        throw std::domain_error("radius must be a positive finite number");
    }
    const double side = 2.0 * radius_cm;
    return {std::numbers::pi * std::pow(radius_cm, 2), 2.0 * std::numbers::pi * radius_cm,
            std::hypot(side, side)};
}

/// Format cents as "12.34".
inline std::string money(long long cents) {
    std::ostringstream out;
    out << (cents < 0 ? "-" : "") << std::llabs(cents) / 100 << '.' << std::setw(2) << std::setfill('0')
        << std::llabs(cents) % 100;
    return out.str();
}

/// The interactive demo: subtotal, discount, tax and diners in, fair shares out.
inline int run(std::istream& in, std::ostream& out) {
    out << "Day 05 – Mathematical Operations\n";
    for (const auto& [expression, value] : precedence_examples()) {
        out << "  " << std::left << std::setw(14) << expression << "= " << value << '\n';
    }
    const auto ask = [&](const char* prompt) -> std::optional<long long> {
        while (auto line = prompt_line(in, out, prompt)) {
            std::istringstream number(*line);
            long long value{};
            if (number >> value && value >= 0 && (number >> std::ws).eof()) {
                return value;
            }
            out << "  please type a whole, non-negative number\n";
        }
        return std::nullopt;
    };
    const auto subtotal = ask("Subtotal in cents: ");
    const auto discount = subtotal ? ask("Discount in cents: ") : std::nullopt;
    const auto tax = discount ? ask("Tax percent: ") : std::nullopt;
    const auto people = tax ? ask("Number of diners: ") : std::nullopt;
    if (!people) {
        out << "Bill cancelled.\n";
        return 0;
    }
    try {
        const long long total = apply_discount_and_tax(*subtotal, *discount, static_cast<int>(*tax));
        out << "Total: " << money(total) << '\n';
        int diner = 1;
        for (long long share : split_bill(total, static_cast<int>(*people))) {
            out << "  diner " << diner++ << " pays " << money(share) << '\n';
        }
    } catch (const std::exception& error) {
        out << "Cannot split this bill: " << error.what() << '\n';
    }
    return 0;
}

}  // namespace cppm::day05
