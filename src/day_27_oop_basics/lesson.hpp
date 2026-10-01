/**
 * @file
 * Day 27 – Object-Oriented Programming Basics.
 *
 * Scenario: a *museum ticket kiosk payment gateway*. Cards, prepaid wallets and gift
 * vouchers are very different, yet the kiosk charges all of them through one abstract
 * interface – and a card number never leaves its object except as `**** 1111`.
 *
 * Deliverables (syllabus):
 * - Encapsulation
 * - Abstraction with pure virtual interfaces
 * - Polymorphism
 * - Information hiding
 */
#pragma once

#include <algorithm>
#include <cctype>
#include <istream>
#include <memory>
#include <ostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

#include "cppm/lesson.hpp"

namespace cppm::day27 {

inline constexpr Deliverable DELIVERABLES[] = {
    {"an abstract interface of pure virtual functions", "PaymentMethod"},
    {"information hiding: the card number never leaves the object", "CardPayment"},
    {"encapsulated state changed only through methods", "WalletPayment"},
    {"a third implementation with one-time state", "VoucherPayment"},
    {"polymorphism: one call, many behaviours", "Gateway::process"},
    {"validation hidden behind the interface", "luhn_valid"},
};

/// The outcome of one charge.
struct Receipt {
    bool approved;
    long long cents;
    std::string method;  // masked description, safe to print
    std::string reason;
};

/// Abstraction: what every payment method can do, with no hint of how.
class PaymentMethod {
public:
    virtual ~PaymentMethod() = default;  // deleting through a base pointer must run the derived destructor
    virtual std::string describe() const = 0;
    virtual Receipt charge(long long cents) = 0;

protected:
    PaymentMethod() = default;
    PaymentMethod(const PaymentMethod&) = default;
    PaymentMethod& operator=(const PaymentMethod&) = default;
};

/// The Luhn checksum every card number satisfies; it catches single-digit typos.
inline bool luhn_valid(std::string_view digits) {
    if (digits.size() < 12 || !std::all_of(digits.begin(), digits.end(), [](char c) {
            return std::isdigit(static_cast<unsigned char>(c)) != 0;
        })) {
        return false;
    }
    int sum = 0;
    bool double_it = false;
    for (auto it = digits.rbegin(); it != digits.rend(); ++it) {
        int digit = *it - '0';
        if (double_it) {
            digit *= 2;
            if (digit > 9) {
                digit -= 9;
            }
        }
        sum += digit;
        double_it = !double_it;
    }
    return sum % 10 == 0;
}

/// Holds a card number privately and exposes only a masked form.
class CardPayment final : public PaymentMethod {
public:
    CardPayment(std::string number, long long limit_cents) : number_(std::move(number)), limit_cents_(limit_cents) {
        number_.erase(std::remove(number_.begin(), number_.end(), ' '), number_.end());
        if (!luhn_valid(number_)) {
            throw std::invalid_argument("card number fails the Luhn check");
        }
    }
    std::string describe() const override { return "card **** " + number_.substr(number_.size() - 4); }
    Receipt charge(long long cents) override {
        if (cents > limit_cents_) {
            return {false, cents, describe(), "over the card limit"};
        }
        limit_cents_ -= cents;
        return {true, cents, describe(), ""};
    }

private:
    std::string number_;  // private: no getter exists, so it cannot leak into logs
    long long limit_cents_;
};

/// A prepaid wallet: the balance can only change through top_up() and charge().
class WalletPayment final : public PaymentMethod {
public:
    explicit WalletPayment(std::string owner) : owner_(std::move(owner)) {}
    void top_up(long long cents) {
        if (cents <= 0) {
            throw std::invalid_argument("top-up must be positive");
        }
        balance_cents_ += cents;
    }
    long long balance_cents() const { return balance_cents_; }
    std::string describe() const override { return "wallet of " + owner_; }
    Receipt charge(long long cents) override {
        if (cents > balance_cents_) {
            return {false, cents, describe(), "insufficient balance"};
        }
        balance_cents_ -= cents;
        return {true, cents, describe(), ""};
    }

private:
    std::string owner_;
    long long balance_cents_{0};
};

/// A gift voucher with a fixed value that can be used exactly once.
class VoucherPayment final : public PaymentMethod {
public:
    VoucherPayment(std::string code, long long value_cents) : code_(std::move(code)), value_cents_(value_cents) {}
    std::string describe() const override { return "voucher " + code_; }
    Receipt charge(long long cents) override {
        if (used_) {
            return {false, cents, describe(), "voucher already used"};
        }
        if (cents > value_cents_) {
            return {false, cents, describe(), "voucher value too low"};
        }
        used_ = true;
        return {true, cents, describe(), ""};
    }

private:
    std::string code_;
    long long value_cents_;
    bool used_{false};
};

/// The kiosk talks to PaymentMethod only; adding a new method needs no change here.
class Gateway {
public:
    /// One call, many behaviours: the right charge() runs through virtual dispatch.
    Receipt process(PaymentMethod& method, long long cents) {
        if (cents <= 0) {
            throw std::invalid_argument("amount must be positive");
        }
        Receipt receipt = method.charge(cents);
        history_.push_back(receipt);
        return receipt;
    }
    /// Read-only view of the history: callers can look but not edit.
    const std::vector<Receipt>& history() const { return history_; }
    long long approved_total() const {
        long long total = 0;
        for (const auto& r : history_) {
            total += r.approved ? r.cents : 0;
        }
        return total;
    }

private:
    std::vector<Receipt> history_;
};

/// The interactive demo: "card|wallet|voucher <amount-cents>".
inline int run(std::istream& in, std::ostream& out) {
    out << "Day 27 – Object-Oriented Programming Basics\n";
    Gateway gateway;
    CardPayment card("4111 1111 1111 1111", 50'000);
    WalletPayment wallet("Ada");
    wallet.top_up(2'000);
    VoucherPayment voucher("MUSEUM-2026", 1'500);
    while (auto line = prompt_line(in, out, "card|wallet|voucher <cents>> ")) {
        std::istringstream words(*line);
        std::string which;
        long long cents = 0;
        if (!(words >> which)) {
            break;
        }
        if (!(words >> cents)) {
            out << "  e.g. wallet 1200\n";
            continue;
        }
        PaymentMethod* method = which == "card" ? static_cast<PaymentMethod*>(&card)
                                : which == "wallet" ? static_cast<PaymentMethod*>(&wallet)
                                : which == "voucher" ? static_cast<PaymentMethod*>(&voucher)
                                                     : nullptr;
        if (method == nullptr) {
            out << "  unknown method\n";
            continue;
        }
        try {
            const Receipt r = gateway.process(*method, cents);
            out << "  " << r.method << ": " << (r.approved ? "approved" : "declined – " + r.reason) << '\n';
        } catch (const std::invalid_argument& error) {
            out << "  " << error.what() << '\n';
        }
    }
    out << "Approved today: " << gateway.approved_total() << " cents in " << gateway.history().size()
        << " attempt(s)\n";
    return 0;
}

}  // namespace cppm::day27
