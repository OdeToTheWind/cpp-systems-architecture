/**
 * @file
 * Day 12 – Functions.
 *
 * Scenario: a *bakery order counter*. Small, documented functions price each pastry by
 * size, build up an order, apply a loyalty-card discount and print the receipt. This header
 * holds only the declarations; their definitions live in lesson.cpp.
 *
 * Deliverables (syllabus):
 * - Declarations vs definitions
 * - Parameters and return values
 * - Pass by value and const reference
 * - Overloading
 * - Default arguments
 */
#pragma once

#include <istream>
#include <ostream>
#include <string>
#include <vector>

#include "cppm/lesson.hpp"

namespace cppm::day12 {

inline constexpr Deliverable DELIVERABLES[] = {
    {"a declaration with a default argument (definition in lesson.cpp)", "unit_price"},
    {"pass by reference to modify the caller's object", "add_item"},
    {"pass by const reference to read without copying", "order_total"},
    {"pass by value for a cheap, independent copy", "apply_loyalty"},
    {"overloading on the parameter list", "format_price"},
};

enum class Size { small, medium, large };

/// One line of an order.
struct OrderLine {
    std::string item;
    int quantity;
    Size size;
    long long line_total_cents;
};

/// A customer's order.
struct Order {
    std::vector<OrderLine> lines;
};

/// Price in cents of one @p item in @p size; throws std::invalid_argument for unknown items.
/// The default argument appears only here, in the declaration – never repeated in the definition.
long long unit_price(const std::string& item, Size size = Size::medium);

/// Append @p quantity of @p item to @p order (modified through the reference); returns the line total.
long long add_item(Order& order, const std::string& item, int quantity, Size size = Size::medium);

/// Sum of all lines. `const Order&` avoids copying the vector and promises not to change it.
long long order_total(const Order& order);

/// Every 10 stamps on a loyalty card take 10 % off; @p total is a copy, so the caller's value is untouched.
long long apply_loyalty(long long total, int stamps);

/// "12.50" – overload 1.
std::string format_price(long long cents);
/// "12.50 EUR" – overload 2: same name, different parameter list.
std::string format_price(long long cents, const std::string& currency);
/// "12.50 EUR" from a whole order – overload 3.
std::string format_price(const Order& order, const std::string& currency = "EUR");

/// Parse "small", "medium" or "large" (default medium).
Size parse_size(const std::string& word);

/// The interactive demo: "<quantity> <item> [size]" lines, then the loyalty stamps.
int run(std::istream& in, std::ostream& out);

}  // namespace cppm::day12
