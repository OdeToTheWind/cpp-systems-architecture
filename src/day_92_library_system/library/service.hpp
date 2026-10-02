// Module 3 – the service: the lending rules, written only against the ports.
#pragma once

#include <stdexcept>
#include <string>

#include "library/model.hpp"
#include "library/ports.hpp"

namespace cppm::day92 {

/// A request the rules refuse – distinct from bugs and infrastructure failures.
class LendingError : public std::runtime_error {
public:
    using std::runtime_error::runtime_error;
};

struct ReturnResult {
    int fine_cents = 0;
    std::string next_reader;  // member notified because they reserved the book, if any
};

class LendingService {
public:
    LendingService(Repository& repo, Notifier& notifier, const Clock& clock, Policy policy = {})
        : repo_(repo), notifier_(notifier), clock_(clock), policy_(policy) {}

    Loan borrow(const std::string& member_id, const std::string& isbn);
    ReturnResult return_book(int loan_id);
    Loan renew(int loan_id);
    /// Join the queue for a book with no free copy; returns the queue position (1 = next).
    int reserve(const std::string& member_id, const std::string& isbn);
    int fine_for(const Loan& loan) const;
    int available(const std::string& isbn) const;

private:
    Member require_member(const std::string& id) const;
    Book require_book(const std::string& isbn) const;
    Repository& repo_;
    Notifier& notifier_;
    const Clock& clock_;
    Policy policy_;
};

}  // namespace cppm::day92
