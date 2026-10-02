/**
 * @file
 * Day 33 – Namespaces.
 *
 * Scenario: a *weather-station data library* that ships two versions of its parser at once.
 * Version 2 is the default through an inline namespace, version 1 stays available for old
 * station firmware, unit helpers live in nested namespaces with a short alias, and two
 * different `mean` functions coexist because each lives in its own namespace.
 *
 * Deliverables (syllabus):
 * - Nested and inline namespaces
 * - Namespace aliases
 * - Anonymous namespaces
 * - using-declarations vs using-directives
 */
#pragma once

#include <istream>
#include <numeric>
#include <ostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

#include "cppm/lesson.hpp"

namespace cppm::day33 {

inline constexpr Deliverable DELIVERABLES[] = {
    {"nested namespaces (C++17 a::b::c syntax)", "weather::units::metric"},
    {"an inline namespace makes v2 the default API", "weather::v2"},
    {"the old API kept reachable by name", "weather::v1"},
    {"a namespace alias shortens long names", "describe_reading"},
    {"two functions with one name in different namespaces", "daily_summary"},
    {"a using-declaration imports exactly one name", "wind_in_kmh"},
};

namespace weather {

/// Old stations (firmware 1.x) send only "temperature_f".
namespace v1 {
struct Reading {
    double temperature_f;
};
inline Reading parse(std::string_view line) {
    std::istringstream in{std::string(line)};
    Reading r{};
    if (!(in >> r.temperature_f)) {
        throw std::invalid_argument("v1 expects one Fahrenheit value");
    }
    return r;
}
inline int api_version() {
    return 1;
}
}  // namespace v1

/// Current stations send "temperature_c,wind_ms". `inline` makes weather::parse mean weather::v2::parse.
inline namespace v2 {
struct Reading {
    double temperature_c;
    double wind_ms;
};
inline Reading parse(std::string_view line) {
    std::istringstream in{std::string(line)};
    Reading r{};
    char comma{};
    if (!(in >> r.temperature_c >> comma >> r.wind_ms) || comma != ',') {
        throw std::invalid_argument("v2 expects 'temperature_c,wind_ms'");
    }
    return r;
}
inline int api_version() {
    return 2;
}
}  // namespace v2

namespace units::metric {  // nested namespace definition (C++17)
inline double to_kmh(double metres_per_second) {
    return metres_per_second * 3.6;
}
}  // namespace units::metric

namespace units::imperial {
inline double to_fahrenheit(double celsius) {
    return celsius * 9.0 / 5.0 + 32.0;
}
inline double to_celsius(double fahrenheit) {
    return (fahrenheit - 32.0) * 5.0 / 9.0;
}
}  // namespace units::imperial

}  // namespace weather

namespace stats {
/// Arithmetic mean of every reading.
inline double mean(const std::vector<double>& values) {
    if (values.empty()) {
        throw std::invalid_argument("mean of nothing");
    }
    return std::accumulate(values.begin(), values.end(), 0.0) / static_cast<double>(values.size());
}
}  // namespace stats

namespace climatology {
/// Meteorologists' "daily mean": the midpoint of the day's minimum and maximum.
inline double mean(const std::vector<double>& values) {
    if (values.empty()) {
        throw std::invalid_argument("mean of nothing");
    }
    double low = values.front();
    double high = values.front();
    for (double v : values) {
        low = v < low ? v : low;
        high = v > high ? v : high;
    }
    return (low + high) / 2.0;
}
}  // namespace climatology

namespace {  // anonymous namespace: visible only inside each file that includes this header
constexpr const char* station_prefix = "WX-";
}  // namespace

/// Both `mean`s in one function: qualifying the name says which one is meant. A using-directive
/// for both namespaces (`using namespace stats; using namespace climatology;`) would make an
/// unqualified `mean(...)` ambiguous and fail to compile.
inline std::pair<double, double> daily_summary(const std::vector<double>& temperatures) {
    return {stats::mean(temperatures), climatology::mean(temperatures)};
}

/// A using-declaration imports one name into this function only – nothing else leaks in.
inline double wind_in_kmh(const weather::Reading& reading) {
    using weather::units::metric::to_kmh;
    return to_kmh(reading.wind_ms);
}

/// A namespace alias keeps long qualified names readable.
inline std::string describe_reading(const std::string& station, const weather::Reading& reading) {
    namespace imperial = weather::units::imperial;
    std::ostringstream out;
    out << station_prefix << station << ": " << reading.temperature_c << " C ("
        << imperial::to_fahrenheit(reading.temperature_c) << " F), wind " << wind_in_kmh(reading) << " km/h";
    return out.str();
}

/// The interactive demo: "v1 <F>" or "v2 <C>,<m/s>" readings; blank line prints the summary.
inline int run(std::istream& in, std::ostream& out) {
    out << "Day 33 – Namespaces\nDefault API version: " << weather::api_version() << '\n';
    std::vector<double> temperatures;
    while (auto line = prompt_line(in, out, "v1 <F> | v2 <C>,<m/s>> ")) {
        if (line->empty()) {
            break;
        }
        try {
            if (line->rfind("v1 ", 0) == 0) {
                const auto old = weather::v1::parse(line->substr(3));
                const double celsius = weather::units::imperial::to_celsius(old.temperature_f);
                temperatures.push_back(celsius);
                out << "  legacy station: " << celsius << " C\n";
            } else if (line->rfind("v2 ", 0) == 0) {
                const auto reading = weather::parse(line->substr(3));  // inline namespace: this is v2::parse
                temperatures.push_back(reading.temperature_c);
                out << "  " << describe_reading("07", reading) << '\n';
            } else {
                out << "  start the line with v1 or v2\n";
            }
        } catch (const std::invalid_argument& error) {
            out << "  " << error.what() << '\n';
        }
    }
    if (!temperatures.empty()) {
        const auto [arithmetic, midpoint] = daily_summary(temperatures);
        out << "stats::mean " << arithmetic << " C, climatology::mean " << midpoint << " C\n";
    }
    return 0;
}

}  // namespace cppm::day33
