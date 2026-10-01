/**
 * @file
 * Day 11 – Error Handling.
 *
 * Scenario: a *greenhouse sensor log reader*. Log files are messy, devices disappear and
 * people type impossible values; the reader must keep going, count what went wrong and why,
 * and only stop for errors it truly cannot handle.
 *
 * Deliverables (syllabus):
 * - throw, try and catch
 * - The std::exception hierarchy
 * - Custom exceptions
 * - Rethrowing
 * - Input validation
 */
#pragma once

#include <cmath>
#include <cstddef>
#include <exception>
#include <istream>
#include <map>
#include <ostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

#include "cppm/lesson.hpp"

namespace cppm::day11 {

inline constexpr Deliverable DELIVERABLES[] = {
    {"a custom exception derived from std::runtime_error", "SensorError"},
    {"throwing standard exceptions for invalid input", "parse_reading"},
    {"catching by const reference, most specific first", "summarise_log"},
    {"rethrowing with throw; after logging", "load_with_audit"},
    {"bounds-checked access with .at() and std::out_of_range", "reading_at"},
};

/// A reading that is well-formed but physically impossible. Carries the offending sensor id.
class SensorError : public std::runtime_error {
public:
    SensorError(std::string sensor, const std::string& message)
        : std::runtime_error(sensor + ": " + message), sensor_(std::move(sensor)) {}
    const std::string& sensor() const noexcept { return sensor_; }

private:
    std::string sensor_;
};

/// One parsed log line.
struct Reading {
    std::string sensor;
    double temperature_c;
    double humidity_pct;
};

/// Parse "sensor,temperature,humidity".
/// Throws std::invalid_argument for malformed text and SensorError for impossible values.
inline Reading parse_reading(const std::string& line) {
    std::istringstream fields(line);
    std::string sensor;
    std::string temperature_text;
    std::string humidity_text;
    if (!std::getline(fields, sensor, ',') || !std::getline(fields, temperature_text, ',') ||
        !std::getline(fields, humidity_text) || sensor.empty()) {
        throw std::invalid_argument("expected 'sensor,temperature,humidity'");
    }
    std::size_t used = 0;
    const double temperature = std::stod(temperature_text, &used);  // throws invalid_argument / out_of_range
    if (used != temperature_text.size()) {
        throw std::invalid_argument("trailing characters after temperature");
    }
    const double humidity = std::stod(humidity_text, &used);
    if (used != humidity_text.size()) {
        throw std::invalid_argument("trailing characters after humidity");
    }
    if (!std::isfinite(temperature) || temperature < -30 || temperature > 60) {
        throw SensorError(sensor, "temperature out of range");
    }
    if (!std::isfinite(humidity) || humidity < 0 || humidity > 100) {
        throw SensorError(sensor, "humidity must be 0-100 %");
    }
    return {sensor, temperature, humidity};
}

/// The outcome of reading a whole log.
struct LogSummary {
    std::vector<Reading> readings;
    int malformed{0};
    std::map<std::string, int> faulty_sensors;  // sensor id -> impossible readings
    double average_temperature() const {
        if (readings.empty()) {
            throw std::logic_error("no valid readings to average");
        }
        double sum = 0;
        for (const auto& r : readings) {
            sum += r.temperature_c;
        }
        return sum / static_cast<double>(readings.size());
    }
};

/// Read every line, recover from bad ones, and keep going. Catch clauses go from the most
/// specific type (SensorError) to the more general ones; all catch by const reference.
inline LogSummary summarise_log(std::istream& log) {
    LogSummary summary;
    std::string line;
    while (std::getline(log, line)) {
        if (line.empty() || line.front() == '#') {
            continue;
        }
        try {
            summary.readings.push_back(parse_reading(line));
        } catch (const SensorError& error) {
            ++summary.faulty_sensors[error.sensor()];
        } catch (const std::invalid_argument&) {
            ++summary.malformed;
        } catch (const std::out_of_range&) {  // std::stod on "1e999"
            ++summary.malformed;
        }
    }
    return summary;
}

/// Records a failure in @p audit, then rethrows the *same* exception object with `throw;`.
inline LogSummary load_with_audit(std::istream& log, std::vector<std::string>& audit) {
    try {
        if (!log) {
            throw std::runtime_error("log stream is not readable");
        }
        auto summary = summarise_log(log);
        if (summary.readings.empty()) {
            throw std::runtime_error("log contained no valid readings");
        }
        return summary;
    } catch (const std::exception& error) {
        audit.push_back(std::string("load failed: ") + error.what());
        throw;  // not `throw error;` – that would copy and slice a derived exception
    }
}

/// Bounds-checked access: `.at()` throws std::out_of_range where `[]` would be undefined behaviour.
inline const Reading& reading_at(const LogSummary& summary, std::size_t index) {
    return summary.readings.at(index);
}

/// The interactive demo: paste log lines, then an empty line prints the summary.
inline int run(std::istream& in, std::ostream& out) {
    out << "Day 11 – Error Handling\nPaste 'sensor,temperature,humidity' lines, blank line to summarise\n";
    std::ostringstream collected;
    while (auto line = prompt_line(in, out, "log> ")) {
        if (line->empty()) {
            break;
        }
        collected << *line << '\n';
    }
    std::istringstream log(collected.str());
    std::vector<std::string> audit;
    try {
        const auto summary = load_with_audit(log, audit);
        out << summary.readings.size() << " valid, " << summary.malformed << " malformed\n";
        for (const auto& [sensor, count] : summary.faulty_sensors) {
            out << "  faulty sensor " << sensor << ": " << count << " impossible reading(s)\n";
        }
        out << "Average temperature: " << summary.average_temperature() << " C\n";
        out << "First reading from: " << reading_at(summary, 0).sensor << '\n';
    } catch (const std::exception& error) {
        out << "Error: " << error.what() << '\n';
    }
    for (const auto& entry : audit) {
        out << "[audit] " << entry << '\n';
    }
    return 0;
}

}  // namespace cppm::day11
