/**
 * @file
 * Day 70 – Compile-time Programming.
 *
 * Scenario: the telemetry firmware of a *soil-moisture sensor board*. Packets are sent over a
 * slow radio link, so the CRC lookup table is computed by the compiler instead of at boot,
 * configuration constants are validated at compile time, and one serialise function handles every
 * field type, choosing its encoding with type traits and `if constexpr`.
 *
 * Deliverables (syllabus):
 * - constexpr and consteval functions
 * - static_assert
 * - Type traits
 * - if constexpr
 */
#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <istream>
#include <ostream>
#include <sstream>
#include <string>
#include <string_view>
#include <type_traits>
#include <vector>

#include "cppm/lesson.hpp"

namespace cppm::day70 {

inline constexpr Deliverable DELIVERABLES[] = {
    {"a lookup table built by the compiler", "crc8_table"},
    {"a constexpr checksum usable at compile time and run time", "crc8"},
    {"a consteval check that rejects bad constants at compile time", "checked_baud"},
    {"a custom type trait", "is_fixed_size"},
    {"one serialiser that branches on types with if constexpr", "serialise"},
};

/// CRC-8 (polynomial 0x07) table: 256 entries computed during compilation.
constexpr std::array<std::uint8_t, 256> crc8_table() {
    std::array<std::uint8_t, 256> table{};
    for (std::size_t i = 0; i < table.size(); ++i) {
        auto crc = static_cast<std::uint8_t>(i);
        for (int bit = 0; bit < 8; ++bit) crc = static_cast<std::uint8_t>((crc & 0x80) ? (crc << 1) ^ 0x07 : crc << 1);
        table[i] = crc;
    }
    return table;
}
inline constexpr auto CRC8_TABLE = crc8_table();  // lives in read-only memory; no start-up cost

template <typename Bytes>
constexpr std::uint8_t crc8(const Bytes& data) {
    std::uint8_t crc = 0;
    for (const auto byte : data) crc = CRC8_TABLE[static_cast<std::uint8_t>(crc ^ static_cast<std::uint8_t>(byte))];
    return crc;
}

static_assert(crc8(std::string_view("123456789")) == 0xF4, "CRC-8 check value from the specification");

/// consteval: *must* run at compile time, so an invalid literal is a build error, not a bug report.
consteval std::uint32_t checked_baud(std::uint32_t rate) {
    constexpr std::uint32_t allowed[] = {9600, 19200, 38400, 57600, 115200};
    for (const auto a : allowed) {
        if (a == rate) return rate;
    }
    throw "unsupported baud rate";  // not a constant expression, so compilation fails
}

inline constexpr std::uint32_t RADIO_BAUD = checked_baud(38400);
// inline constexpr std::uint32_t BAD = checked_baud(38401);  // error: call to consteval function is not a constant expression

enum class Status : std::uint8_t { ok = 0, dry = 1, sensor_fault = 2 };

/// Custom trait: true for types with a fixed encoded size (arithmetic types and enums).
template <typename T>
struct is_fixed_size : std::bool_constant<std::is_arithmetic_v<T> || std::is_enum_v<T>> {};
template <typename T, std::size_t N>
struct is_fixed_size<std::array<T, N>> : is_fixed_size<T> {};
template <typename T>
inline constexpr bool is_fixed_size_v = is_fixed_size<T>::value;

/// Encoded size in bytes of a fixed-size type, computed at compile time.
template <typename T>
constexpr std::size_t encoded_size() {
    static_assert(is_fixed_size_v<T>, "only fixed-size types have a compile-time size");
    if constexpr (std::is_enum_v<T>) {
        return sizeof(std::underlying_type_t<T>);
    } else if constexpr (std::is_floating_point_v<T>) {
        return 2;  // sent as centi-units in an int16
    } else if constexpr (std::is_arithmetic_v<T>) {
        return sizeof(T);
    } else {
        return std::tuple_size_v<T> * encoded_size<typename T::value_type>();
    }
}

/// Append @p value in little-endian wire format. Each branch is compiled only for the types
/// that take it, so e.g. `.size()` is never instantiated for an int.
template <typename T>
void serialise(std::vector<std::uint8_t>& out, const T& value) {
    if constexpr (std::is_same_v<T, bool>) {
        out.push_back(value ? 1 : 0);
    } else if constexpr (std::is_enum_v<T>) {
        serialise(out, static_cast<std::underlying_type_t<T>>(value));
    } else if constexpr (std::is_floating_point_v<T>) {
        const double scaled = static_cast<double>(value) * 100.0;
        serialise(out, static_cast<std::int16_t>(scaled < 0 ? scaled - 0.5 : scaled + 0.5));
    } else if constexpr (std::is_integral_v<T>) {
        using U = std::make_unsigned_t<T>;
        const auto bits = static_cast<U>(value);
        for (std::size_t i = 0; i < sizeof(T); ++i) out.push_back(static_cast<std::uint8_t>(bits >> (8 * i)));
    } else if constexpr (std::is_convertible_v<T, std::string_view>) {
        const std::string_view text(value);
        out.push_back(static_cast<std::uint8_t>(text.size() > 255 ? 255 : text.size()));  // length prefix
        out.insert(out.end(), text.begin(), text.begin() + static_cast<std::ptrdiff_t>(out.back()));
    } else {
        static_assert(is_fixed_size_v<T>, "no encoding for this type");
        for (const auto& item : value) serialise(out, item);
    }
}

struct Reading {
    std::uint16_t board;
    float moisture_pct;
    float temperature_c;
    Status status;
    std::array<std::uint8_t, 3> depth_cm;
};

inline constexpr std::size_t READING_BYTES = encoded_size<std::uint16_t>() + 2 * encoded_size<float>() +
                                             encoded_size<Status>() + encoded_size<std::array<std::uint8_t, 3>>();
static_assert(READING_BYTES == 10, "the radio frame budget assumes 10-byte readings");

/// A complete packet: the fields followed by their CRC-8.
inline std::vector<std::uint8_t> packet(const Reading& r) {
    std::vector<std::uint8_t> out;
    serialise(out, r.board);
    serialise(out, r.moisture_pct);
    serialise(out, r.temperature_c);
    serialise(out, r.status);
    serialise(out, r.depth_cm);
    out.push_back(crc8(out));
    return out;
}

inline std::string hex(const std::vector<std::uint8_t>& bytes) {
    static constexpr char digits[] = "0123456789ABCDEF";
    std::string out;
    for (const auto b : bytes) {
        if (!out.empty()) out += ' ';
        out += digits[b >> 4];
        out += digits[b & 0xF];
    }
    return out;
}

/// The interactive demo: "board moisture temperature" -> the radio packet in hex.
inline int run(std::istream& in, std::ostream& out) {
    out << "Day 70 – Compile-time Programming\n";
    out << "radio at " << RADIO_BAUD << " baud, " << READING_BYTES << "-byte readings + 1 CRC byte\n";
    while (auto line = prompt_line(in, out, "board moisture temp> ")) {
        std::istringstream words(*line);
        unsigned board = 0;
        float moisture = 0;
        float temperature = 0;
        if (!(words >> board >> moisture >> temperature)) break;
        const Status status = moisture < 15 ? Status::dry : Status::ok;
        const auto bytes = packet({static_cast<std::uint16_t>(board), moisture, temperature, status, {10, 20, 40}});
        out << "  " << hex(bytes) << "  (" << bytes.size() << " bytes)\n";
    }
    return 0;
}

}  // namespace cppm::day70
