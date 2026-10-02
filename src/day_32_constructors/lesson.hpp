/**
 * @file
 * Day 32 – Constructors and Initialiser Lists.
 *
 * Scenario: a *car-rental reservation desk*. Every way of creating a reservation – a blank
 * walk-in form, a full booking, a booking for "N days from a start day", or a copy of last
 * year's booking – goes through a constructor that guarantees a valid object from the very
 * first moment it exists.
 *
 * Deliverables (syllabus):
 * - Default, parameterised, delegating and copy constructors
 * - Member initialiser lists
 * - Validation at construction
 */
#pragma once

#include <istream>
#include <ostream>
#include <sstream>
#include <stdexcept>
#include <string>

#include "cppm/lesson.hpp"

namespace cppm::day32 {

inline constexpr Deliverable DELIVERABLES[] = {
    {"a parameterised constructor that validates everything", "Reservation::Reservation"},
    {"members initialised in the initialiser list, in declaration order", "Reservation::confirmation_"},
    {"a custom copy constructor with different copy semantics", "Reservation::copy_for_rebooking"},
    {"a default constructor for a blank walk-in form", "WalkInForm"},
    {"counting constructions to see which constructor ran", "ConstructionLog"},
};

/// Counts how many objects each constructor has created – for the lesson's tests and demo.
struct ConstructionLog {
    static inline int parameterised = 0;
    static inline int delegated = 0;
    static inline int copied = 0;
};

enum class CarClass { compact, estate, van };

inline long long daily_rate_cents(CarClass car) {
    return car == CarClass::compact ? 3'900 : car == CarClass::estate ? 5'500 : 8'900;
}

class Reservation {
  public:
    /// Parameterised constructor. The initialiser list initialises members directly (no default
    /// construction followed by assignment) – and const members *must* be initialised here.
    Reservation(std::string customer, CarClass car, int first_day, int last_day)
        : confirmation_(next_confirmation_++),
          customer_(std::move(customer)),
          car_(car),
          first_day_(first_day),
          last_day_(last_day),
          total_cents_(daily_rate_cents(car) * (last_day - first_day + 1)) {
        if (customer_.empty()) {
            throw std::invalid_argument("a reservation needs a customer name");
        }
        if (first_day_ < 1 || last_day_ < first_day_ || last_day_ > 366) {
            throw std::invalid_argument("rental days must be 1 <= first <= last <= 366");
        }
        ++ConstructionLog::parameterised;
    }

    /// Delegating constructor: "N days from first_day" forwards to the main constructor.
    Reservation(std::string customer, CarClass car, int first_day, unsigned days)
        : Reservation(std::move(customer), car, first_day, first_day + static_cast<int>(days) - 1) {
        if (days == 0) {
            throw std::invalid_argument("a rental lasts at least one day");
        }
        ++ConstructionLog::delegated;
    }

    /// Copy constructor: a copy is a *new* reservation, so it gets its own confirmation number.
    Reservation(const Reservation& other)
        : confirmation_(next_confirmation_++),
          customer_(other.customer_),
          car_(other.car_),
          first_day_(other.first_day_),
          last_day_(other.last_day_),
          total_cents_(other.total_cents_) {
        ++ConstructionLog::copied;
    }
    Reservation& operator=(const Reservation&) = delete;  // const members cannot be reassigned anyway

    /// Next year's booking: copy, then the copy constructor issues a fresh confirmation number.
    Reservation copy_for_rebooking() const { return Reservation(*this); }

    int confirmation() const { return confirmation_; }
    const std::string& customer() const { return customer_; }
    int days() const { return last_day_ - first_day_ + 1; }
    long long total_cents() const { return total_cents_; }

  private:
    static inline int next_confirmation_ = 1000;

    // Members are initialised in THIS order, whatever order the initialiser list uses (-Wreorder warns).
    const int confirmation_;
    std::string customer_;
    CarClass car_;
    int first_day_;
    int last_day_;
    long long total_cents_;
};

/// A blank form handed to walk-in customers: every member has a default, so `WalkInForm form;` is valid.
struct WalkInForm {
    std::string customer;
    CarClass car = CarClass::compact;
    int first_day = 1;
    int days = 1;

    /// Turn a filled-in form into a reservation (validation happens in the constructor).
    Reservation submit() const { return Reservation(customer, car, first_day, static_cast<unsigned>(days)); }
};

/// The interactive demo: "name compact|estate|van first_day days".
inline int run(std::istream& in, std::ostream& out) {
    out << "Day 32 – Constructors and Initialiser Lists\n";
    while (auto line = prompt_line(in, out, "name class first_day days> ")) {
        if (line->empty()) {
            break;
        }
        std::istringstream words(*line);
        WalkInForm form;  // default constructor
        std::string car;
        if (!(words >> form.customer >> car >> form.first_day >> form.days) || form.days < 1) {
            out << "  e.g. Ada estate 120 3\n";
            continue;
        }
        form.car = car == "van" ? CarClass::van : car == "estate" ? CarClass::estate : CarClass::compact;
        try {
            const Reservation booking = form.submit();
            const Reservation next_year = booking.copy_for_rebooking();
            out << "  #" << booking.confirmation() << " " << booking.customer() << ", " << booking.days() << " day(s), "
                << booking.total_cents() << " cents (next year: #" << next_year.confirmation() << ")\n";
        } catch (const std::invalid_argument& error) {
            out << "  " << error.what() << '\n';
        }
    }
    return 0;
}

}  // namespace cppm::day32
