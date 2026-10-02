/**
 * @file
 * Day 65 – Concepts & Constraints.
 *
 * Scenario: a *dashboard widget library*. A widget shows a value or a series of values – an
 * order count, a CPU percentage, a sequence of response times, or a domain object such as a
 * server that knows its own label. Concepts state exactly which types each widget accepts, so a
 * wrong type is rejected with a readable message, and constrained overloads pick the right
 * formatting automatically.
 *
 * Deliverables (syllabus):
 * - Standard concepts
 * - Writing custom concepts
 * - requires clauses
 * - Constrained overloads
 */
#pragma once

#include <algorithm>
#include <concepts>
#include <cstddef>
#include <iomanip>
#include <istream>
#include <iterator>
#include <ostream>
#include <ranges>
#include <sstream>
#include <string>
#include <vector>

#include "cppm/lesson.hpp"

namespace cppm::day65 {

inline constexpr Deliverable DELIVERABLES[] = {
    {"a function constrained with a standard concept", "clamp_to"},
    {"custom concepts built from requires expressions", "Labelled"},
    {"a concept for a numeric series", "Series"},
    {"overloads chosen by the most specific constraint", "format_value"},
    {"a constrained widget that renders any series", "sparkline"},
};

/// std::totally_ordered: <, >, <=, >= and == all exist and agree.
template <std::totally_ordered T>
constexpr T clamp_to(const T& value, const T& low, const T& high) {
    return value < low ? low : high < value ? high : value;
}

/// Numbers, but not bool or characters, which are integral yet never a metric.
template <typename T>
concept Numeric = (std::integral<T> || std::floating_point<T>) && !std::same_as<T, bool> && !std::same_as<T, char>;

/// Anything that can name itself: has a label() convertible to std::string.
template <typename T>
concept Labelled = requires(const T& t) {
    { t.label() } -> std::convertible_to<std::string>;
};

/// A sized range of numbers: vector<int>, array<double, N>, a view…
template <typename R>
concept Series = std::ranges::sized_range<R> && Numeric<std::ranges::range_value_t<R>>;

// Constrained overloads: for an unsigned integer both of the first two match, and the
// compiler chooses the more constrained one (std::unsigned_integral subsumes std::integral).
template <std::integral T>
    requires Numeric<T>
std::string format_value(T value) {
    std::string digits = std::to_string(value);
    const bool negative = digits.front() == '-';
    if (negative) digits.erase(0, 1);
    for (auto i = static_cast<std::ptrdiff_t>(digits.size()) - 3; i > 0; i -= 3) digits.insert(static_cast<std::size_t>(i), ",");
    return (negative ? "-" : "") + digits;
}

template <std::unsigned_integral T>
    requires Numeric<T>
std::string format_value(T value) {
    return format_value(static_cast<long long>(value)) + " (count)";
}

template <std::floating_point T>
std::string format_value(T value) {
    std::ostringstream out;
    out << std::fixed << std::setprecision(1) << value;
    return out.str();
}

template <Labelled T>
std::string format_value(const T& item) {
    return "[" + std::string(item.label()) + "]";
}

/// A one-line chart of a series using eight bar heights; constant series draw mid-height bars.
template <Series R>
std::string sparkline(const R& values) {
    static const char* const bars[] = {"▁", "▂", "▃", "▄", "▅", "▆", "▇", "█"};
    if (std::ranges::empty(values)) return "";
    const auto [lo, hi] = std::ranges::minmax(values);
    std::string out;
    for (const auto& v : values) {
        const double span = static_cast<double>(hi) - static_cast<double>(lo);
        const auto level = span == 0 ? 3 : static_cast<int>((static_cast<double>(v) - static_cast<double>(lo)) / span * 7.0 + 0.5);
        out += bars[clamp_to(level, 0, 7)];
    }
    return out;
}

/// The average of a series, in double, whatever its element type.
template <typename R>
    requires Series<R>
double average(const R& values) {
    double total = 0;
    for (const auto& v : values) total += static_cast<double>(v);
    return std::ranges::empty(values) ? 0.0 : total / static_cast<double>(std::ranges::size(values));
}

/// Example domain type that satisfies Labelled.
struct Server {
    std::string host;
    int region;
    std::string label() const { return host + "@" + std::to_string(region); }
};

/// The interactive demo: type response times in ms on one line; a sparkline and summary are drawn.
inline int run(std::istream& in, std::ostream& out) {
    out << "Day 65 – Concepts & Constraints\n";
    out << "orders today: " << format_value(1234567) << ", queue: " << format_value(42u) << ", cpu: " << format_value(73.44)
        << "%, host: " << format_value(Server{"web-1", 3}) << '\n';
    while (auto line = prompt_line(in, out, "response times (ms)> ")) {
        std::istringstream words(*line);
        const std::vector<int> times{std::istream_iterator<int>(words), std::istream_iterator<int>()};
        if (times.empty()) break;
        out << "  " << sparkline(times) << "  avg " << format_value(average(times)) << " ms, max "
            << format_value(std::ranges::max(times)) << " ms\n";
    }
    return 0;
}

}  // namespace cppm::day65
