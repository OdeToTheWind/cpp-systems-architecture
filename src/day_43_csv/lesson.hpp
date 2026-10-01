/**
 * @file
 * Day 43 – Reading and Writing CSV.
 *
 * Scenario: a *charity fun-run registration import*. The web form exports a CSV in which
 * names contain commas and quotes, some rows are broken and some values are impossible. The
 * importer parses quoted fields correctly, reports every bad row with its line number instead
 * of crashing, and writes a clean CSV for the timing company.
 *
 * Deliverables (syllabus):
 * - Parsing quoted CSV fields
 * - Validating rows
 * - Reporting bad rows
 * - Writing CSV safely
 */
#pragma once

#include <cstddef>
#include <istream>
#include <map>
#include <optional>
#include <ostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "cppm/lesson.hpp"

namespace cppm::day43 {

inline constexpr Deliverable DELIVERABLES[] = {
    {"a quote-aware CSV record parser (RFC 4180)", "read_record"},
    {"validating one row into a typed record", "validate_row"},
    {"importing everything, collecting bad rows with line numbers", "import_runners"},
    {"quoting fields only when necessary", "csv_escape"},
    {"writing a CSV that round-trips", "write_runners"},
};

/// Read one record, which may span several physical lines if a quoted field contains a newline.
/// Returns std::nullopt at end of input; throws std::runtime_error for an unterminated quote.
/// @p line_number is advanced by the number of physical lines consumed.
inline std::optional<std::vector<std::string>> read_record(std::istream& in, int& line_number) {
    std::string line;
    if (!std::getline(in, line)) {
        return std::nullopt;
    }
    ++line_number;
    std::vector<std::string> fields(1);
    bool quoted = false;
    for (std::size_t i = 0;; ++i) {
        if (i == line.size()) {
            if (!quoted) {
                break;
            }
            std::string next;  // a newline inside quotes belongs to the field
            if (!std::getline(in, next)) {
                throw std::runtime_error("line " + std::to_string(line_number) + ": unterminated quoted field");
            }
            ++line_number;
            fields.back() += '\n';
            line = std::move(next);
            i = static_cast<std::size_t>(-1);  // ++i makes it 0
            continue;
        }
        const char c = line[i];
        if (quoted) {
            if (c == '"' && i + 1 < line.size() && line[i + 1] == '"') {
                fields.back() += '"';  // "" inside quotes is one literal quote
                ++i;
            } else if (c == '"') {
                quoted = false;
            } else {
                fields.back() += c;
            }
        } else if (c == '"') {
            quoted = true;
        } else if (c == ',') {
            fields.emplace_back();
        } else if (c != '\r') {
            fields.back() += c;
        }
    }
    return fields;
}

struct Runner {
    std::string name;
    std::string email;
    int age;
    int distance_km;
};

/// A row is valid when it has 4 fields, a name, an email with '@', an age 8–99 and a distance of 5, 10 or 21.
/// Returns the runner, or an error message.
inline std::pair<std::optional<Runner>, std::string> validate_row(const std::vector<std::string>& fields) {
    if (fields.size() != 4) {
        return {std::nullopt, "expected 4 fields, found " + std::to_string(fields.size())};
    }
    Runner runner{fields[0], fields[1], 0, 0};
    if (runner.name.empty()) {
        return {std::nullopt, "name is empty"};
    }
    if (runner.email.find('@') == std::string::npos) {
        return {std::nullopt, "email '" + runner.email + "' has no @"};
    }
    try {
        std::size_t used = 0;
        runner.age = std::stoi(fields[2], &used);
        if (used != fields[2].size()) throw std::invalid_argument("trailing text");
        runner.distance_km = std::stoi(fields[3], &used);
        if (used != fields[3].size()) throw std::invalid_argument("trailing text");
    } catch (const std::exception&) {
        return {std::nullopt, "age and distance must be whole numbers"};
    }
    if (runner.age < 8 || runner.age > 99) {
        return {std::nullopt, "age " + std::to_string(runner.age) + " is outside 8-99"};
    }
    if (runner.distance_km != 5 && runner.distance_km != 10 && runner.distance_km != 21) {
        return {std::nullopt, "distance must be 5, 10 or 21 km"};
    }
    return {runner, ""};
}

struct ImportResult {
    std::vector<Runner> runners;
    std::vector<std::pair<int, std::string>> errors;  // (line number, message)
};

/// Skip the header row, validate every record, keep going after bad rows.
inline ImportResult import_runners(std::istream& csv) {
    ImportResult result;
    int line_number = 0;
    bool header = true;
    while (true) {
        const int starts_at = line_number + 1;
        std::optional<std::vector<std::string>> record;
        try {
            record = read_record(csv, line_number);
        } catch (const std::runtime_error& error) {
            result.errors.emplace_back(starts_at, error.what());
            break;
        }
        if (!record) {
            break;
        }
        if (header) {
            header = false;
            continue;
        }
        if (record->size() == 1 && record->front().empty()) {
            continue;  // blank line
        }
        auto [runner, error] = validate_row(*record);
        if (runner) {
            result.runners.push_back(*runner);
        } else {
            result.errors.emplace_back(starts_at, error);
        }
    }
    return result;
}

/// Quote a field only if it contains a comma, a quote or a line break; double any quotes inside.
inline std::string csv_escape(std::string_view field) {
    if (field.find_first_of(",\"\r\n") == std::string_view::npos) {
        return std::string(field);
    }
    std::string quoted = "\"";
    for (const char c : field) {
        quoted += c;
        if (c == '"') {
            quoted += '"';
        }
    }
    return quoted + '"';
}

inline void write_runners(std::ostream& out, const std::vector<Runner>& runners) {
    out << "name,email,age,distance_km\n";
    for (const auto& r : runners) {
        out << csv_escape(r.name) << ',' << csv_escape(r.email) << ',' << r.age << ',' << r.distance_km << '\n';
    }
}

/// The interactive demo: paste the export (header first), finish with a line "END".
inline int run(std::istream& in, std::ostream& out) {
    out << "Day 43 – Reading and Writing CSV\nPaste the CSV export, then END\n";
    std::string text;
    while (auto line = prompt_line(in, out, "")) {
        if (*line == "END") {
            break;
        }
        text += *line + '\n';
    }
    std::istringstream csv(text);
    const auto result = import_runners(csv);
    for (const auto& [line, message] : result.errors) {
        out << "line " << line << ": " << message << '\n';
    }
    std::map<int, int> per_distance;
    for (const auto& runner : result.runners) {
        ++per_distance[runner.distance_km];
    }
    for (const auto& [km, count] : per_distance) {
        out << km << " km: " << count << " runner(s)\n";
    }
    write_runners(out, result.runners);
    return 0;
}

}  // namespace cppm::day43
