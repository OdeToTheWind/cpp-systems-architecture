/**
 * @file
 * Day 03 – Input & Output Streams.
 *
 * Scenario: a *workshop registration desk* that asks attendees questions in the console,
 * re-asks after every invalid answer instead of crashing or looping forever, and prints a
 * neatly aligned receipt with the ticket price.
 *
 * Deliverables (syllabus):
 * - Validated stream extraction
 * - Recovering from failed reads and clearing the buffer
 * - iomanip formatting
 */
#pragma once

#include <cctype>
#include <iomanip>
#include <ios>
#include <istream>
#include <limits>
#include <optional>
#include <ostream>
#include <sstream>
#include <string>
#include <string_view>

#include "cppm/lesson.hpp"

namespace cppm::day03 {

inline constexpr Deliverable DELIVERABLES[] = {
    {"validated extraction with range checks", "ask_int"},
    {"recovering from a failed read: clear() + ignore()", "recover_stream"},
    {"choosing from a fixed set of answers", "ask_choice"},
    {"iomanip formatting: setw, left/right, fixed, setprecision", "format_receipt"},
    {"a whole registration as one value", "Registration"},
};

/// Everything the desk collects about one attendee.
struct Registration {
    std::string name;
    int age{0};
    char ticket{'S'};  // 'S'tandard, 'V'IP or s'T'udent
    int workshops{1};
};

/// After a failed `>>`, reset the error flags and discard the rest of the bad line.
/// Returns false at end of input, where retrying would loop forever.
inline bool recover_stream(std::istream& in) {
    if (in.eof()) {
        return false;
    }
    in.clear();
    in.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    return true;
}

/// Read an int in [low, high] with `>>`, re-asking on bad input; std::nullopt at end of input.
inline std::optional<int> ask_int(std::istream& in, std::ostream& out, std::string_view prompt, int low,
                                  int high) {
    while (true) {
        out << prompt;
        int value{};
        if (in >> value) {
            in.ignore(std::numeric_limits<std::streamsize>::max(), '\n');  // drop the rest of the line
            if (value >= low && value <= high) {
                return value;
            }
            out << "  please enter a number from " << low << " to " << high << '\n';
            continue;
        }
        if (!recover_stream(in)) {
            return std::nullopt;
        }
        out << "  that was not a whole number\n";
    }
}

/// Read one character from @p allowed (case-insensitive); returns it upper-cased.
inline std::optional<char> ask_choice(std::istream& in, std::ostream& out, std::string_view prompt,
                                      std::string_view allowed) {
    while (auto line = prompt_line(in, out, prompt)) {
        if (line->size() == 1) {
            const char upper = static_cast<char>(std::toupper(static_cast<unsigned char>(line->front())));
            if (allowed.find(upper) != std::string_view::npos) {
                return upper;
            }
        }
        out << "  choose one of: " << allowed << '\n';
    }
    return std::nullopt;
}

/// Ticket price in cents: VIP 120.00, student 45.50, standard 80.00; each extra workshop 15.00.
inline long long price_cents(const Registration& r) {
    const long long base = r.ticket == 'V' ? 12'000 : r.ticket == 'T' ? 4'550 : 8'000;
    return base + 1'500LL * (r.workshops - 1);
}

/// An aligned receipt: labels left-aligned in 12 columns, the amount right-aligned with 2 decimals.
inline std::string format_receipt(const Registration& r) {
    const char* ticket = r.ticket == 'V' ? "VIP" : r.ticket == 'T' ? "Student" : "Standard";
    std::ostringstream out;
    out << std::string(30, '=') << '\n'
        << std::left << std::setw(12) << "Attendee" << r.name << '\n'
        << std::setw(12) << "Age" << r.age << '\n'
        << std::setw(12) << "Ticket" << ticket << '\n'
        << std::setw(12) << "Workshops" << r.workshops << '\n'
        << std::setw(12) << "Total" << std::right << std::setw(15) << std::fixed << std::setprecision(2)
        << static_cast<double>(price_cents(r)) / 100.0 << " EUR\n"
        << std::string(30, '=') << '\n';
    return out.str();
}

/// The interactive demo: ask, validate, print the receipt.
inline int run(std::istream& in, std::ostream& out) {
    out << "Day 03 – Input & Output Streams\n";
    Registration r;
    auto name = prompt_line(in, out, "Name: ");
    if (!name) {
        return 0;
    }
    r.name = name->empty() ? "Anonymous" : *name;
    const auto age = ask_int(in, out, "Age: ", 5, 120);
    const auto ticket = age ? ask_choice(in, out, "Ticket [S]tandard, [V]IP, s[T]udent: ", "SVT") : std::nullopt;
    const auto workshops = ticket ? ask_int(in, out, "Workshops (1-5): ", 1, 5) : std::nullopt;
    if (!workshops) {
        out << "\nRegistration incomplete – nothing was booked.\n";
        return 0;
    }
    r.age = *age;
    r.ticket = *ticket;
    r.workshops = *workshops;
    out << '\n' << format_receipt(r);
    return 0;
}

}  // namespace cppm::day03
