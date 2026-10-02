// Module 1 – the domain model: plain data, no I/O, no dependencies.
#pragma once

#include <string>

namespace cppm::day92 {

/// A calendar date stored as days since 1970-01-01, so date arithmetic is integer arithmetic.
class Date {
  public:
    constexpr Date() = default;
    constexpr explicit Date(int days_since_epoch) : days_(days_since_epoch) {}
    static Date from_ymd(int year, int month, int day);
    std::string str() const;  // "2026-06-12"
    constexpr int days() const { return days_; }
    constexpr Date operator+(int n) const { return Date(days_ + n); }
    constexpr int operator-(Date other) const { return days_ - other.days_; }
    constexpr auto operator<=>(const Date&) const = default;

  private:
    int days_ = 0;
};

struct Book {
    std::string isbn;
    std::string title;
    int copies = 1;
};

struct Member {
    std::string id;
    std::string name;
    std::string email;
};

struct Loan {
    int id = 0;
    std::string isbn;
    std::string member;
    Date due;
    int renewals = 0;
};

/// Lending rules in one place, so tests and the service agree on them.
struct Policy {
    int loan_days = 14;
    int max_renewals = 2;
    int max_loans_per_member = 3;
    int fine_cents_per_day = 25;
    int fine_cap_cents = 1000;
};

}  // namespace cppm::day92
