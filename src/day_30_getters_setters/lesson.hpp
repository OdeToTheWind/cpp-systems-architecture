/**
 * @file
 * Day 30 – Getters and Setters.
 *
 * Scenario: an *aquarium controller*. The keeper can read and set the water temperature in
 * Celsius or Fahrenheit and adjust the pH and lighting, but the controller refuses any value
 * that would harm the fish – and stores temperatures in one exact internal unit.
 *
 * Deliverables (syllabus):
 * - Accessors and mutators
 * - Validation in setters
 * - Unit conversion behind an interface
 */
#pragma once

#include <cmath>
#include <iomanip>
#include <istream>
#include <ostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

#include "cppm/lesson.hpp"

namespace cppm::day30 {

inline constexpr Deliverable DELIVERABLES[] = {
    {"const getters that never change the object", "AquariumController::temperature_c"},
    {"a getter that converts units on the way out", "AquariumController::temperature_f"},
    {"a setter that validates before it changes anything", "AquariumController::set_temperature_c"},
    {"a setter in a second unit reusing the first", "AquariumController::set_temperature_f"},
    {"a validated setter with a different range", "AquariumController::set_ph"},
    {"an audit trail of every accepted change", "AquariumController::history"},
};

class AquariumController {
  public:
    static constexpr double min_temperature_c = 22.0;
    static constexpr double max_temperature_c = 30.0;

    /// Internally the temperature is whole tenths of a degree, so 25.3 stays exactly 25.3.
    double temperature_c() const { return tenths_c_ / 10.0; }
    double temperature_f() const { return temperature_c() * 9.0 / 5.0 + 32.0; }
    double ph() const { return ph_hundredths_ / 100.0; }
    int light_hours() const { return light_hours_; }
    const std::vector<std::string>& history() const { return history_; }

    /// Throws std::out_of_range for unsafe values; the object is unchanged when it throws.
    void set_temperature_c(double celsius) {
        if (!std::isfinite(celsius) || celsius < min_temperature_c || celsius > max_temperature_c) {
            throw std::out_of_range("temperature must stay between 22 and 30 C");
        }
        tenths_c_ = static_cast<int>(std::lround(celsius * 10.0));
        log("temperature " + format(temperature_c()) + " C");
    }

    /// Converts and delegates, so the rules live in exactly one setter.
    void set_temperature_f(double fahrenheit) { set_temperature_c((fahrenheit - 32.0) * 5.0 / 9.0); }

    void set_ph(double value) {
        if (!std::isfinite(value) || value < 6.0 || value > 8.0) {
            throw std::out_of_range("pH must stay between 6.0 and 8.0");
        }
        ph_hundredths_ = static_cast<int>(std::lround(value * 100.0));
        log("pH " + format(ph()));
    }

    void set_light_hours(int hours) {
        if (hours < 0 || hours > 14) {
            throw std::out_of_range("light must be on 0-14 hours a day");
        }
        light_hours_ = hours;
        log("light " + std::to_string(hours) + " h");
    }

  private:
    static std::string format(double value) {
        std::ostringstream out;
        out << std::fixed << std::setprecision(1) << value;
        return out.str();
    }
    void log(std::string entry) { history_.push_back(std::move(entry)); }

    int tenths_c_{250};       // 25.0 C
    int ph_hundredths_{700};  // 7.00
    int light_hours_{10};
    std::vector<std::string> history_;
};

/// The interactive demo: "c 26.5", "f 80", "ph 7.2", "light 8", "show".
inline int run(std::istream& in, std::ostream& out) {
    out << "Day 30 – Getters and Setters\n";
    AquariumController tank;
    while (auto line = prompt_line(in, out, "c|f|ph|light <value> | show> ")) {
        std::istringstream words(*line);
        std::string what;
        if (!(words >> what)) {
            break;
        }
        double value = 0;
        try {
            if (what == "show") {
                out << std::fixed << std::setprecision(1) << "  " << tank.temperature_c() << " C / "
                    << tank.temperature_f() << " F, pH " << tank.ph() << ", light " << tank.light_hours() << " h\n";
                continue;
            }
            if (!(words >> value)) {
                out << "  a number is required\n";
                continue;
            }
            if (what == "c")
                tank.set_temperature_c(value);
            else if (what == "f")
                tank.set_temperature_f(value);
            else if (what == "ph")
                tank.set_ph(value);
            else if (what == "light")
                tank.set_light_hours(static_cast<int>(value));
            else
                out << "  unknown setting\n";
        } catch (const std::out_of_range& error) {
            out << "  refused: " << error.what() << '\n';
        }
    }
    out << tank.history().size() << " change(s) recorded\n";
    return 0;
}

}  // namespace cppm::day30
