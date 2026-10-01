/**
 * @file
 * Day 49 – Advanced Error Handling.
 *
 * Scenario: a *concert ticket booking service*. Business failures – sold out, seat taken,
 * payment declined – form an exception hierarchy callers can catch as broadly or as precisely
 * as they need; the low-level seat-code parser reports problems as std::error_code values from
 * its own error category; and a small Result type is used where failure is an ordinary outcome.
 *
 * Deliverables (syllabus):
 * - Custom exception hierarchies
 * - std::error_code
 * - Result types
 * - Choosing exceptions vs error values
 */
#pragma once

#include <cctype>
#include <istream>
#include <map>
#include <ostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <system_error>
#include <type_traits>
#include <variant>

#include "cppm/lesson.hpp"

namespace cppm::day49 {

inline constexpr Deliverable DELIVERABLES[] = {
    {"the root of a custom exception hierarchy", "BookingError"},
    {"specific exceptions callers may catch precisely", "SeatTaken"},
    {"an error-code enum and its category", "seat_errc"},
    {"a Result type for expected failures", "Result"},
    {"the low-level parser returning error codes", "parse_seat"},
    {"the high-level API that throws", "BoxOffice::book"},
};

// ── Exceptions: for failures the immediate caller usually cannot fix ─────────────

class BookingError : public std::runtime_error {
public:
    using std::runtime_error::runtime_error;
};
class SoldOut final : public BookingError {
public:
    SoldOut() : BookingError("the concert is sold out") {}
};
class SeatTaken final : public BookingError {
public:
    explicit SeatTaken(const std::string& seat) : BookingError("seat " + seat + " is already taken"), seat_(seat) {}
    const std::string& seat() const noexcept { return seat_; }

private:
    std::string seat_;
};
class PaymentDeclined final : public BookingError {
public:
    explicit PaymentDeclined(const std::string& reason) : BookingError("payment declined: " + reason) {}
};

// ── Error codes: cheap, no unwinding, ideal for parsers and hot paths ─────────────

enum class seat_errc { empty = 1, bad_row, bad_number, out_of_range };

class seat_category_impl final : public std::error_category {
public:
    const char* name() const noexcept override { return "seat"; }
    std::string message(int code) const override {
        switch (static_cast<seat_errc>(code)) {
            case seat_errc::empty:
                return "empty seat code";
            case seat_errc::bad_row:
                return "row must be a letter A-Z";
            case seat_errc::bad_number:
                return "seat number must be digits";
            case seat_errc::out_of_range:
                return "seat number must be 1-30";
        }
        return "unknown seat error";
    }
};

inline const std::error_category& seat_category() {
    static const seat_category_impl instance;
    return instance;
}

inline std::error_code make_error_code(seat_errc e) { return {static_cast<int>(e), seat_category()}; }

}  // namespace cppm::day49

template <>
struct std::is_error_code_enum<cppm::day49::seat_errc> : std::true_type {};  // enables `std::error_code ec = seat_errc::empty;`

namespace cppm::day49 {

/// Either a value or an error code – a minimal version of C++23 std::expected.
template <typename T>
class Result {
public:
    Result(T value) : data_(std::move(value)) {}
    Result(std::error_code error) : data_(error) {}
    /// Lets `return seat_errc::empty;` work: enum -> error_code -> Result would be two conversions.
    template <typename E>
        requires std::is_error_code_enum_v<E>
    Result(E error) : data_(std::error_code(error)) {}
    bool ok() const { return std::holds_alternative<T>(data_); }
    explicit operator bool() const { return ok(); }
    const T& value() const {
        if (!ok()) throw std::system_error(error());
        return std::get<T>(data_);
    }
    std::error_code error() const { return ok() ? std::error_code{} : std::get<std::error_code>(data_); }

private:
    std::variant<T, std::error_code> data_;
};

struct Seat {
    char row;
    int number;
    std::string code() const { return std::string(1, row) + std::to_string(number); }
};

/// "C12" -> Seat{'C', 12}. Bad input is normal here (people mistype), so no exceptions.
inline Result<Seat> parse_seat(const std::string& text) {
    if (text.empty()) return seat_errc::empty;
    const char row = static_cast<char>(std::toupper(static_cast<unsigned char>(text[0])));
    if (row < 'A' || row > 'Z') return seat_errc::bad_row;
    if (text.size() == 1 || text.size() > 3) return seat_errc::bad_number;
    int number = 0;
    for (std::size_t i = 1; i < text.size(); ++i) {
        if (!std::isdigit(static_cast<unsigned char>(text[i]))) return seat_errc::bad_number;
        number = number * 10 + (text[i] - '0');
    }
    if (number < 1 || number > 30) return seat_errc::out_of_range;
    return Seat{row, number};
}

/// The booking service: its public operations throw BookingError subclasses.
class BoxOffice {
public:
    explicit BoxOffice(int capacity) : capacity_(capacity) {}

    /// Book @p seat_text for @p customer paying with @p card. Throws SoldOut, SeatTaken,
    /// PaymentDeclined, or std::system_error (wrapping the parser's error code) for a bad seat code.
    std::string book(const std::string& seat_text, const std::string& customer, const std::string& card) {
        const auto parsed = parse_seat(seat_text);
        if (!parsed) {
            throw std::system_error(parsed.error(), "cannot book '" + seat_text + "'");
        }
        if (static_cast<int>(sold_.size()) >= capacity_) throw SoldOut();
        const std::string code = parsed.value().code();
        if (sold_.contains(code)) throw SeatTaken(code);
        if (card.size() != 16 || card.back() == '0') throw PaymentDeclined("card rejected by the bank");
        sold_[code] = customer;
        return code;
    }
    std::size_t sold() const { return sold_.size(); }

private:
    int capacity_;
    std::map<std::string, std::string> sold_;
};

/// The interactive demo: "seat customer card" lines, with every kind of failure reported.
inline int run(std::istream& in, std::ostream& out) {
    out << "Day 49 – Advanced Error Handling\n";
    BoxOffice office(3);
    while (auto line = prompt_line(in, out, "seat customer card> ")) {
        std::istringstream words(*line);
        std::string seat;
        std::string customer;
        std::string card;
        if (!(words >> seat >> customer >> card)) break;
        try {
            out << "  booked " << office.book(seat, customer, card) << " for " << customer << '\n';
        } catch (const SeatTaken& taken) {
            out << "  sorry, " << taken.seat() << " is taken – pick another seat\n";
        } catch (const BookingError& error) {  // every other business failure
            out << "  booking failed: " << error.what() << '\n';
        } catch (const std::system_error& error) {
            out << "  " << error.code().category().name() << " error " << error.code().value() << ": "
                << error.code().message() << '\n';
        }
    }
    out << office.sold() << " ticket(s) sold\n";
    return 0;
}

}  // namespace cppm::day49
