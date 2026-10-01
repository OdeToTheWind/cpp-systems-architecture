/**
 * @file
 * Day 48 – Static vs Dynamic Typing.
 *
 * Scenario: a *spreadsheet cell engine*. A cell may hold nothing, a number, text, a boolean or
 * an error – decided at run time from what the user typed – yet every operation on cells is
 * checked at compile time through std::variant and std::visit. Units are given their own
 * types so metres and feet can never be added by accident, and a plugin metadata bag shows
 * where std::any fits.
 *
 * Deliverables (syllabus):
 * - Static type checking
 * - std::variant and std::visit
 * - std::any
 * - Type-safe heterogeneous data
 */
#pragma once

#include <any>
#include <cmath>
#include <iomanip>
#include <istream>
#include <map>
#include <ostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <variant>
#include <vector>

#include "cppm/lesson.hpp"

namespace cppm::day48 {

inline constexpr Deliverable DELIVERABLES[] = {
    {"strong types that the compiler keeps apart", "Length"},
    {"a closed set of run-time types: std::variant", "Cell"},
    {"deciding the type from text at run time", "parse_cell"},
    {"std::visit with an overload set", "display"},
    {"type-safe aggregation over mixed cells", "sum_numbers"},
    {"an open set of types: std::any and any_cast", "PluginInfo"},
};

/// A length that remembers its unit in its *type*: Length<Metres> + Length<Feet> does not compile.
template <typename Unit>
struct Length {
    double value;
    constexpr Length operator+(Length other) const { return {value + other.value}; }
    constexpr bool operator==(const Length&) const = default;
};
struct Metres {};
struct Feet {};

/// Conversions are explicit functions, never implicit.
constexpr Length<Metres> to_metres(Length<Feet> feet) { return {feet.value * 0.3048}; }

/// A spreadsheet error such as #DIV/0!.
struct CellError {
    std::string code;
    bool operator==(const CellError&) const = default;
};

/// Exactly one of these alternatives at any time; std::monostate means "empty cell".
using Cell = std::variant<std::monostate, double, bool, std::string, CellError>;

/// What the user typed decides the alternative: "", "42", "TRUE", "#N/A", "=1/0" or text.
inline Cell parse_cell(const std::string& text) {
    if (text.empty()) return std::monostate{};
    if (text == "TRUE" || text == "FALSE") return text == "TRUE";
    if (text.front() == '#') return CellError{text};
    if (text.front() == '=') {  // tiny formula support: =a/b
        std::istringstream formula(text.substr(1));
        double a = 0;
        double b = 0;
        char op{};
        if (formula >> a >> op >> b && op == '/') {
            return b == 0 ? Cell{CellError{"#DIV/0!"}} : Cell{a / b};
        }
        return CellError{"#NAME?"};
    }
    std::istringstream number(text);
    double value = 0;
    if (number >> value && (number >> std::ws).eof()) return value;
    return text;
}

/// Helper so std::visit can take several lambdas, one per alternative.
template <typename... Lambdas>
struct overloaded : Lambdas... {
    using Lambdas::operator()...;
};
template <typename... Lambdas>
overloaded(Lambdas...) -> overloaded<Lambdas...>;

/// The compiler checks that every alternative is handled – leaving one out is a compile error.
inline std::string display(const Cell& cell) {
    return std::visit(overloaded{
                          [](std::monostate) { return std::string(); },
                          [](double d) {
                              std::ostringstream out;
                              out << d;
                              return out.str();
                          },
                          [](bool b) { return std::string(b ? "TRUE" : "FALSE"); },
                          [](const std::string& s) { return s; },
                          [](const CellError& e) { return e.code; },
                      },
                      cell);
}

/// Sum the numeric cells; text and empty cells are skipped, but an error anywhere makes the sum an error.
inline Cell sum_numbers(const std::vector<Cell>& column) {
    double total = 0;
    for (const auto& cell : column) {
        if (const auto* error = std::get_if<CellError>(&cell)) {
            return *error;  // errors propagate, as in real spreadsheets
        }
        if (const auto* number = std::get_if<double>(&cell)) {
            total += *number;
        }
    }
    return total;
}

/// Metadata from plugins written by other people: any value type, checked only when read.
class PluginInfo {
public:
    void set(const std::string& key, std::any value) { values_[key] = std::move(value); }
    /// Typed read: throws std::bad_any_cast if the stored type is not exactly T.
    template <typename T>
    T get(const std::string& key) const {
        const auto it = values_.find(key);
        if (it == values_.end()) throw std::out_of_range("no key " + key);
        return std::any_cast<T>(it->second);
    }
    /// Non-throwing read: nullptr when the key is missing or the type differs.
    template <typename T>
    const T* try_get(const std::string& key) const {
        const auto it = values_.find(key);
        return it == values_.end() ? nullptr : std::any_cast<T>(&it->second);
    }

private:
    std::map<std::string, std::any> values_;
};

/// The interactive demo: type cell contents; each is classified and the numeric sum is shown.
inline int run(std::istream& in, std::ostream& out) {
    out << "Day 48 – Static vs Dynamic Typing\n";
    constexpr Length<Metres> run_track = Length<Metres>{400} + to_metres(Length<Feet>{1000});
    out << "track + 1000 ft = " << run_track.value << " m\n";
    std::vector<Cell> column;
    static const char* names[] = {"empty", "number", "boolean", "text", "error"};
    while (auto line = prompt_line(in, out, "cell> ")) {
        if (*line == "END") break;
        column.push_back(parse_cell(*line));
        out << "  " << names[column.back().index()] << ": '" << display(column.back()) << "'\n";
    }
    out << "SUM = " << display(sum_numbers(column)) << '\n';
    return 0;
}

}  // namespace cppm::day48
