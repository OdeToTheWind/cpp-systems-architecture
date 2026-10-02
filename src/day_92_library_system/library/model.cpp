#include "library/model.hpp"

#include <stdexcept>

namespace cppm::day92 {

// Howard Hinnant's days_from_civil: proleptic Gregorian calendar, no time zones involved.
Date Date::from_ymd(int year, int month, int day) {
    if (month < 1 || month > 12 || day < 1 || day > 31) throw std::invalid_argument("invalid date");
    year -= month <= 2 ? 1 : 0;
    const int era = (year >= 0 ? year : year - 399) / 400;
    const int yoe = year - era * 400;
    const int doy = (153 * (month + (month > 2 ? -3 : 9)) + 2) / 5 + day - 1;
    const int doe = yoe * 365 + yoe / 4 - yoe / 100 + doy;
    return Date(era * 146097 + doe - 719468);
}

std::string Date::str() const {
    const int z = days_ + 719468;
    const int era = (z >= 0 ? z : z - 146096) / 146097;
    const int doe = z - era * 146097;
    const int yoe = (doe - doe / 1460 + doe / 36524 - doe / 146096) / 365;
    const int doy = doe - (365 * yoe + yoe / 4 - yoe / 100);
    const int mp = (5 * doy + 2) / 153;
    const int d = doy - (153 * mp + 2) / 5 + 1;
    const int m = mp < 10 ? mp + 3 : mp - 9;
    const int y = yoe + era * 400 + (m <= 2 ? 1 : 0);
    auto two = [](int v) { return (v < 10 ? "0" : "") + std::to_string(v); };
    return std::to_string(y) + "-" + two(m) + "-" + two(d);
}

}  // namespace cppm::day92
