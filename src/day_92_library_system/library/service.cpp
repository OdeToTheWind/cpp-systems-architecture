#include "library/service.hpp"

#include <algorithm>
#include <iterator>

namespace cppm::day92 {

Member LendingService::require_member(const std::string& id) const {
    const auto member = repo_.member(id);
    if (!member) throw LendingError("unknown member " + id);
    return *member;
}

Book LendingService::require_book(const std::string& isbn) const {
    const auto book = repo_.book(isbn);
    if (!book) throw LendingError("unknown book " + isbn);
    return *book;
}

int LendingService::available(const std::string& isbn) const {
    return require_book(isbn).copies - static_cast<int>(repo_.loans_of_book(isbn).size());
}

Loan LendingService::borrow(const std::string& member_id, const std::string& isbn) {
    require_member(member_id);
    require_book(isbn);
    const auto loans = repo_.loans_of_member(member_id);
    if (static_cast<int>(loans.size()) >= policy_.max_loans_per_member) throw LendingError("loan limit reached");
    if (std::any_of(loans.begin(), loans.end(), [&](const Loan& l) { return clock_.today() > l.due; })) {
        throw LendingError("return overdue books first");
    }
    auto& queue = repo_.reservations(isbn);
    // Free copies are held for people in the queue, in order.
    const int free = available(isbn);
    const auto position = std::find(queue.begin(), queue.end(), member_id);
    const auto ahead = static_cast<int>(std::distance(queue.begin(), position));  // whole queue if not in it
    if (free - ahead <= 0) throw LendingError(free > 0 ? "copies are held for earlier reservations" : "no copy available");
    if (position != queue.end()) queue.erase(position);
    Loan loan{0, isbn, member_id, clock_.today() + policy_.loan_days, 0};
    loan.id = repo_.add_loan(loan);
    return loan;
}

int LendingService::fine_for(const Loan& loan) const {
    const int late_days = clock_.today() - loan.due;
    return late_days <= 0 ? 0 : std::min(late_days * policy_.fine_cents_per_day, policy_.fine_cap_cents);
}

ReturnResult LendingService::return_book(int loan_id) {
    const auto loan = repo_.loan(loan_id);
    if (!loan) throw LendingError("no loan " + std::to_string(loan_id));
    ReturnResult result{fine_for(*loan), ""};
    repo_.remove_loan(loan_id);
    const auto& queue = repo_.reservations(loan->isbn);
    if (!queue.empty()) {
        const auto next = repo_.member(queue.front());
        if (next) {
            notifier_.notify(*next, "'" + require_book(loan->isbn).title + "' is ready for you");
            result.next_reader = next->id;
        }
    }
    return result;
}

Loan LendingService::renew(int loan_id) {
    auto loan = repo_.loan(loan_id);
    if (!loan) throw LendingError("no loan " + std::to_string(loan_id));
    if (loan->renewals >= policy_.max_renewals) throw LendingError("renewal limit reached");
    if (!repo_.reservations(loan->isbn).empty()) throw LendingError("someone is waiting for this book");
    if (clock_.today() > loan->due) throw LendingError("overdue loans cannot be renewed");
    loan->due = loan->due + policy_.loan_days;  // from the old due date, not from today
    ++loan->renewals;
    repo_.update_loan(*loan);
    return *loan;
}

int LendingService::reserve(const std::string& member_id, const std::string& isbn) {
    require_member(member_id);
    require_book(isbn);
    auto& queue = repo_.reservations(isbn);
    if (available(isbn) > static_cast<int>(queue.size())) throw LendingError("a copy is available – borrow it instead");
    if (std::find(queue.begin(), queue.end(), member_id) != queue.end()) throw LendingError("already reserved");
    const auto loans = repo_.loans_of_book(isbn);
    if (std::any_of(loans.begin(), loans.end(), [&](const Loan& l) { return l.member == member_id; })) {
        throw LendingError("you already have this book");
    }
    queue.push_back(member_id);
    return static_cast<int>(queue.size());
}

}  // namespace cppm::day92
