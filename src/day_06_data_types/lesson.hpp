/**
 * @file
 * Day 06 – Data Types & Fixed-width Integers.
 *
 * Scenario: a *weather-station firmware memory planner*. The microcontroller has a few
 * kilobytes of RAM, so every sensor field must use the smallest integer type that can hold
 * its range – and the team must know exactly when a counter will wrap around.
 *
 * Deliverables (syllabus):
 * - Fundamental types and modifiers
 * - sizeof and numeric_limits
 * - Fixed-width integers
 * - Signed vs unsigned wrap-around
 */
#pragma once

#include <cstddef>
#include <cstdint>
#include <iomanip>
#include <istream>
#include <limits>
#include <optional>
#include <ostream>
#include <sstream>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "cppm/lesson.hpp"

namespace cppm::day06 {

inline constexpr Deliverable DELIVERABLES[] = {
    {"one row per fundamental type and modifier", "type_table"},
    {"sizeof and std::numeric_limits for any type", "describe_type"},
    {"choosing the smallest fixed-width integer", "smallest_fitting_type"},
    {"unsigned arithmetic wraps modulo 2^N", "counter_after"},
    {"floating-point precision limits", "float_loses_integer"},
    {"a RAM budget built from fixed-width types", "memory_plan"},
};

/// Size and range of one type, with limits printed as numbers (never as raw characters).
struct TypeInfo {
    std::string name;
    std::size_t bytes;
    std::string min;
    std::string max;
    bool is_signed;
};

/// Describe @p T. Character types are promoted with unary + so they print as numbers.
template <typename T>
TypeInfo describe_type(std::string name) {
    using limits = std::numeric_limits<T>;
    std::ostringstream lo;
    std::ostringstream hi;
    if constexpr (limits::is_integer) {
        lo << +limits::min();
        hi << +limits::max();
    } else {
        lo << std::setprecision(3) << limits::lowest();
        hi << std::setprecision(3) << limits::max();
    }
    return {std::move(name), sizeof(T), lo.str(), hi.str(), limits::is_signed};
}

/// Every fundamental arithmetic type with its modifiers, then the fixed-width aliases.
inline std::vector<TypeInfo> type_table() {
    return {
        describe_type<bool>("bool"),
        describe_type<char>("char"),
        describe_type<signed char>("signed char"),
        describe_type<unsigned char>("unsigned char"),
        describe_type<short>("short"),
        describe_type<unsigned short>("unsigned short"),
        describe_type<int>("int"),
        describe_type<unsigned int>("unsigned int"),
        describe_type<long>("long"),
        describe_type<unsigned long>("unsigned long"),
        describe_type<long long>("long long"),
        describe_type<unsigned long long>("unsigned long long"),
        describe_type<float>("float"),
        describe_type<double>("double"),
        describe_type<long double>("long double"),
        describe_type<std::int8_t>("std::int8_t"),
        describe_type<std::uint8_t>("std::uint8_t"),
        describe_type<std::int16_t>("std::int16_t"),
        describe_type<std::uint16_t>("std::uint16_t"),
        describe_type<std::int32_t>("std::int32_t"),
        describe_type<std::uint32_t>("std::uint32_t"),
        describe_type<std::int64_t>("std::int64_t"),
        describe_type<std::uint64_t>("std::uint64_t"),
    };
}

template <typename T>
constexpr bool fits(long long low, long long high) {
    // std::cmp_* compare signed and unsigned values by value, never by converted bit patterns
    return std::cmp_greater_equal(low, std::numeric_limits<T>::min()) &&
           std::cmp_less_equal(high, std::numeric_limits<T>::max());
}

/// Smallest fixed-width type holding [low, high]; unsigned types are preferred for ranges starting at 0.
inline std::string_view smallest_fitting_type(long long low, long long high) {
    if (low > high) {
        return "invalid range";
    }
    if (low >= 0) {
        if (fits<std::uint8_t>(low, high)) return "std::uint8_t";
        if (fits<std::uint16_t>(low, high)) return "std::uint16_t";
        if (fits<std::uint32_t>(low, high)) return "std::uint32_t";
        return "std::uint64_t";
    }
    if (fits<std::int8_t>(low, high)) return "std::int8_t";
    if (fits<std::int16_t>(low, high)) return "std::int16_t";
    if (fits<std::int32_t>(low, high)) return "std::int32_t";
    return "std::int64_t";
}

/// Byte size of a fixed-width type name returned by smallest_fitting_type.
inline std::size_t bytes_of(std::string_view type) {
    if (type.find("8_t") != std::string_view::npos) return 1;
    if (type.find("16_t") != std::string_view::npos) return 2;
    if (type.find("32_t") != std::string_view::npos) return 4;
    return 8;
}

/// An 8-bit packet counter after @p increments: unsigned overflow is defined and wraps modulo 256.
inline std::uint8_t counter_after(std::uint8_t start, unsigned increments) {
    std::uint8_t counter = start;
    for (unsigned i = 0; i < increments; ++i) {
        ++counter;  // 255 + 1 == 0 for unsigned types; for signed types this would be undefined behaviour
    }
    return counter;
}

/// True when @p value cannot be stored exactly in a float (above 2^24 not every integer exists).
inline bool float_loses_integer(std::int32_t value) {
    const auto as_float = static_cast<float>(value);
    return static_cast<std::int64_t>(as_float) != value;
}

/// One sensor field the firmware stores for every reading.
struct Field {
    std::string name;
    long long low;
    long long high;
};

/// A field with its chosen type.
struct PlannedField {
    std::string name;
    std::string_view type;
    std::size_t bytes;
};

/// Choose a type per field and total the bytes for @p readings stored readings.
inline std::vector<PlannedField> memory_plan(const std::vector<Field>& fields, std::size_t& total_bytes,
                                             std::size_t readings) {
    std::vector<PlannedField> plan;
    std::size_t per_reading = 0;
    for (const auto& field : fields) {
        const auto type = smallest_fitting_type(field.low, field.high);
        plan.push_back({field.name, type, bytes_of(type)});
        per_reading += bytes_of(type);
    }
    total_bytes = per_reading * readings;
    return plan;
}

/// The interactive demo: the type table, then "name low high" fields until a blank line.
inline int run(std::istream& in, std::ostream& out) {
    out << "Day 06 – Data Types & Fixed-width Integers\n";
    out << std::left << std::setw(20) << "type" << std::setw(7) << "bytes" << "range\n";
    for (const auto& info : type_table()) {
        out << std::setw(20) << info.name << std::setw(7) << info.bytes << info.min << " .. " << info.max << '\n';
    }
    out << "uint8_t counter: 250 + 10 increments = " << +counter_after(250, 10) << " (wrapped)\n";

    std::vector<Field> fields;
    out << "\nDescribe sensor fields as 'name low high' (blank line to plan):\n";
    while (auto line = prompt_line(in, out, "field> ")) {
        if (line->empty()) {
            break;
        }
        std::istringstream words(*line);
        Field field;
        if (words >> field.name >> field.low >> field.high && field.low <= field.high) {
            fields.push_back(field);
        } else {
            out << "  expected: name low high (low <= high)\n";
        }
    }
    std::size_t total = 0;
    for (const auto& planned : memory_plan(fields, total, 1000)) {
        out << "  " << std::setw(14) << planned.name << planned.type << '\n';
    }
    out << "1000 readings need " << total << " bytes\n";
    return 0;
}

}  // namespace cppm::day06
