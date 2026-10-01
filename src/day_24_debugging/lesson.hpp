/**
 * @file
 * Day 24 – Debugging Techniques.
 *
 * Scenario: a *library late-fee calculator* that shipped with a real bug: some borrowers were
 * charged for the grace day. The bug is reproduced with a minimal failing case, traced with
 * debug output on std::cerr, cornered by bisecting the inputs, and fixed – with assertions
 * guarding the invariants so it cannot come back.
 *
 * Deliverables (syllabus):
 * - Reproducing bugs
 * - Assertions
 * - std::cerr tracing
 * - Bisecting failing inputs
 * - Debugger-friendly code
 */
#pragma once

#include <functional>
#include <iostream>
#include <istream>
#include <optional>
#include <ostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>

#include "cppm/lesson.hpp"

namespace cppm::day24 {

inline constexpr Deliverable DELIVERABLES[] = {
    {"the shipped bug, kept for study", "late_fee_buggy"},
    {"debugger-friendly code: one named step per line", "late_fee"},
    {"runtime contract checks that throw instead of aborting", "expects"},
    {"opt-in tracing to std::cerr (or any stream)", "Tracer"},
    {"bisecting to the first failing input", "first_failing_day"},
    {"a minimal reproduction report", "reproduce"},
};

inline constexpr int grace_days = 3;              // returning within 3 days of the due date is free
inline constexpr long long cents_per_day = 25;    // after that, 0.25 per day …
inline constexpr long long cap_cents = 1'000;     // … capped at 10.00 per item

/// A contract check: throws std::logic_error naming the broken condition. Unlike assert(),
/// it also runs in release builds and can be tested.
inline void expects(bool condition, std::string_view what) {
    if (!condition) {
        throw std::logic_error("contract violated: " + std::string(what));
    }
}

/// Writes debug lines only when enabled – to std::cerr by default, so normal output stays clean.
class Tracer {
public:
    explicit Tracer(bool enabled = false, std::ostream& sink = std::cerr) : enabled_(enabled), sink_(&sink) {}
    void trace(std::string_view where, std::string_view what, long long value) const {
        if (enabled_) {
            *sink_ << "[trace] " << where << ": " << what << " = " << value << '\n';
        }
    }

private:
    bool enabled_;
    std::ostream* sink_;
};

/// THE BUG (kept for study): `>=` charges a borrower who returns on the last grace day, and the
/// grace days are not subtracted from the chargeable days.
inline long long late_fee_buggy(int days_late) {
    if (days_late >= grace_days) {
        const long long fee = days_late * cents_per_day;
        return fee > cap_cents ? cap_cents : fee;
    }
    return 0;
}

/// The fixed version, written so that a debugger can show every intermediate value.
inline long long late_fee(int days_late, const Tracer& tracer = Tracer{}) {
    expects(days_late >= 0, "days_late >= 0");
    const int chargeable_days = days_late > grace_days ? days_late - grace_days : 0;
    tracer.trace("late_fee", "chargeable_days", chargeable_days);
    const long long uncapped = chargeable_days * cents_per_day;
    tracer.trace("late_fee", "uncapped", uncapped);
    const long long fee = uncapped > cap_cents ? cap_cents : uncapped;
    expects(fee >= 0 && fee <= cap_cents, "0 <= fee <= cap");
    return fee;
}

/// The smallest day in [low, high] where @p fails is true, assuming every later day fails too.
/// Binary search needs about log2(high - low) checks instead of high - low.
inline std::optional<int> first_failing_day(const std::function<bool(int)>& fails, int low, int high,
                                            int* checks = nullptr) {
    std::optional<int> first;
    int count = 0;
    while (low <= high) {
        const int middle = low + (high - low) / 2;  // never (low + high) / 2: that can overflow
        ++count;
        if (fails(middle)) {
            first = middle;
            high = middle - 1;
        } else {
            low = middle + 1;
        }
    }
    if (checks != nullptr) {
        *checks = count;
    }
    return first;
}

/// A bug report someone else can reproduce: the input, what was expected and what happened.
inline std::string reproduce(int days_late) {
    const long long expected = late_fee(days_late);
    const long long actual = late_fee_buggy(days_late);
    std::ostringstream report;
    report << "input days_late=" << days_late << " expected=" << expected << " actual=" << actual << ' '
           << (expected == actual ? "OK" : "MISMATCH");
    return report.str();
}

/// The interactive demo: "days [trace]" lines; prints both versions and traces on request.
inline int run(std::istream& in, std::ostream& out) {
    out << "Day 24 – Debugging Techniques\n";
    int checks = 0;
    const auto boundary = first_failing_day([](int day) { return late_fee(day) != late_fee_buggy(day); }, 0, 365,
                                            &checks);
    out << "Bisect: first mismatch at day " << boundary.value_or(-1) << " after " << checks << " checks\n";
    while (auto line = prompt_line(in, out, "days late [trace]> ")) {
        if (line->empty()) {
            break;
        }
        std::istringstream words(*line);
        int days = 0;
        std::string flag;
        if (!(words >> days)) {
            out << "  type a number of days\n";
            continue;
        }
        words >> flag;
        try {
            const Tracer tracer(flag == "trace", out);  // traces go to `out` here so the demo shows them
            // compute first: calling late_fee inside the << chain would print its traces mid-line
            const long long fee = late_fee(days, tracer);
            out << "  fee " << fee << " cents; " << reproduce(days) << '\n';
        } catch (const std::logic_error& error) {
            out << "  " << error.what() << '\n';
        }
    }
    return 0;
}

}  // namespace cppm::day24
