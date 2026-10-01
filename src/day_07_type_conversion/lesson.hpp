/**
 * @file
 * Day 07 – Type Conversion & Casting.
 *
 * Scenario: a *payment-terminal message decoder*. Amounts arrive as raw integers and bytes
 * from a card terminal; the decoder must convert them without silent truncation, talk to a
 * legacy C checksum API, inspect the wire bytes, and tell payment messages from refunds.
 *
 * Deliverables (syllabus):
 * - Implicit promotions
 * - static_cast, const_cast, reinterpret_cast and dynamic_cast
 * - Narrowing checks with brace initialisation
 */
#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <istream>
#include <limits>
#include <memory>
#include <ostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

#include "cppm/lesson.hpp"

namespace cppm::day07 {

inline constexpr Deliverable DELIVERABLES[] = {
    {"integer promotion: uint8_t + uint8_t is an int", "promotion_report"},
    {"static_cast to avoid integer division", "average_ticket"},
    {"checked narrowing that refuses lossy conversions", "narrow"},
    {"const_cast to call a non-const legacy C API", "checksum_of"},
    {"reinterpret_cast to inspect object bytes", "wire_bytes"},
    {"dynamic_cast to recognise a derived message", "describe"},
};

/// Shows what the usual arithmetic conversions produce for a few operand pairs.
inline std::vector<std::string> promotion_report() {
    const std::uint8_t a = 200;
    const std::uint8_t b = 100;
    const auto sum = a + b;  // both promoted to int first: 300, no wrap-around
    const short s = 1;
    const auto doubled = s * 2.0;  // int/short op double -> double
    const unsigned u = 1;
    const int negative = -1;
    const bool surprising = negative < static_cast<int>(u);  // compare as int on purpose
    return {
        std::string("uint8_t + uint8_t is ") + (std::is_same_v<decltype(sum), const int> ? "int" : "?") +
            " = " + std::to_string(sum),
        std::string("short * double is ") + (std::is_same_v<decltype(doubled), const double> ? "double" : "?"),
        std::string("-1 < 1u as int: ") + (surprising ? "true" : "false") +
            " (without the cast -1 converts to UINT_MAX and the answer is false)",
    };
}

/// Average ticket in currency units; static_cast makes the division happen in double.
inline double average_ticket(long long total_cents, int transactions) {
    if (transactions <= 0) {
        throw std::invalid_argument("no transactions");
    }
    return static_cast<double>(total_cents) / transactions / 100.0;
}

/// True for negative values; always false for unsigned types (without a "comparison is always false" warning).
template <typename T>
constexpr bool is_negative(T value) {
    if constexpr (std::is_signed_v<T>) {
        return value < T{};
    } else {
        return false;
    }
}

/// Convert @p value to @p To, throwing std::range_error if the value would change.
template <typename To, typename From>
To narrow(From value) {
    static_assert(std::is_arithmetic_v<To> && std::is_arithmetic_v<From>);
    const To converted = static_cast<To>(value);
    // a round trip that changes the value, or a flipped sign (-1 -> 4294967295u -> -1), means data loss
    if (static_cast<From>(converted) != value || is_negative(converted) != is_negative(value)) {
        throw std::range_error("narrowing conversion changed the value");
    }
    return converted;
}

/// A legacy C function: it takes `char*` although it never writes through it.
inline unsigned legacy_checksum(char* buffer, std::size_t size) {
    unsigned sum = 0;
    for (std::size_t i = 0; i < size; ++i) {
        sum = (sum * 31u + static_cast<unsigned char>(buffer[i])) & 0xFFFFu;
    }
    return sum;
}

/// Wraps the legacy API. const_cast is safe here only because legacy_checksum never modifies the data.
inline unsigned checksum_of(const std::string& message) {
    return legacy_checksum(const_cast<char*>(message.data()), message.size());
}

/// The four bytes of @p value in memory order. Reading an object through unsigned char* is allowed.
inline std::array<unsigned char, 4> wire_bytes(std::uint32_t value) {
    const auto* bytes = reinterpret_cast<const unsigned char*>(&value);
    return {bytes[0], bytes[1], bytes[2], bytes[3]};
}

/// True on little-endian machines (x86, ARM in its usual mode).
inline bool is_little_endian() { return wire_bytes(1u)[0] == 1; }

/// A message from the terminal. Polymorphic (virtual destructor) so dynamic_cast works.
class Message {
public:
    explicit Message(long long amount_cents) : amount_cents_(amount_cents) {}
    virtual ~Message() = default;
    long long amount_cents() const { return amount_cents_; }

private:
    long long amount_cents_;
};

class Payment : public Message {
public:
    using Message::Message;
};

class Refund : public Message {
public:
    Refund(long long amount_cents, std::string reason) : Message(amount_cents), reason_(std::move(reason)) {}
    const std::string& reason() const { return reason_; }

private:
    std::string reason_;
};

/// dynamic_cast returns nullptr when the object is not a Refund – safe runtime type checking.
inline std::string describe(const Message& message) {
    if (const auto* refund = dynamic_cast<const Refund*>(&message)) {
        return "refund of " + std::to_string(message.amount_cents()) + " cents (" + refund->reason() + ")";
    }
    if (dynamic_cast<const Payment*>(&message) != nullptr) {
        return "payment of " + std::to_string(message.amount_cents()) + " cents";
    }
    return "unknown message";
}

/// The interactive demo: type amounts in cents; each is narrowed into a 16-bit terminal field.
inline int run(std::istream& in, std::ostream& out) {
    out << "Day 07 – Type Conversion & Casting\n";
    for (const auto& line : promotion_report()) {
        out << "  " << line << '\n';
    }
    out << "  this machine is " << (is_little_endian() ? "little" : "big") << "-endian\n";
    std::vector<std::unique_ptr<Message>> messages;
    out << "Amounts in cents (negative = refund), blank line to finish\n";
    while (auto line = prompt_line(in, out, "amount> ")) {
        if (line->empty()) {
            break;
        }
        std::istringstream number(*line);
        long long cents{};
        if (!(number >> cents)) {
            out << "  not a number\n";
            continue;
        }
        try {
            const auto field = narrow<std::int16_t>(cents);  // the terminal only has 16 bits
            if (field < 0) {
                messages.push_back(std::make_unique<Refund>(-cents, "customer return"));
            } else {
                messages.push_back(std::make_unique<Payment>(cents));
            }
            out << "  " << describe(*messages.back()) << ", checksum " << checksum_of(*line) << '\n';
        } catch (const std::range_error&) {
            out << "  " << cents << " does not fit the terminal's 16-bit field\n";
        }
    }
    long long total = 0;
    for (const auto& message : messages) {
        total += message->amount_cents();
    }
    if (!messages.empty()) {
        out << "Average ticket: " << average_ticket(total, static_cast<int>(messages.size())) << '\n';
    }
    return 0;
}

}  // namespace cppm::day07
