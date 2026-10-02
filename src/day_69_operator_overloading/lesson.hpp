/**
 * @file
 * Day 69 – Operator Overloading.
 *
 * Scenario: a *recipe scaler* for a bakery. Quantities such as "1 1/2 cups" or "3/4 tsp" must be
 * scaled by 2/3 or 5/2 without rounding errors, so they are stored as exact fractions. With
 * overloaded operators the fraction type reads like a built-in number: arithmetic, comparisons
 * with the spaceship operator, and reading and writing with streams.
 *
 * Deliverables (syllabus):
 * - Arithmetic and comparison operators
 * - The spaceship operator
 * - Stream operators
 * - Class invariants
 */
#pragma once

#include <compare>
#include <cstdint>
#include <cstdlib>
#include <istream>
#include <numeric>
#include <ostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

#include "cppm/lesson.hpp"

namespace cppm::day69 {

inline constexpr Deliverable DELIVERABLES[] = {
    {"a fraction type whose invariant every operation keeps", "Rational"},
    {"compound assignment as the basis for binary operators", "Rational::operator+="},
    {"three-way comparison by cross-multiplication", "operator<=>"},
    {"writing mixed numbers to a stream", "operator<<"},
    {"reading \"1 1/2\" or \"3/4\" from a stream", "operator>>"},
};

/// An exact fraction. Invariant: the denominator is positive and gcd(numerator, denominator) == 1,
/// so equal values always have equal members and == can be defaulted.
class Rational {
  public:
    constexpr Rational(std::int64_t whole = 0) : num_(whole), den_(1) {}  // implicit: 2 means 2/1
    constexpr Rational(std::int64_t num, std::int64_t den) : num_(num), den_(den) {
        if (den == 0) throw std::invalid_argument("denominator must not be zero");
        normalise();
    }
    constexpr std::int64_t num() const { return num_; }
    constexpr std::int64_t den() const { return den_; }

    constexpr Rational& operator+=(const Rational& o) {
        *this = Rational(num_ * o.den_ + o.num_ * den_, den_ * o.den_);
        return *this;
    }
    constexpr Rational& operator-=(const Rational& o) { return *this += -o; }
    constexpr Rational& operator*=(const Rational& o) {
        *this = Rational(num_ * o.num_, den_ * o.den_);
        return *this;
    }
    constexpr Rational& operator/=(const Rational& o) {
        if (o.num_ == 0) throw std::domain_error("division by zero");
        *this = Rational(num_ * o.den_, den_ * o.num_);
        return *this;
    }
    constexpr Rational operator-() const { return Rational(-num_, den_); }

    // Binary operators are non-members (hidden friends), so 2 * r works as well as r * 2.
    friend constexpr Rational operator+(Rational a, const Rational& b) { return a += b; }
    friend constexpr Rational operator-(Rational a, const Rational& b) { return a -= b; }
    friend constexpr Rational operator*(Rational a, const Rational& b) { return a *= b; }
    friend constexpr Rational operator/(Rational a, const Rational& b) { return a /= b; }

    /// Members are canonical, so member-wise equality is value equality.
    friend constexpr bool operator==(const Rational&, const Rational&) = default;
    /// a/b <=> c/d is a*d <=> c*b, because both denominators are positive.
    friend constexpr std::strong_ordering operator<=>(const Rational& a, const Rational& b) {
        return a.num_ * b.den_ <=> b.num_ * a.den_;
    }

    /// Conversions that lose precision are explicit.
    explicit constexpr operator double() const { return static_cast<double>(num_) / static_cast<double>(den_); }

  private:
    constexpr void normalise() {
        if (den_ < 0) {
            num_ = -num_;
            den_ = -den_;
        }
        const std::int64_t g = std::gcd(num_, den_);
        if (g > 1) {
            num_ /= g;
            den_ /= g;
        }
    }
    std::int64_t num_;
    std::int64_t den_;
};

/// Mixed-number output: 3/2 -> "1 1/2", -7/4 -> "-1 3/4", 4/2 -> "2".
inline std::ostream& operator<<(std::ostream& out, const Rational& r) {
    const std::int64_t whole = r.num() / r.den();
    const std::int64_t rest = std::llabs(r.num() % r.den());
    if (rest == 0) return out << whole;
    if (whole == 0) return out << r.num() << '/' << r.den();
    return out << whole << ' ' << rest << '/' << r.den();
}

/// Reads "2", "3/4" or "1 1/2". On bad input the stream's failbit is set and @p r is unchanged,
/// as with built-in types.
inline std::istream& operator>>(std::istream& in, Rational& r) {
    std::int64_t first = 0;
    if (!(in >> first)) return in;
    if (in.peek() == '/') {  // "3/4"
        in.get();
        std::int64_t den = 0;
        if (!(in >> den) || den <= 0) {
            in.setstate(std::ios::failbit);
            return in;
        }
        r = Rational(first, den);
        return in;
    }
    // Perhaps "1 1/2": try to read a proper fraction, and roll back to just after the whole
    // number if what follows is something else (e.g. "2 cups").
    const auto mark = in.tellg();
    std::int64_t num = 0;
    std::int64_t den = 0;
    char slash = 0;
    if (in >> num && in.get(slash) && slash == '/' && in >> den && den > 0 && num >= 0 && num < den) {
        const Rational part(num, den);
        r = Rational(first) + (first < 0 ? -part : part);
        return in;
    }
    in.clear();
    if (mark != std::istream::pos_type(-1)) in.seekg(mark);
    r = Rational(first);
    return in;
}

struct Ingredient {
    Rational amount;
    std::string unit;
    std::string name;
};

inline std::vector<Ingredient> scale(std::vector<Ingredient> recipe, const Rational& factor) {
    for (auto& item : recipe) item.amount *= factor;
    return recipe;
}

/// The interactive demo: enter a factor such as "2/3" or "1 1/2"; the shortbread recipe is rescaled.
inline int run(std::istream& in, std::ostream& out) {
    out << "Day 69 – Operator Overloading\n";
    const std::vector<Ingredient> shortbread{{Rational(3, 2), "cups", "flour"},
                                             {Rational(3, 4), "cup", "butter"},
                                             {Rational(1, 3), "cup", "sugar"},
                                             {Rational(1, 4), "tsp", "salt"}};
    while (auto line = prompt_line(in, out, "scale by> ")) {
        std::istringstream words(*line);
        Rational factor;
        if (!(words >> factor)) break;
        if (factor <= Rational(0)) {
            out << "  the factor must be positive\n";
            continue;
        }
        for (const auto& item : scale(shortbread, factor))
            out << "  " << item.amount << ' ' << item.unit << ' ' << item.name << '\n';
    }
    return 0;
}

}  // namespace cppm::day69
