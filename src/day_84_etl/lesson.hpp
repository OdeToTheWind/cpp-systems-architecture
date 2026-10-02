/**
 * @file
 * Day 84 – Capstone: Data Pipeline (ETL).
 *
 * Scenario: the *nightly order import* of an online shop that sells through three marketplaces.
 * Each marketplace exports a CSV in its own quirks; the pipeline extracts rows, transforms them
 * into one clean order format (dates checked, money in integer cents, everything converted to
 * EUR), loads them into the warehouse table without duplicates, and puts every bad row into a
 * quarantine with the reason – so one broken line never stops the whole import.
 *
 * Deliverables (syllabus):
 * - Extract, transform and load stages
 * - Validation and quarantine of bad rows
 * - Idempotent loads
 * - Run summaries
 */
#pragma once

#include <algorithm>
#include <cctype>
#include <cstdint>
#include <istream>
#include <map>
#include <optional>
#include <ostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <variant>
#include <vector>

#include "cppm/lesson.hpp"

namespace cppm::day84 {

inline constexpr Deliverable DELIVERABLES[] = {
    {"extracting CSV rows by header name", "extract"},
    {"transforming one raw row into a clean order or a rejection", "transform"},
    {"an idempotent load keyed by order id", "Warehouse::load"},
    {"the whole pipeline with quarantine", "run_pipeline"},
    {"a run summary for the morning report", "RunSummary::report"},
};

/// Split one CSV line; fields may be "quoted", with "" for a literal quote.
inline std::vector<std::string> parse_csv_line(const std::string& line) {
    std::vector<std::string> fields(1);
    bool quoted = false;
    for (std::size_t i = 0; i < line.size(); ++i) {
        const char c = line[i];
        if (quoted) {
            if (c == '"' && i + 1 < line.size() && line[i + 1] == '"') {
                fields.back() += '"';
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
    if (quoted) throw std::runtime_error("unterminated quote");
    return fields;
}

struct RawRow {
    int line;                                  // in the source file, for the quarantine report
    std::map<std::string, std::string> field;  // by lower-cased header name
};

struct Rejection {
    int line;
    std::string reason;
};

/// Extract: header names become keys, so column order does not matter.
inline std::vector<std::variant<RawRow, Rejection>> extract(std::istream& in) {
    std::vector<std::variant<RawRow, Rejection>> rows;
    std::string text;
    if (!std::getline(in, text)) return rows;
    std::vector<std::string> header = parse_csv_line(text);
    for (auto& h : header) {
        std::transform(h.begin(), h.end(), h.begin(), [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
        h.erase(0, h.find_first_not_of(' '));
    }
    for (int line = 2; std::getline(in, text); ++line) {
        if (text.find_first_not_of(" \r") == std::string::npos) continue;
        try {
            const auto values = parse_csv_line(text);
            if (values.size() != header.size()) {
                rows.emplace_back(Rejection{line, "expected " + std::to_string(header.size()) + " fields, got " + std::to_string(values.size())});
                continue;
            }
            RawRow row{line, {}};
            for (std::size_t i = 0; i < header.size(); ++i) row.field[header[i]] = values[i];
            rows.emplace_back(row);
        } catch (const std::exception& e) {
            rows.emplace_back(Rejection{line, e.what()});
        }
    }
    return rows;
}

struct Order {
    std::string id;
    std::string date;  // YYYY-MM-DD
    std::string email;
    std::int64_t eur_cents;
    std::string source;
    bool operator==(const Order&) const = default;
};

inline bool valid_date(const std::string& d) {
    if (d.size() != 10 || d[4] != '-' || d[7] != '-') return false;
    for (const int i : {0, 1, 2, 3, 5, 6, 8, 9}) {
        if (!std::isdigit(static_cast<unsigned char>(d[static_cast<std::size_t>(i)]))) return false;
    }
    const int y = std::stoi(d.substr(0, 4));
    const int m = std::stoi(d.substr(5, 2));
    const int day = std::stoi(d.substr(8, 2));
    constexpr int days[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    if (m < 1 || m > 12 || day < 1) return false;
    const bool leap = (y % 4 == 0 && y % 100 != 0) || y % 400 == 0;
    return day <= days[m - 1] + (m == 2 && leap ? 1 : 0);
}

/// "1,234.50" or "12.5" -> cents. Floating point is never used for money.
inline std::int64_t parse_cents(std::string text) {
    text.erase(std::remove(text.begin(), text.end(), ','), text.end());
    const auto dot = text.find('.');
    const std::string whole = text.substr(0, dot);
    std::string frac = dot == std::string::npos ? "" : text.substr(dot + 1);
    if (whole.empty() || whole.size() > 12 || whole.find_first_not_of("0123456789") != std::string::npos || frac.size() > 2 ||
        frac.find_first_not_of("0123456789") != std::string::npos) {
        throw std::invalid_argument("bad amount '" + text + "'");
    }
    frac.resize(2, '0');
    return std::stoll(whole) * 100 + std::stoll(frac);
}

/// Transform: validate and normalise one row. Rates are cents of EUR per 100 cents of a currency.
inline std::variant<Order, Rejection> transform(const RawRow& row, const std::map<std::string, std::int64_t>& eur_per_100,
                                               const std::string& source) {
    auto get = [&](const char* name) {
        const auto it = row.field.find(name);
        return it == row.field.end() ? std::string() : it->second;
    };
    const std::string id = get("order_id");
    if (id.empty()) return Rejection{row.line, "missing order_id"};
    std::string date = get("date");
    if (date.size() == 10 && date[2] == '/' && date[5] == '/') date = date.substr(6, 4) + "-" + date.substr(3, 2) + "-" + date.substr(0, 2);  // DD/MM/YYYY
    if (!valid_date(date)) return Rejection{row.line, "invalid date '" + get("date") + "'"};
    std::string email = get("email");
    std::transform(email.begin(), email.end(), email.begin(), [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    if (email.find('@') == std::string::npos) return Rejection{row.line, "invalid email '" + get("email") + "'"};
    std::string currency = get("currency");
    if (currency.empty()) currency = "EUR";
    const auto rate = eur_per_100.find(currency);
    if (rate == eur_per_100.end()) return Rejection{row.line, "unknown currency " + currency};
    std::int64_t cents = 0;
    try {
        cents = parse_cents(get("amount"));
    } catch (const std::invalid_argument& e) {
        return Rejection{row.line, e.what()};
    }
    const std::int64_t eur = (cents * rate->second + 50) / 100;  // round half up
    return Order{source + ":" + id, date, email, eur, source};
}

/// The warehouse table. Loading is idempotent: re-running last night's file changes nothing.
class Warehouse {
public:
    enum class Outcome { inserted, unchanged, updated };
    Outcome load(const Order& order) {
        const auto it = orders_.find(order.id);
        if (it == orders_.end()) {
            orders_.emplace(order.id, order);
            return Outcome::inserted;
        }
        if (it->second == order) return Outcome::unchanged;
        it->second = order;  // a corrected re-export replaces the old row
        return Outcome::updated;
    }
    std::size_t size() const { return orders_.size(); }
    std::int64_t revenue_cents() const {
        std::int64_t total = 0;
        for (const auto& [id, o] : orders_) total += o.eur_cents;
        return total;
    }
    const Order* find(const std::string& id) const {
        const auto it = orders_.find(id);
        return it == orders_.end() ? nullptr : &it->second;
    }

private:
    std::map<std::string, Order> orders_;
};

struct RunSummary {
    std::string source;
    int read = 0;
    int inserted = 0;
    int updated = 0;
    int unchanged = 0;
    std::vector<Rejection> quarantine;

    std::string report() const {
        std::ostringstream out;
        out << source << ": read " << read << ", inserted " << inserted << ", updated " << updated << ", unchanged " << unchanged
            << ", quarantined " << quarantine.size() << '\n';
        for (const auto& r : quarantine) out << "  line " << r.line << ": " << r.reason << '\n';
        return out.str();
    }
};

inline RunSummary run_pipeline(std::istream& csv, const std::string& source, Warehouse& warehouse,
                               const std::map<std::string, std::int64_t>& eur_per_100) {
    RunSummary summary{source, 0, 0, 0, 0, {}};
    for (const auto& extracted : extract(csv)) {
        ++summary.read;
        if (const auto* rejected = std::get_if<Rejection>(&extracted)) {
            summary.quarantine.push_back(*rejected);
            continue;
        }
        const auto result = transform(std::get<RawRow>(extracted), eur_per_100, source);
        if (const auto* rejected = std::get_if<Rejection>(&result)) {
            summary.quarantine.push_back(*rejected);
            continue;
        }
        switch (warehouse.load(std::get<Order>(result))) {
            case Warehouse::Outcome::inserted: ++summary.inserted; break;
            case Warehouse::Outcome::updated: ++summary.updated; break;
            case Warehouse::Outcome::unchanged: ++summary.unchanged; break;
        }
    }
    return summary;
}

inline const std::map<std::string, std::int64_t>& demo_rates() {
    static const std::map<std::string, std::int64_t> rates{{"EUR", 100}, {"GBP", 117}, {"USD", 92}};
    return rates;
}

/// The interactive demo: paste CSV lines (header first) for marketplace "shop"; a blank line imports.
inline int run(std::istream& in, std::ostream& out) {
    out << "Day 84 – Capstone: Data Pipeline (ETL)\n";
    Warehouse warehouse;
    while (true) {
        std::string csv;
        while (auto line = prompt_line(in, out, "csv> ")) {
            if (line->empty()) break;
            csv += *line + "\n";
        }
        if (csv.empty()) break;
        std::istringstream file(csv);
        out << run_pipeline(file, "shop", warehouse, demo_rates()).report();
    }
    out << "warehouse: " << warehouse.size() << " order(s), EUR " << warehouse.revenue_cents() / 100 << "." << (warehouse.revenue_cents() % 100 < 10 ? "0" : "")
        << warehouse.revenue_cents() % 100 << '\n';
    return 0;
}

}  // namespace cppm::day84
