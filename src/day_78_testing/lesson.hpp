/**
 * @file
 * Day 78 – Unit Testing & Test Doubles.
 *
 * Scenario: the *loyalty-points service* of a coffee chain. Awarding points depends on today's
 * date (double points on Fridays), a customer database and an email service – none of which a
 * unit test should touch. The service receives its dependencies through interfaces
 * (dependency injection), so tests swap in a stub clock, a fake in-memory database and a mock
 * mailer that records what it was asked to send.
 *
 * Deliverables (syllabus):
 * - Test fixtures
 * - Table-driven tests
 * - Fakes, stubs and mocks
 * - Dependency injection
 */
#pragma once

#include <istream>
#include <map>
#include <optional>
#include <ostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include "cppm/lesson.hpp"

namespace cppm::day78 {

inline constexpr Deliverable DELIVERABLES[] = {
    {"pure rules that suit table-driven tests", "tier_for"},
    {"dependencies expressed as interfaces", "CustomerRepository"},
    {"a service that receives its dependencies (dependency injection)", "LoyaltyService"},
    {"a stub clock with a fixed answer", "FixedClock"},
    {"a fake in-memory repository", "InMemoryRepository"},
    {"a mock mailer that records and verifies calls", "RecordingMailer"},
};

enum class Tier { bronze, silver, gold };

inline std::string tier_name(Tier tier) {
    return tier == Tier::gold ? "gold" : tier == Tier::silver ? "silver" : "bronze";
}

/// Pure function of the points balance – ideal for a table of input/expected pairs.
inline Tier tier_for(int points) {
    if (points < 0) throw std::invalid_argument("points cannot be negative");
    if (points >= 1000) return Tier::gold;
    if (points >= 300) return Tier::silver;
    return Tier::bronze;
}

struct Customer {
    std::string id;
    std::string email;
    int points = 0;
};

// ---- The seams: interfaces for everything outside the business logic ----
class Clock {
public:
    virtual ~Clock() = default;
    virtual int weekday() const = 0;  // 1 = Monday … 7 = Sunday
};

class CustomerRepository {
public:
    virtual ~CustomerRepository() = default;
    virtual std::optional<Customer> find(const std::string& id) const = 0;
    virtual void save(const Customer& customer) = 0;
};

class Mailer {
public:
    virtual ~Mailer() = default;
    virtual void send(const std::string& to, const std::string& subject) = 0;
};

/// The unit under test. It owns no infrastructure: everything arrives through the constructor.
class LoyaltyService {
public:
    LoyaltyService(const Clock& clock, CustomerRepository& customers, Mailer& mailer)
        : clock_(clock), customers_(customers), mailer_(mailer) {}

    /// One point per whole euro spent, doubled on Fridays. A tier upgrade triggers an email.
    /// Returns the points awarded.
    int award(const std::string& customer_id, int cents) {
        if (cents < 0) throw std::invalid_argument("refunds are not handled here");
        auto customer = customers_.find(customer_id);
        if (!customer) throw std::out_of_range("unknown customer " + customer_id);
        int points = cents / 100;
        if (clock_.weekday() == 5) points *= 2;
        const Tier before = tier_for(customer->points);
        customer->points += points;
        customers_.save(*customer);
        if (const Tier after = tier_for(customer->points); after > before) {
            mailer_.send(customer->email, "You reached " + tier_name(after) + "!");
        }
        return points;
    }

private:
    const Clock& clock_;
    CustomerRepository& customers_;
    Mailer& mailer_;
};

// ---- Test doubles (they would normally live with the tests; here they are the lesson) ----

/// Stub: returns a canned answer, nothing more.
class FixedClock : public Clock {
public:
    explicit FixedClock(int weekday) : weekday_(weekday) {}
    int weekday() const override { return weekday_; }
    void set(int weekday) { weekday_ = weekday; }

private:
    int weekday_;
};

/// Fake: a working but simplified implementation – a map instead of a database.
class InMemoryRepository : public CustomerRepository {
public:
    std::optional<Customer> find(const std::string& id) const override {
        const auto it = rows_.find(id);
        return it == rows_.end() ? std::nullopt : std::optional<Customer>(it->second);
    }
    void save(const Customer& customer) override {
        rows_[customer.id] = customer;
        ++saves_;
    }
    int saves() const { return saves_; }

private:
    std::map<std::string, Customer> rows_;
    int saves_ = 0;
};

/// Mock: records every interaction so a test can verify *how* the dependency was used.
class RecordingMailer : public Mailer {
public:
    struct Call {
        std::string to;
        std::string subject;
        bool operator==(const Call&) const = default;
    };
    void send(const std::string& to, const std::string& subject) override {
        if (fail_next_) {
            fail_next_ = false;
            throw std::runtime_error("SMTP unavailable");
        }
        calls_.push_back({to, subject});
    }
    const std::vector<Call>& calls() const { return calls_; }
    void fail_next() { fail_next_ = true; }
    /// Verification helper: exactly one email, to this address.
    bool sent_once_to(const std::string& to) const { return calls_.size() == 1 && calls_[0].to == to; }

private:
    std::vector<Call> calls_;
    bool fail_next_ = false;
};

/// The interactive demo runs the service against the doubles: "day customer cents".
inline int run(std::istream& in, std::ostream& out) {
    out << "Day 78 – Unit Testing & Test Doubles\n";
    FixedClock clock(1);
    InMemoryRepository customers;
    customers.save({"c1", "ana@example.com", 250});
    customers.save({"c2", "ben@example.com", 990});
    RecordingMailer mailer;
    LoyaltyService service(clock, customers, mailer);
    while (auto line = prompt_line(in, out, "weekday customer cents> ")) {
        std::istringstream words(*line);
        int day = 0;
        std::string id;
        int cents = 0;
        if (!(words >> day >> id >> cents)) break;
        clock.set(day);
        try {
            const int points = service.award(id, cents);
            out << "  +" << points << " -> " << customers.find(id)->points << " (" << tier_name(tier_for(customers.find(id)->points))
                << ")\n";
        } catch (const std::exception& error) {
            out << "  " << error.what() << '\n';
        }
    }
    for (const auto& call : mailer.calls()) out << "email to " << call.to << ": " << call.subject << '\n';
    return 0;
}

}  // namespace cppm::day78
