/**
 * @file
 * Day 44 – Tabular Data Analysis.
 *
 * Scenario: a *bike-share trip analysis*. A month of trips is loaded into a small
 * column-oriented table – the same idea as a pandas DataFrame – then filtered, extended with a
 * derived speed column, grouped by station and summarised with count, mean, median and
 * standard deviation.
 *
 * Deliverables (syllabus):
 * - Column-oriented tables
 * - Filtering
 * - Derived columns
 * - Group-by aggregation
 * - Summary statistics
 */
#pragma once

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <functional>
#include <iomanip>
#include <istream>
#include <map>
#include <ostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

#include "cppm/lesson.hpp"

namespace cppm::day44 {

inline constexpr Deliverable DELIVERABLES[] = {
    {"a column-oriented table with typed columns", "Table"},
    {"filtering rows with a predicate", "Table::filter"},
    {"adding a column derived from others", "Table::derive"},
    {"group-by with an aggregation", "group_mean"},
    {"count, mean, median, standard deviation, min and max", "describe"},
};

/// Columns stored separately (one vector per column): scanning one column touches only its data.
class Table {
  public:
    /// A view of one row, used by predicates and derivations.
    struct Row {
        const Table* table;
        std::size_t index;
        double number(const std::string& column) const { return table->numbers(column)[index]; }
        const std::string& text(const std::string& column) const { return table->texts(column)[index]; }
    };

    void add_numeric(const std::string& name, std::vector<double> values) {
        check_and_store(name, numeric_, std::move(values));
    }
    void add_text(const std::string& name, std::vector<std::string> values) {
        check_and_store(name, text_, std::move(values));
    }

    std::size_t rows() const { return rows_; }
    const std::vector<double>& numbers(const std::string& column) const { return find(numeric_, column); }
    const std::vector<std::string>& texts(const std::string& column) const { return find(text_, column); }

    /// A new table holding only the rows for which @p keep returns true.
    Table filter(const std::function<bool(const Row&)>& keep) const {
        std::vector<std::size_t> selected;
        for (std::size_t i = 0; i < rows_; ++i) {
            if (keep(Row{this, i})) {
                selected.push_back(i);
            }
        }
        Table result;
        for (const auto& [name, values] : numeric_) {
            std::vector<double> kept;
            for (std::size_t i : selected) kept.push_back(values[i]);
            result.add_numeric(name, std::move(kept));
        }
        for (const auto& [name, values] : text_) {
            std::vector<std::string> kept;
            for (std::size_t i : selected) kept.push_back(values[i]);
            result.add_text(name, std::move(kept));
        }
        result.rows_ = selected.size();
        return result;
    }

    /// Add a numeric column computed from each row.
    void derive(const std::string& name, const std::function<double(const Row&)>& formula) {
        std::vector<double> values;
        values.reserve(rows_);
        for (std::size_t i = 0; i < rows_; ++i) {
            values.push_back(formula(Row{this, i}));
        }
        add_numeric(name, std::move(values));
    }

  private:
    template <typename T>
    void check_and_store(const std::string& name, std::map<std::string, std::vector<T>>& store, std::vector<T> values) {
        if (numeric_.contains(name) || text_.contains(name)) {
            throw std::invalid_argument("duplicate column " + name);
        }
        if (!numeric_.empty() || !text_.empty()) {
            if (values.size() != rows_) {
                throw std::invalid_argument("column " + name + " has the wrong number of rows");
            }
        } else {
            rows_ = values.size();
        }
        store.emplace(name, std::move(values));
    }
    template <typename T>
    static const std::vector<T>& find(const std::map<std::string, std::vector<T>>& store, const std::string& name) {
        const auto it = store.find(name);
        if (it == store.end()) {
            throw std::out_of_range("no column " + name);
        }
        return it->second;
    }

    std::map<std::string, std::vector<double>> numeric_;
    std::map<std::string, std::vector<std::string>> text_;
    std::size_t rows_{0};
};

struct Summary {
    std::size_t count;
    double mean;
    double median;
    double stddev;  // sample standard deviation (n - 1)
    double min;
    double max;
};

inline Summary describe(std::vector<double> values) {
    if (values.empty()) {
        throw std::invalid_argument("cannot describe an empty column");
    }
    std::sort(values.begin(), values.end());
    const auto n = values.size();
    double sum = 0;
    for (double v : values) sum += v;
    const double mean = sum / static_cast<double>(n);
    double squares = 0;
    for (double v : values) squares += (v - mean) * (v - mean);
    const double median = n % 2 == 1 ? values[n / 2] : (values[n / 2 - 1] + values[n / 2]) / 2.0;
    const double stddev = n > 1 ? std::sqrt(squares / static_cast<double>(n - 1)) : 0.0;
    return {n, mean, median, stddev, values.front(), values.back()};
}

/// Mean of @p value_column for every distinct value of @p key_column (like df.groupby(key)[value].mean()).
inline std::map<std::string, double> group_mean(const Table& table, const std::string& key_column,
                                                const std::string& value_column) {
    std::map<std::string, std::pair<double, int>> sums;
    const auto& keys = table.texts(key_column);
    const auto& values = table.numbers(value_column);
    for (std::size_t i = 0; i < table.rows(); ++i) {
        auto& [sum, count] = sums[keys[i]];
        sum += values[i];
        ++count;
    }
    std::map<std::string, double> means;
    for (const auto& [key, total] : sums) {
        means[key] = total.first / total.second;
    }
    return means;
}

/// Build the trips table from "station minutes km" lines.
inline Table load_trips(std::istream& in) {
    std::vector<std::string> stations;
    std::vector<double> minutes;
    std::vector<double> km;
    std::string line;
    while (std::getline(in, line)) {
        std::istringstream words(line);
        std::string station;
        double m = 0;
        double d = 0;
        if (words >> station >> m >> d && m > 0 && d >= 0) {
            stations.push_back(station);
            minutes.push_back(m);
            km.push_back(d);
        }
    }
    Table trips;
    trips.add_text("station", std::move(stations));
    trips.add_numeric("minutes", std::move(minutes));
    trips.add_numeric("km", std::move(km));
    return trips;
}

/// The interactive demo: "station minutes km" lines, then END.
inline int run(std::istream& in, std::ostream& out) {
    out << "Day 44 – Tabular Data Analysis\nTrips as 'station minutes km', then END\n";
    std::string text;
    while (auto line = prompt_line(in, out, "")) {
        if (*line == "END") break;
        text += *line + '\n';
    }
    std::istringstream data(text);
    Table trips = load_trips(data);
    if (trips.rows() == 0) {
        out << "no valid trips\n";
        return 0;
    }
    trips.derive("kmh", [](const Table::Row& r) { return r.number("km") / (r.number("minutes") / 60.0); });
    const Table rides =
        trips.filter([](const Table::Row& r) { return r.number("minutes") >= 2; });  // drop docking errors
    out << std::fixed << std::setprecision(1);
    out << rides.rows() << " of " << trips.rows() << " trips kept\n";
    for (const auto& [station, mean] : group_mean(rides, "station", "kmh")) {
        out << "  " << station << ": " << mean << " km/h\n";
    }
    const Summary s = describe(rides.numbers("minutes"));
    out << "minutes: mean " << s.mean << ", median " << s.median << ", sd " << s.stddev << ", range " << s.min << "-"
        << s.max << '\n';
    return 0;
}

}  // namespace cppm::day44
