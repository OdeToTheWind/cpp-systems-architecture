/**
 * @file
 * Day 28 – Creating Classes.
 *
 * Scenario: a *gym class booking system*. A `FitnessClass` object guards its own rules –
 * never more attendees than places, nobody booked twice, the waiting list promoted in order –
 * so no caller can ever put it into an invalid state.
 *
 * Deliverables (syllabus):
 * - Class definitions
 * - Constructors and member functions
 * - Invariants
 * - Explicit constructors
 * - operator<<
 */
#pragma once

#include <algorithm>
#include <cstddef>
#include <iomanip>
#include <istream>
#include <optional>
#include <ostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

#include "cppm/lesson.hpp"

namespace cppm::day28 {

inline constexpr Deliverable DELIVERABLES[] = {
    {"a small value class with an explicit constructor", "TimeSlot"},
    {"a class whose constructor establishes the invariant", "FitnessClass::FitnessClass"},
    {"member functions that preserve the invariant", "FitnessClass::book"},
    {"cancelling promotes the waiting list in order", "FitnessClass::cancel"},
    {"a self-check of the invariant", "FitnessClass::invariant_holds"},
    {"stream output with operator<<", "operator<<"},
};

/// Start time in minutes after midnight. `explicit` stops `TimeSlot slot = 1080;` from compiling.
class TimeSlot {
public:
    explicit TimeSlot(int minutes_after_midnight) : minutes_(minutes_after_midnight) {
        if (minutes_ < 0 || minutes_ >= 24 * 60) {
            throw std::out_of_range("a time slot must be within one day");
        }
    }
    int hour() const { return minutes_ / 60; }
    int minute() const { return minutes_ % 60; }
    bool operator==(const TimeSlot&) const = default;

private:
    int minutes_;
};

inline std::ostream& operator<<(std::ostream& out, const TimeSlot& slot) {
    const char previous_fill = out.fill('0');
    out << std::setw(2) << slot.hour() << ':' << std::setw(2) << slot.minute();
    out.fill(previous_fill);
    return out;
}

/// What happened to a booking request.
enum class BookingResult { booked, waitlisted, already_booked };

/// Invariant: attendees.size() <= capacity, and nobody is listed twice (in either list).
class FitnessClass {
public:
    FitnessClass(std::string name, TimeSlot start, int capacity)
        : name_(std::move(name)), start_(start), capacity_(capacity) {
        if (name_.empty()) {
            throw std::invalid_argument("a class needs a name");
        }
        if (capacity_ <= 0) {
            throw std::invalid_argument("capacity must be positive");
        }
    }

    /// Book @p member, or put them on the waiting list when the class is full.
    BookingResult book(const std::string& member) {
        if (contains(attendees_, member) || contains(waitlist_, member)) {
            return BookingResult::already_booked;
        }
        if (is_full()) {
            waitlist_.push_back(member);
            return BookingResult::waitlisted;
        }
        attendees_.push_back(member);
        return BookingResult::booked;
    }

    /// Remove @p member; the first person on the waiting list takes the free place.
    /// Returns the promoted member, if any.
    std::optional<std::string> cancel(const std::string& member) {
        if (std::erase(waitlist_, member) > 0) {
            return std::nullopt;
        }
        if (std::erase(attendees_, member) == 0) {
            throw std::invalid_argument(member + " is not booked on " + name_);
        }
        if (waitlist_.empty()) {
            return std::nullopt;
        }
        std::string promoted = waitlist_.front();
        waitlist_.erase(waitlist_.begin());
        attendees_.push_back(promoted);
        return promoted;
    }

    bool is_full() const { return static_cast<int>(attendees_.size()) >= capacity_; }
    int places_left() const { return capacity_ - static_cast<int>(attendees_.size()); }
    const std::string& name() const { return name_; }
    TimeSlot start() const { return start_; }
    const std::vector<std::string>& attendees() const { return attendees_; }
    const std::vector<std::string>& waitlist() const { return waitlist_; }

    /// True when the class invariant holds – useful in tests and debug checks.
    bool invariant_holds() const {
        if (static_cast<int>(attendees_.size()) > capacity_) {
            return false;
        }
        std::vector<std::string> everyone = attendees_;
        everyone.insert(everyone.end(), waitlist_.begin(), waitlist_.end());
        std::sort(everyone.begin(), everyone.end());
        return std::adjacent_find(everyone.begin(), everyone.end()) == everyone.end();
    }

private:
    static bool contains(const std::vector<std::string>& people, const std::string& member) {
        return std::find(people.begin(), people.end(), member) != people.end();
    }

    std::string name_;
    TimeSlot start_;
    int capacity_;
    std::vector<std::string> attendees_;
    std::vector<std::string> waitlist_;
};

/// "Spin 18:00 (2/3 booked, 1 waiting)".
inline std::ostream& operator<<(std::ostream& out, const FitnessClass& fitness_class) {
    return out << fitness_class.name() << ' ' << fitness_class.start() << " ("
               << fitness_class.attendees().size() << '/' << fitness_class.attendees().size() + static_cast<std::size_t>(fitness_class.places_left())
               << " booked, " << fitness_class.waitlist().size() << " waiting)";
}

/// The interactive demo: "book <name>" / "cancel <name>" on an 18:00 spin class with 3 places.
inline int run(std::istream& in, std::ostream& out) {
    out << "Day 28 – Creating Classes\n";
    FitnessClass spin("Spin", TimeSlot(18 * 60), 3);
    while (auto line = prompt_line(in, out, "book|cancel <name>> ")) {
        std::istringstream words(*line);
        std::string command;
        std::string member;
        if (!(words >> command >> member)) {
            break;
        }
        try {
            if (command == "book") {
                const auto result = spin.book(member);
                out << "  " << (result == BookingResult::booked       ? "booked"
                                : result == BookingResult::waitlisted ? "class full – waiting list"
                                                                      : "already booked")
                    << '\n';
            } else if (command == "cancel") {
                const auto promoted = spin.cancel(member);
                out << "  cancelled" << (promoted ? ", " + *promoted + " gets the place" : "") << '\n';
            } else {
                out << "  unknown command\n";
            }
        } catch (const std::invalid_argument& error) {
            out << "  " << error.what() << '\n';
        }
        out << "  " << spin << '\n';
    }
    return 0;
}

}  // namespace cppm::day28
