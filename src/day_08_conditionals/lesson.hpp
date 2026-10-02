/**
 * @file
 * Day 08 – Conditional Statements.
 *
 * Scenario: a *ski-resort lift operations board*. Every morning the duty manager enters the
 * wind speed, temperature, visibility and fresh snow; the board decides which lifts may open,
 * explains every closure, and rejects sensor readings that cannot be real.
 *
 * Deliverables (syllabus):
 * - if / else if / else chains
 * - switch with enums
 * - Guard clauses
 * - The conditional operator
 */
#pragma once

#include <cmath>
#include <istream>
#include <optional>
#include <ostream>
#include <sstream>
#include <string>
#include <string_view>

#include "cppm/lesson.hpp"

namespace cppm::day08 {

inline constexpr Deliverable DELIVERABLES[] = {
    {"guard clauses reject impossible readings early", "validate"},
    {"an if / else if / else chain over ranges", "classify_wind"},
    {"switch over an enum class with no default", "lift_decision"},
    {"nested conditions kept shallow", "avalanche_warning"},
    {"the conditional operator for short choices", "status_label"},
};

/// One morning's sensor readings.
struct Reading {
    double wind_kmh{0};
    double temperature_c{0};
    double visibility_m{0};
    double new_snow_cm{0};
};

enum class Wind { calm, breezy, strong, storm };
enum class Lift { drag_lift, chairlift, gondola };

/// The board's verdict for one lift.
struct Decision {
    bool open;
    std::string reason;
};

/// Guard clauses: return the first problem, or std::nullopt when the reading is plausible.
inline std::optional<std::string> validate(const Reading& r) {
    if (!std::isfinite(r.wind_kmh) || !std::isfinite(r.temperature_c) || !std::isfinite(r.visibility_m) ||
        !std::isfinite(r.new_snow_cm)) {
        return "a sensor sent a non-numeric value";
    }
    if (r.wind_kmh < 0) {
        return "wind speed cannot be negative";
    }
    if (r.visibility_m < 0) {
        return "visibility cannot be negative";
    }
    if (r.temperature_c < -60 || r.temperature_c > 40) {
        return "temperature outside the sensor's range";
    }
    if (r.new_snow_cm < 0) {
        return "new snow cannot be negative";
    }
    return std::nullopt;
}

/// Ranges checked from the bottom up: each `else if` may assume the previous tests failed.
inline Wind classify_wind(double kmh) {
    if (kmh < 20) {
        return Wind::calm;
    } else if (kmh < 45) {
        return Wind::breezy;
    } else if (kmh < 70) {
        return Wind::strong;
    } else {
        return Wind::storm;
    }
}

/// Each lift type tolerates different wind; a switch without `default` lets the compiler warn
/// if a new Lift value is added and forgotten here.
inline Decision lift_decision(Lift lift, const Reading& r) {
    if (auto problem = validate(r)) {
        return {false, "invalid reading: " + *problem};
    }
    if (r.visibility_m < 50) {
        return {false, "visibility below 50 m"};
    }
    const Wind wind = classify_wind(r.wind_kmh);
    switch (lift) {
        case Lift::drag_lift: return wind == Wind::storm ? Decision{false, "storm-force wind"} : Decision{true, "ok"};
        case Lift::chairlift:
            if (wind == Wind::strong || wind == Wind::storm) {
                return {false, "chairs swing in strong wind"};
            }
            return {true, "ok"};
        case Lift::gondola:
            if (wind != Wind::calm) {
                return {false, "gondola runs only in calm wind"};
            }
            if (r.temperature_c < -25) {
                return {false, "too cold for the cabin doors"};
            }
            return {true, "ok"};
    }
    return {false, "unknown lift"};  // unreachable for valid enum values
}

/// Avalanche warning level 1–4 from fresh snow and temperature.
inline int avalanche_warning(const Reading& r) {
    if (r.new_snow_cm < 10) {
        return 1;
    }
    if (r.new_snow_cm < 30) {
        return r.temperature_c > 0 ? 3 : 2;  // warm, wet snow is less stable
    }
    return 4;
}

/// Short display labels are where the conditional operator shines.
inline std::string_view status_label(const Decision& d) {
    return d.open ? "OPEN" : "CLOSED";
}

inline std::string_view lift_name(Lift lift) {
    switch (lift) {
        case Lift::drag_lift: return "drag lift";
        case Lift::chairlift: return "chairlift";
        case Lift::gondola: return "gondola";
    }
    return "?";
}

/// The interactive demo: one line "wind temperature visibility snow" per morning.
inline int run(std::istream& in, std::ostream& out) {
    out << "Day 08 – Conditional Statements\n";
    while (auto line = prompt_line(in, out, "wind temp visibility snow> ")) {
        if (line->empty()) {
            break;
        }
        std::istringstream values(*line);
        Reading r;
        if (!(values >> r.wind_kmh >> r.temperature_c >> r.visibility_m >> r.new_snow_cm)) {
            out << "  please type four numbers\n";
            continue;
        }
        if (auto problem = validate(r)) {
            out << "  rejected: " << *problem << '\n';
            continue;
        }
        for (Lift lift : {Lift::drag_lift, Lift::chairlift, Lift::gondola}) {
            const Decision d = lift_decision(lift, r);
            out << "  " << lift_name(lift) << ": " << status_label(d) << (d.open ? "" : " – " + d.reason) << '\n';
        }
        out << "  avalanche warning level " << avalanche_warning(r) << '\n';
    }
    return 0;
}

}  // namespace cppm::day08
