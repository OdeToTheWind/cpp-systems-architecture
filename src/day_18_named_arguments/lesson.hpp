/**
 * @file
 * Day 18 – Positional and Named Arguments.
 *
 * Scenario: an *airline booking API*. The route is passed positionally (it is always needed),
 * optional extras have defaults, overloads accept a date in two shapes, and the many
 * optional settings travel in a parameter struct filled with C++20 designated initialisers –
 * C++'s closest equivalent to keyword arguments.
 *
 * Deliverables (syllabus):
 * - Positional parameters
 * - Default arguments
 * - Overloads
 * - Parameter structs and designated initialisers
 */
#pragma once

#include <istream>
#include <optional>
#include <ostream>
#include <sstream>
#include <stdexcept>
#include <string>

#include "cppm/lesson.hpp"

namespace cppm::day18 {

inline constexpr Deliverable DELIVERABLES[] = {
    {"positional parameters for required data", "base_fare_cents"},
    {"default arguments for optional trailing parameters", "seat_fee_cents"},
    {"overloads accepting different argument shapes", "make_date"},
    {"a parameter struct used with designated initialisers", "BookingOptions"},
    {"a function taking the parameter struct", "book_flight"},
    {"a fluent builder as the named-parameter idiom", "BookingBuilder"},
};

enum class Cabin { economy, premium, business };

/// Required, ordered data: the route always comes first, as positional parameters.
inline long long base_fare_cents(const std::string& from, const std::string& to, int distance_km) {
    if (from.size() != 3 || to.size() != 3 || from == to) {
        throw std::invalid_argument("routes use two different three-letter airport codes");
    }
    if (distance_km <= 0) {
        throw std::invalid_argument("distance must be positive");
    }
    return 2'000 + 9LL * distance_km;  // 20.00 plus 0.09 per km
}

/// Trailing parameters may have defaults; a caller can omit them from the right only.
inline long long seat_fee_cents(Cabin cabin = Cabin::economy, bool extra_legroom = false, bool window = false) {
    long long fee = cabin == Cabin::business ? 0 : cabin == Cabin::premium ? 1'500 : 0;
    if (extra_legroom && cabin == Cabin::economy) {
        fee += 2'500;
    }
    if (window && cabin != Cabin::business) {
        fee += 800;
    }
    return fee;
}

/// A calendar date as "YYYY-MM-DD".
struct Date {
    int year;
    int month;
    int day;
};

/// Overload 1: three integers.
inline Date make_date(int year, int month, int day) {
    if (month < 1 || month > 12 || day < 1 || day > 31) {
        throw std::invalid_argument("not a valid date");
    }
    return {year, month, day};
}

/// Overload 2: the same date as text.
inline Date make_date(const std::string& iso) {
    std::istringstream in(iso);
    int year{};
    int month{};
    int day{};
    char dash1{};
    char dash2{};
    if (!(in >> year >> dash1 >> month >> dash2 >> day) || dash1 != '-' || dash2 != '-') {
        throw std::invalid_argument("expected YYYY-MM-DD");
    }
    return make_date(year, month, day);  // reuse the validating overload
}

/// Every optional setting has a default; callers name only what they change:
/// `book_flight("LHR", "JFK", 5'540, {.cabin = Cabin::premium, .bags = 2})`.
struct BookingOptions {
    Cabin cabin = Cabin::economy;
    int bags = 0;
    bool extra_legroom = false;
    bool window = false;
    bool flexible = false;
    std::optional<std::string> meal = std::nullopt;  // every member has a default, so any subset may be named
};

/// A confirmed booking.
struct Booking {
    std::string route;
    long long total_cents;
    std::string summary;
};

/// Positional route + one struct of named options.
inline Booking book_flight(const std::string& from, const std::string& to, int distance_km,
                           const BookingOptions& options = {}) {
    if (options.bags < 0 || options.bags > 3) {
        throw std::invalid_argument("between 0 and 3 checked bags");
    }
    long long total = base_fare_cents(from, to, distance_km);
    if (options.cabin == Cabin::premium) {
        total = total * 3 / 2;
    } else if (options.cabin == Cabin::business) {
        total *= 3;
    }
    total += seat_fee_cents(options.cabin, options.extra_legroom, options.window);
    total += 3'500LL * options.bags;
    if (options.flexible) {
        total += total / 5;
    }
    std::string summary = std::to_string(options.bags) + " bag(s)";
    if (options.meal) {
        summary += ", meal: " + *options.meal;
    }
    if (options.flexible) {
        summary += ", flexible";
    }
    return {from + "-" + to, total, summary};
}

/// The classic Named Parameter Idiom: each setter returns *this, so calls chain by name.
class BookingBuilder {
  public:
    BookingBuilder(std::string from, std::string to, int distance_km)
        : from_(std::move(from)), to_(std::move(to)), distance_km_(distance_km) {}
    BookingBuilder& cabin(Cabin value) {
        options_.cabin = value;
        return *this;
    }
    BookingBuilder& bags(int value) {
        options_.bags = value;
        return *this;
    }
    BookingBuilder& window(bool value = true) {
        options_.window = value;
        return *this;
    }
    BookingBuilder& flexible(bool value = true) {
        options_.flexible = value;
        return *this;
    }
    BookingBuilder& meal(std::string value) {
        options_.meal = std::move(value);
        return *this;
    }
    Booking book() const { return book_flight(from_, to_, distance_km_, options_); }

  private:
    std::string from_;
    std::string to_;
    int distance_km_;
    BookingOptions options_;
};

/// The interactive demo: "FROM TO km [bags] [flex]" lines.
inline int run(std::istream& in, std::ostream& out) {
    out << "Day 18 – Positional and Named Arguments\n";
    const Booking sample = book_flight("LHR", "JFK", 5'540, {.cabin = Cabin::premium, .bags = 1, .window = true});
    out << "Sample: " << sample.route << " " << sample.total_cents << " cents (" << sample.summary << ")\n";
    while (auto line = prompt_line(in, out, "FROM TO km [bags] [flex]> ")) {
        if (line->empty()) {
            break;
        }
        std::istringstream words(*line);
        std::string from;
        std::string to;
        int km = 0;
        int bags = 0;
        std::string flex;
        if (!(words >> from >> to >> km)) {
            out << "  e.g. CDG FCO 1105 2 flex\n";
            continue;
        }
        words >> bags >> flex;
        try {
            const Booking booking = BookingBuilder(from, to, km).bags(bags).flexible(flex == "flex").book();
            out << "  " << booking.route << ": " << booking.total_cents << " cents (" << booking.summary << ")\n";
        } catch (const std::invalid_argument& error) {
            out << "  " << error.what() << '\n';
        }
    }
    return 0;
}

}  // namespace cppm::day18
