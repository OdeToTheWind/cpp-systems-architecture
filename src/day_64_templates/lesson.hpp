/**
 * @file
 * Day 64 – Templates & Generic Programming.
 *
 * Scenario: the firmware library of a *weather station*. Temperature, wind-speed and status
 * sensors all keep their recent readings, and all need the same "latest N values, min/max/mean"
 * logic. Instead of three copies, one set of templates works for every value type, with
 * specialisations where a type needs different treatment.
 *
 * Deliverables (syllabus):
 * - Function and class templates
 * - Template argument deduction
 * - Specialisation
 * - Dependent names
 */
#pragma once

#include <array>
#include <cstddef>
#include <istream>
#include <optional>
#include <ostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include "cppm/lesson.hpp"

namespace cppm::day64 {

inline constexpr Deliverable DELIVERABLES[] = {
    {"a function template with argument deduction", "largest"},
    {"a class template with a non-type parameter", "RingBuffer"},
    {"class template argument deduction with a guide", "Reading"},
    {"full and partial specialisation", "TypeName"},
    {"dependent names with typename", "summarise"},
};

/// The largest element of a non-empty range; T is deduced from the argument.
template <typename T>
const T& largest(const std::vector<T>& values) {
    if (values.empty()) throw std::invalid_argument("largest() of an empty vector");
    const T* best = &values.front();
    for (const T& v : values) {
        if (*best < v) best = &v;  // only needs operator<
    }
    return *best;
}

/// Two different deduced types: the result type is whatever a + b gives.
template <typename A, typename B>
auto add(const A& a, const B& b) -> decltype(a + b) {
    return a + b;
}

/// Keeps the last Capacity values; the oldest is overwritten when full. Capacity is a
/// compile-time constant, so storage is a std::array with no heap allocation.
template <typename T, std::size_t Capacity>
class RingBuffer {
    static_assert(Capacity > 0, "a RingBuffer needs room for at least one value");

  public:
    using value_type = T;
    using size_type = std::size_t;

    void push(T value) {
        data_[(start_ + size_) % Capacity] = std::move(value);
        if (size_ < Capacity)
            ++size_;
        else
            start_ = (start_ + 1) % Capacity;
    }
    size_type size() const { return size_; }
    static constexpr size_type capacity() { return Capacity; }
    /// 0 is the oldest value still stored.
    const T& operator[](size_type i) const {
        if (i >= size_) throw std::out_of_range("RingBuffer index");
        return data_[(start_ + i) % Capacity];
    }
    const T& latest() const { return (*this)[size_ - 1]; }
    std::vector<T> to_vector() const {
        std::vector<T> out;
        for (size_type i = 0; i < size_; ++i) out.push_back((*this)[i]);
        return out;
    }

  private:
    std::array<T, Capacity> data_{};
    size_type start_ = 0;
    size_type size_ = 0;
};

/// A reading from a named sensor. `Reading r{"wind", 4.5}` deduces Reading<double>.
template <typename T>
struct Reading {
    std::string sensor;
    T value;
};
template <typename T>
Reading(const char*, T) -> Reading<T>;

/// Human-readable type names. The primary template is only declared, so an unsupported type
/// is a compile error rather than a wrong answer.
template <typename T>
struct TypeName;
template <>
struct TypeName<int> {
    static std::string get() { return "int"; }
};
template <>
struct TypeName<double> {
    static std::string get() { return "double"; }
};
template <>
struct TypeName<std::string> {
    static std::string get() { return "string"; }
};
/// Partial specialisation: any vector, built from its element's name.
template <typename T>
struct TypeName<std::vector<T>> {
    static std::string get() { return "vector<" + TypeName<T>::get() + ">"; }
};
template <typename T, std::size_t N>
struct TypeName<RingBuffer<T, N>> {
    static std::string get() { return "RingBuffer<" + TypeName<T>::get() + ", " + std::to_string(N) + ">"; }
};

template <typename T>
std::string type_name() {
    return TypeName<T>::get();
}

/// Summary for any container whose value_type supports < and +. `Container::value_type` is a
/// *dependent name* – it depends on the template parameter, so `typename` tells the compiler
/// it names a type.
template <typename Container>
struct Summary {
    typename Container::value_type min;
    typename Container::value_type max;
    double mean;
    std::size_t count;
};

template <typename Container>
std::optional<Summary<Container>> summarise(const Container& values) {
    using Value = typename Container::value_type;
    using Size = typename Container::size_type;
    if (values.size() == 0) return std::nullopt;
    Value lo = values[0];
    Value hi = values[0];
    double total = 0;
    for (Size i = 0; i < values.size(); ++i) {
        if (values[i] < lo) lo = values[i];
        if (hi < values[i]) hi = values[i];
        total += static_cast<double>(values[i]);
    }
    return Summary<Container>{lo, hi, total / static_cast<double>(values.size()),
                              static_cast<std::size_t>(values.size())};
}

/// Format any summary; works for ints, doubles and anything streamable.
template <typename Container>
std::string describe(const Summary<Container>& s) {
    std::ostringstream out;
    out << s.count << " x " << type_name<typename Container::value_type>() << ": min " << s.min << ", max " << s.max
        << ", mean " << s.mean;
    return out.str();
}

/// The interactive demo: "temp <°C>" or "wind <km/h>" readings; the last 5 of each are kept.
inline int run(std::istream& in, std::ostream& out) {
    out << "Day 64 – Templates & Generic Programming\n";
    RingBuffer<double, 5> temperatures;
    RingBuffer<int, 5> wind;
    while (auto line = prompt_line(in, out, "temp <C> | wind <kmh>> ")) {
        std::istringstream words(*line);
        std::string kind;
        double value = 0;
        if (!(words >> kind >> value)) break;
        if (kind == "temp")
            temperatures.push(value);
        else if (kind == "wind")
            wind.push(static_cast<int>(value));
        else
            out << "  unknown sensor " << kind << '\n';
    }
    if (const auto s = summarise(temperatures)) out << "temperature: " << describe(*s) << '\n';
    if (const auto s = summarise(wind)) out << "wind: " << describe(*s) << '\n';
    out << "buffers are " << type_name<decltype(temperatures)>() << " and " << type_name<decltype(wind)>() << '\n';
    return 0;
}

}  // namespace cppm::day64
