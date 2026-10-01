/**
 * @file
 * Day 54 – Date and Time with chrono.
 *
 * Scenario: a *freight-forwarding delivery estimator*. Orders placed after the warehouse
 * cut-off ship the next business day, transit counts business days only (no weekends, no
 * public holidays), and the promised pick-up time is shown in every partner office's local time.
 *
 * Deliverables (syllabus):
 * - Durations and time points
 * - Calendar dates
 * - Business-day arithmetic
 * - UTC offsets
 */
#pragma once

#include <chrono>
#include <cstdio>
#include <iomanip>
#include <istream>
#include <optional>
#include <ostream>
#include <set>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>

#include "cppm/lesson.hpp"

namespace cppm::day54 {

namespace chr = std::chrono;

inline constexpr Deliverable DELIVERABLES[] = {
    {"parsing and validating calendar dates", "parse_date"},
    {"weekday names from a date", "weekday_name"},
    {"business-day arithmetic that skips weekends and holidays", "add_business_days"},
    {"durations converted and formatted", "format_duration"},
    {"time points shifted by UTC offsets", "local_time"},
    {"a cut-off rule combining date and time of day", "ship_date"},
};

/// "2026-05-29" -> a valid year_month_day, or std::nullopt (e.g. for 2026-02-30).
inline std::optional<chr::year_month_day> parse_date(std::string_view text) {
    int y = 0;
    unsigned m = 0;
    unsigned d = 0;
    char dash1{};
    char dash2{};
    std::istringstream in{std::string(text)};
    if (!(in >> y >> dash1 >> m >> dash2 >> d) || dash1 != '-' || dash2 != '-' || !(in >> std::ws).eof()) {
        return std::nullopt;
    }
    const chr::year_month_day date{chr::year{y}, chr::month{m}, chr::day{d}};
    if (!date.ok()) return std::nullopt;  // ok() knows month lengths and leap years
    return date;
}

inline std::string format_date(chr::year_month_day date) {
    char buffer[16];
    std::snprintf(buffer, sizeof buffer, "%04d-%02u-%02u", static_cast<int>(date.year()), static_cast<unsigned>(date.month()),
                  static_cast<unsigned>(date.day()));
    return buffer;
}

inline std::string_view weekday_name(chr::year_month_day date) {
    static constexpr std::string_view names[] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};
    return names[chr::weekday{chr::sys_days{date}}.c_encoding()];
}

inline bool is_business_day(chr::sys_days day, const std::set<chr::sys_days>& holidays) {
    const chr::weekday wd{day};
    return wd != chr::Saturday && wd != chr::Sunday && !holidays.contains(day);
}

/// Move forward @p days business days (0 = the same day if it is a business day, else the next one).
inline chr::year_month_day add_business_days(chr::year_month_day start, int days, const std::set<chr::sys_days>& holidays) {
    if (days < 0) throw std::invalid_argument("days must not be negative");
    chr::sys_days current{start};
    while (!is_business_day(current, holidays)) current += chr::days{1};
    for (int counted = 0; counted < days;) {
        current += chr::days{1};
        if (is_business_day(current, holidays)) ++counted;
    }
    return chr::year_month_day{current};
}

/// 1 day 2 h 5 min -> "1d 02h 05m". Uses duration_cast to split one duration into units.
inline std::string format_duration(chr::minutes total) {
    if (total < chr::minutes{0}) throw std::invalid_argument("negative duration");
    const auto d = chr::duration_cast<chr::days>(total);
    const auto h = chr::duration_cast<chr::hours>(total - d);
    const auto m = total - d - h;
    char buffer[32];
    std::snprintf(buffer, sizeof buffer, "%lldd %02lldh %02lldm", static_cast<long long>(d.count()),
                  static_cast<long long>(h.count()), static_cast<long long>(m.count()));
    return buffer;
}

/// A UTC time point shown in a fixed UTC offset, e.g. +05:30 for Mumbai: "2026-05-29 15:30".
inline std::string local_time(chr::sys_time<chr::minutes> utc, chr::minutes offset) {
    const auto local = utc + offset;
    const auto day = chr::floor<chr::days>(local);
    const chr::hh_mm_ss<chr::minutes> clock{local - day};
    char buffer[8];
    std::snprintf(buffer, sizeof buffer, "%02d:%02d", static_cast<int>(clock.hours().count()),
                  static_cast<int>(clock.minutes().count()));
    return format_date(chr::year_month_day{day}) + ' ' + buffer;
}

/// Orders received after 15:00 UTC ship on the next business day.
inline chr::year_month_day ship_date(chr::sys_time<chr::minutes> ordered_utc, const std::set<chr::sys_days>& holidays) {
    const auto day = chr::floor<chr::days>(ordered_utc);
    const bool after_cutoff = ordered_utc - day >= chr::hours{15};
    return add_business_days(chr::year_month_day{day}, after_cutoff ? 1 : 0, holidays);
}

inline chr::sys_time<chr::minutes> at(chr::year_month_day date, int hour, int minute) {
    return chr::sys_days{date} + chr::hours{hour} + chr::minutes{minute};
}

/// The interactive demo: "YYYY-MM-DD HH:MM transit_days" orders.
inline int run(std::istream& in, std::ostream& out) {
    out << "Day 54 – Date and Time with chrono\n";
    using namespace std::chrono_literals;
    const std::set<chr::sys_days> holidays{chr::sys_days{2026y / chr::May / 25}, chr::sys_days{2026y / chr::December / 25}};
    while (auto line = prompt_line(in, out, "order date HH:MM transit_days> ")) {
        std::istringstream words(*line);
        std::string date_text;
        int hour = 0;
        int minute = 0;
        char colon{};
        int transit = 0;
        if (!(words >> date_text >> hour >> colon >> minute >> transit)) break;
        const auto date = parse_date(date_text);
        if (!date || colon != ':' || hour < 0 || hour > 23 || minute < 0 || minute > 59 || transit < 0) {
            out << "  invalid order\n";
            continue;
        }
        const auto ordered = at(*date, hour, minute);
        const auto ships = ship_date(ordered, holidays);
        const auto arrives = add_business_days(ships, transit, holidays);
        const auto pickup = at(arrives, 9, 0);
        out << "  ships " << format_date(ships) << " (" << weekday_name(ships) << "), arrives " << format_date(arrives)
            << " (" << weekday_name(arrives) << ")\n"
            << "  pick-up 09:00 UTC = New York " << local_time(pickup, -4h) << ", Mumbai " << local_time(pickup, 5h + 30min)
            << ", Tokyo " << local_time(pickup, 9h) << '\n'
            << "  door to door: " << format_duration(chr::duration_cast<chr::minutes>(pickup - ordered)) << '\n';
    }
    return 0;
}

}  // namespace cppm::day54
