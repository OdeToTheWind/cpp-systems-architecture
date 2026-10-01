/**
 * @file
 * Day 46 – Variadic Templates.
 *
 * Scenario: a *structured logging and metrics helper for a game server*. One `log` call takes
 * any number of key/value pairs of any types, a `sum_all` adds whatever numbers it is given,
 * tuples of coordinates are unpacked straight into function calls, and a factory forwards its
 * arguments perfectly to whatever object it creates.
 *
 * Deliverables (syllabus):
 * - Parameter packs
 * - Fold expressions
 * - Perfect forwarding
 * - std::tuple and std::apply
 */
#pragma once

#include <cmath>
#include <cstddef>
#include <istream>
#include <memory>
#include <ostream>
#include <sstream>
#include <string>
#include <tuple>
#include <type_traits>
#include <utility>
#include <vector>

#include "cppm/lesson.hpp"

namespace cppm::day46 {

inline constexpr Deliverable DELIVERABLES[] = {
    {"a parameter pack and sizeof...", "count_args"},
    {"a fold expression over +", "sum_all"},
    {"a fold over the comma operator to print key/value pairs", "format_fields"},
    {"perfect forwarding with std::forward", "make_tracked"},
    {"std::apply unpacks a tuple into a call", "distance_from_origin"},
    {"returning several values as a tuple", "min_max_mean"},
};

/// How many arguments were passed – known at compile time.
template <typename... Args>
constexpr std::size_t count_args(const Args&...) {
    return sizeof...(Args);
}

/// (... + values) expands to ((v1 + v2) + v3) + …; an empty pack yields 0 thanks to the init value.
template <typename... Numbers>
constexpr auto sum_all(Numbers... values) {
    static_assert((std::is_arithmetic_v<Numbers> && ...), "sum_all only adds numbers");
    return (0 + ... + values);
}

/// key=value pairs: format_fields("player", "ada", "hp", 42) -> "player=ada hp=42".
template <typename... Fields>
std::string format_fields(const Fields&... fields) {
    static_assert(sizeof...(Fields) % 2 == 0, "fields come in key/value pairs");
    std::ostringstream out;
    std::size_t index = 0;
    // a fold over the comma operator runs the lambda once per argument, left to right
    ((out << (index == 0 ? "" : (index % 2 == 0 ? " " : "=")) << fields, ++index), ...);
    static_cast<void>(index);  // with an empty pack the fold expands to nothing
    return out.str();
}

/// One structured log line with a level and any number of fields.
template <typename... Fields>
std::string log_line(const std::string& level, const std::string& message, const Fields&... fields) {
    std::string line = "[" + level + "] " + message;
    if constexpr (sizeof...(Fields) > 0) {
        line += " | " + format_fields(fields...);
    }
    return line;
}

/// Records whether a constructor argument arrived as an lvalue (copied) or an rvalue (moved).
struct Tracked {
    std::string name;
    bool moved_in;
    Tracked(const std::string& n, int) : name(n), moved_in(false) {}
    Tracked(std::string&& n, int) : name(std::move(n)), moved_in(true) {}
};

/// std::forward keeps each argument's value category: lvalues stay lvalues, temporaries are moved.
template <typename T, typename... Args>
std::unique_ptr<T> make_tracked(Args&&... args) {
    return std::make_unique<T>(std::forward<Args>(args)...);
}

inline double distance(double x, double y, double z) { return std::sqrt(x * x + y * y + z * z); }

/// std::apply calls a function with the tuple's elements as separate arguments.
inline double distance_from_origin(const std::tuple<double, double, double>& position) {
    return std::apply(distance, position);
}

/// Three results at once; callers unpack them with structured bindings.
template <typename... Values>
std::tuple<double, double, double> min_max_mean(Values... values) {
    static_assert(sizeof...(Values) > 0, "need at least one value");
    const double all[] = {static_cast<double>(values)...};  // pack expansion inside an initialiser list
    double low = all[0];
    double high = all[0];
    for (double v : all) {
        low = v < low ? v : low;
        high = v > high ? v : high;
    }
    return {low, high, sum_all(static_cast<double>(values)...) / static_cast<double>(sizeof...(values))};
}

/// The interactive demo: type "x y z" positions; each is logged with variadic helpers.
inline int run(std::istream& in, std::ostream& out) {
    out << "Day 46 – Variadic Templates\n";
    out << log_line("INFO", "server started", "port", 7777, "tick_ms", 16, "debug", false) << '\n';
    int players = 0;
    while (auto line = prompt_line(in, out, "player position x y z> ")) {
        std::istringstream words(*line);
        double x = 0;
        double y = 0;
        double z = 0;
        if (!(words >> x >> y >> z)) {
            break;
        }
        ++players;
        const auto position = std::make_tuple(x, y, z);
        out << log_line("INFO", "player joined", "id", players, "distance", distance_from_origin(position)) << '\n';
    }
    const auto [low, high, mean] = min_max_mean(3, 9.5, 4);
    out << "stats over (3, 9.5, 4): min " << low << ", max " << high << ", mean " << mean << '\n';
    out << players << " player(s) joined; " << count_args("a", 1, 2.0) << " arguments in the last count\n";
    return 0;
}

}  // namespace cppm::day46
