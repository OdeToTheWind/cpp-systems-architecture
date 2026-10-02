// budget/report.hpp – monthly report as text and CSV (Days 44, 88).
#pragma once

#include <map>
#include <string>
#include <vector>

#include "budget/config.hpp"
#include "budget/money.hpp"
#include "budget/store.hpp"

namespace cppm::day100 {

struct MonthReport {
    std::string month;
    Cents income = 0;
    Cents spent = 0;  // positive number
    std::map<std::string, Cents> spent_by_category;
    std::vector<std::string> warnings;
    Cents balance() const { return income - spent; }
};

/// Totals for one month and a warning per category at >= 90% of its limit or over it.
inline MonthReport build_month(const std::vector<Entry>& entries, const std::string& month, const Settings& settings) {
    MonthReport r{month, 0, 0, {}, {}};
    for (const auto& e : entries) {
        if (e.month() != month) continue;
        if (e.cents > 0) {
            r.income += e.cents;
        } else {
            r.spent += -e.cents;
            r.spent_by_category[e.category] += -e.cents;
        }
    }
    for (const auto& [category, limit] : settings.monthly_limits) {
        const Cents used = r.spent_by_category.count(category) ? r.spent_by_category.at(category) : 0;
        if (used > limit) {
            r.warnings.push_back(category + " is over budget by " + format_money(used - limit, settings.symbol));
        } else if (limit > 0 && used * 10 >= limit * 9) {
            r.warnings.push_back(category + " has used " + std::to_string(used * 100 / limit) + "% of its budget");
        }
    }
    return r;
}

inline std::string render_text(const MonthReport& r, const Settings& s) {
    std::string out = "Report " + r.month + "\n  income   " + format_money(r.income, s.symbol) + "\n  spent    " +
                      format_money(r.spent, s.symbol) + "\n  balance  " + format_money(r.balance(), s.symbol) + "\n";
    for (const auto& [category, cents] : r.spent_by_category)
        out += "    " + category + ": " + format_money(cents, s.symbol) + "\n";
    for (const auto& w : r.warnings) out += "  ! " + w + "\n";
    return out;
}

inline std::string csv_cell(const std::string& v) {
    std::string cell = (!v.empty() && (v[0] == '=' || v[0] == '+' || v[0] == '-' || v[0] == '@')) ? "'" + v : v;
    if (cell.find_first_of(",\"\n") == std::string::npos) return cell;
    std::string out = "\"";
    for (const char c : cell) out += c == '"' ? std::string("\"\"") : std::string(1, c);
    return out + "\"";
}

/// Every entry of the month, for a spreadsheet. Amounts are plain decimals.
inline std::string render_csv(const std::vector<Entry>& entries, const std::string& month) {
    std::string out = "id,date,amount,category,note\n";
    for (const auto& e : entries) {
        if (e.month() != month) continue;
        const Cents abs = e.cents < 0 ? -e.cents : e.cents;
        const std::string amount = (e.cents < 0 ? "-" : "") + std::to_string(abs / 100) + "." +
                                   (abs % 100 < 10 ? "0" : "") + std::to_string(abs % 100);
        out += std::to_string(e.id) + "," + e.date + "," + amount + "," + csv_cell(e.category) + "," +
               csv_cell(e.note) + "\n";
    }
    return out;
}

}  // namespace cppm::day100
