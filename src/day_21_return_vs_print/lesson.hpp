/**
 * @file
 * Day 21 – Return vs Print.
 *
 * Scenario: an *apartment-building electricity billing tool*. The same tiered tariff is
 * written twice: once as a function that prints as it calculates (hard to reuse or test),
 * and once as pure functions that return a bill which is formatted separately – and only the
 * second design can total the whole building.
 *
 * Deliverables (syllabus):
 * - Pure functions vs side effects
 * - Returning data and formatting it separately
 * - Testable design
 */
#pragma once

#include <iomanip>
#include <istream>
#include <ostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

#include "cppm/lesson.hpp"

namespace cppm::day21 {

inline constexpr Deliverable DELIVERABLES[] = {
    {"a print-only function with side effects (the anti-pattern)", "print_bill_badly"},
    {"a pure function: same input, same output, no I/O", "energy_charge_cents"},
    {"returning structured data instead of text", "compute_bill"},
    {"formatting kept separate from calculation", "format_bill"},
    {"reuse that only the returning design allows", "building_total"},
    {"what reusing printed output costs", "total_from_printed_bill"},
};

/// Tiered tariff: the first 100 kWh at 0.20, the next 200 at 0.25, everything above at 0.32.
inline long long energy_charge_cents(int kwh) {
    if (kwh < 0) {
        throw std::invalid_argument("meter readings cannot go backwards");
    }
    const long long first = kwh < 100 ? kwh : 100;
    const long long second = kwh <= 100 ? 0 : (kwh < 300 ? kwh - 100 : 200);
    const long long rest = kwh > 300 ? kwh - 300 : 0;
    return first * 20 + second * 25 + rest * 32;
}

/// The anti-pattern: calculates *and* prints, returns nothing. Callers cannot use the numbers.
inline void print_bill_badly(std::ostream& out, const std::string& flat, int previous_reading, int current_reading) {
    const int kwh = current_reading - previous_reading;
    long long total = energy_charge_cents(kwh) + 900;  // standing charge
    total += total * 5 / 100;                          // VAT
    out << "Flat " << flat << ": " << kwh << " kWh, total " << total / 100 << '.' << std::setw(2)
        << std::setfill('0') << total % 100 << std::setfill(' ') << '\n';
}

/// Everything a bill contains, as data.
struct Bill {
    std::string flat;
    int kwh;
    long long energy_cents;
    long long standing_cents;
    long long vat_cents;
    long long total_cents() const { return energy_cents + standing_cents + vat_cents; }
};

/// Pure: depends only on its arguments, changes nothing, prints nothing.
inline Bill compute_bill(const std::string& flat, int previous_reading, int current_reading) {
    const int kwh = current_reading - previous_reading;
    const long long energy = energy_charge_cents(kwh);
    const long long standing = 900;
    const long long vat = (energy + standing) * 5 / 100;
    return {flat, kwh, energy, standing, vat};
}

inline std::string money(long long cents) {
    std::ostringstream out;
    out << cents / 100 << '.' << std::setw(2) << std::setfill('0') << cents % 100;
    return out.str();
}

/// Presentation only: turns a Bill into text. Changing the layout cannot break the maths.
inline std::string format_bill(const Bill& bill) {
    std::ostringstream out;
    out << "Flat " << bill.flat << ": " << bill.kwh << " kWh\n"
        << "  energy   " << std::setw(8) << money(bill.energy_cents) << '\n'
        << "  standing " << std::setw(8) << money(bill.standing_cents) << '\n'
        << "  VAT 5%   " << std::setw(8) << money(bill.vat_cents) << '\n'
        << "  total    " << std::setw(8) << money(bill.total_cents()) << '\n';
    return out.str();
}

/// Totalling the building is one line of arithmetic on returned values.
inline long long building_total(const std::vector<Bill>& bills) {
    long long total = 0;
    for (const auto& bill : bills) {
        total += bill.total_cents();
    }
    return total;
}

/// With the print-only design, the only way to reuse the total is to parse the text back – fragile.
inline long long total_from_printed_bill(const std::string& printed) {
    const auto at = printed.find("total ");
    if (at == std::string::npos) {
        throw std::runtime_error("output format changed: cannot find the total");
    }
    std::istringstream number(printed.substr(at + 6));
    long long units = 0;
    char dot{};
    int cents = 0;
    if (!(number >> units >> dot >> cents) || dot != '.') {
        throw std::runtime_error("output format changed: cannot read the total");
    }
    return units * 100 + cents;
}

/// The interactive demo: "flat previous current" per line, then the building total.
inline int run(std::istream& in, std::ostream& out) {
    out << "Day 21 – Return vs Print\n";
    std::vector<Bill> bills;
    while (auto line = prompt_line(in, out, "flat previous current> ")) {
        if (line->empty()) {
            break;
        }
        std::istringstream words(*line);
        std::string flat;
        int previous = 0;
        int current = 0;
        if (!(words >> flat >> previous >> current)) {
            out << "  e.g. 3B 1200 1450\n";
            continue;
        }
        try {
            bills.push_back(compute_bill(flat, previous, current));
            out << format_bill(bills.back());
        } catch (const std::invalid_argument& error) {
            out << "  " << error.what() << '\n';
        }
    }
    out << "Building total: " << money(building_total(bills)) << " (" << bills.size() << " flats)\n";
    return 0;
}

}  // namespace cppm::day21
