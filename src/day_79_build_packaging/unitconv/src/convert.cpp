#include "unitconv/convert.hpp"

#include <map>
#include <stdexcept>
#include <utility>

#include "unitconv/version.hpp"

namespace unitconv {
namespace {

struct Unit {
    char dimension;  // 'L' length, 'M' mass
    double to_base;  // metres or grams
};

const std::map<std::string, Unit>& linear_units() {
    static const std::map<std::string, Unit> units{{"m", {'L', 1.0}},      {"km", {'L', 1000.0}}, {"mi", {'L', 1609.344}},
                                                   {"ft", {'L', 0.3048}},  {"g", {'M', 1.0}},     {"kg", {'M', 1000.0}},
                                                   {"lb", {'M', 453.59237}}};
    return units;
}

double to_kelvin(double value, const std::string& unit) {
    if (unit == "K") return value;
    if (unit == "C") return value + 273.15;
    return (value - 32.0) * 5.0 / 9.0 + 273.15;  // F
}

double from_kelvin(double kelvin, const std::string& unit) {
    if (unit == "K") return kelvin;
    if (unit == "C") return kelvin - 273.15;
    return (kelvin - 273.15) * 9.0 / 5.0 + 32.0;  // F
}

bool is_temperature(const std::string& unit) { return unit == "C" || unit == "F" || unit == "K"; }

}  // namespace

double convert(double value, const std::string& from, const std::string& to) {
    if (is_temperature(from) && is_temperature(to)) return from_kelvin(to_kelvin(value, from), to);
    const auto& units = linear_units();
    const auto a = units.find(from);
    const auto b = units.find(to);
    if (a == units.end() || b == units.end()) throw std::invalid_argument("unknown unit '" + (a == units.end() ? from : to) + "'");
    if (a->second.dimension != b->second.dimension) throw std::invalid_argument("cannot convert " + from + " to " + to);
    return value * a->second.to_base / b->second.to_base;
}

const char* library_version() { return UNITCONV_VERSION; }

}  // namespace unitconv
