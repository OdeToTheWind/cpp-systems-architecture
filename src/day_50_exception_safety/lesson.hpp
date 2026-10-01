/**
 * @file
 * Day 50 – Exception Safety & RAII.
 *
 * Scenario: a *credit-union ledger*. A transfer touches two accounts and an audit journal; if
 * anything throws halfway – a limit check, a full journal, a failing audit write – the ledger
 * must never lose or create money. The lesson compares an unsafe transfer with versions that
 * give the basic and the strong exception guarantee.
 *
 * Deliverables (syllabus):
 * - Basic, strong and nothrow guarantees
 * - Scope guards
 * - Copy-and-swap
 * - noexcept
 */
#pragma once

#include <functional>
#include <istream>
#include <map>
#include <numeric>
#include <ostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include "cppm/lesson.hpp"

namespace cppm::day50 {

inline constexpr Deliverable DELIVERABLES[] = {
    {"a scope guard that rolls back unless dismissed", "ScopeGuard"},
    {"a noexcept swap used for commit", "Ledger::swap"},
    {"the unsafe version that can lose money", "Ledger::transfer_unsafe"},
    {"the basic guarantee: valid, but maybe changed", "Ledger::transfer_basic"},
    {"the strong guarantee via copy-and-swap", "Ledger::transfer_strong"},
};

/// Runs a rollback action in its destructor unless dismiss() was called – RAII for "undo".
class ScopeGuard {
public:
    explicit ScopeGuard(std::function<void()> rollback) : rollback_(std::move(rollback)) {}
    ~ScopeGuard() {
        if (active_) {
            rollback_();
        }
    }
    void dismiss() noexcept { active_ = false; }
    ScopeGuard(const ScopeGuard&) = delete;
    ScopeGuard& operator=(const ScopeGuard&) = delete;

private:
    std::function<void()> rollback_;
    bool active_{true};
};

class Ledger {
public:
    void open(const std::string& account, long long cents) { balances_[account] = cents; }
    long long balance(const std::string& account) const { return balances_.at(account); }
    long long total() const {
        return std::accumulate(balances_.begin(), balances_.end(), 0LL,
                               [](long long sum, const auto& entry) { return sum + entry.second; });
    }
    const std::vector<std::string>& journal() const { return journal_; }

    /// Test hook: make the next journal write throw, as a full disk would.
    void fail_next_journal_write() { fail_journal_ = true; }

    /// Exchanging two ledgers cannot fail: it only swaps internal pointers. noexcept says so.
    void swap(Ledger& other) noexcept {
        balances_.swap(other.balances_);
        journal_.swap(other.journal_);
        std::swap(fail_journal_, other.fail_journal_);
    }

    /// NO guarantee: if the journal write throws, the money has left `from` and never arrived.
    void transfer_unsafe(const std::string& from, const std::string& to, long long cents) {
        check(from, to, cents);
        balances_.at(from) -= cents;
        write_journal(from + " -> " + to + ": " + std::to_string(cents));  // may throw
        balances_.at(to) += cents;
    }

    /// BASIC guarantee: on failure the ledger is consistent (no money lost), though the attempt
    /// may be visible. The scope guard puts the withdrawn money back.
    void transfer_basic(const std::string& from, const std::string& to, long long cents) {
        check(from, to, cents);
        balances_.at(from) -= cents;
        ScopeGuard refund([&] { balances_.at(from) += cents; });
        balances_.at(to) += cents;
        ScopeGuard reverse([&] { balances_.at(to) -= cents; });
        write_journal(from + " -> " + to + ": " + std::to_string(cents));
        reverse.dismiss();
        refund.dismiss();
    }

    /// STRONG guarantee: either everything happens or nothing does. Work on a copy, then
    /// commit with the noexcept swap – the commit step itself can never fail.
    void transfer_strong(const std::string& from, const std::string& to, long long cents) {
        Ledger copy = *this;
        copy.fail_journal_ = std::exchange(fail_journal_, false);  // the copy does the work, so it gets the hook
        copy.check(from, to, cents);
        copy.balances_.at(from) -= cents;
        copy.balances_.at(to) += cents;
        copy.write_journal(from + " -> " + to + ": " + std::to_string(cents));
        swap(copy);  // commit: noexcept
    }

private:
    void check(const std::string& from, const std::string& to, long long cents) const {
        if (!balances_.contains(from) || !balances_.contains(to)) throw std::invalid_argument("unknown account");
        if (cents <= 0) throw std::invalid_argument("amount must be positive");
        if (balances_.at(from) < cents) throw std::runtime_error("insufficient funds");
    }
    void write_journal(std::string entry) {
        if (fail_journal_) {
            fail_journal_ = false;
            throw std::runtime_error("journal is full");
        }
        journal_.push_back(std::move(entry));
    }

    std::map<std::string, long long> balances_;
    std::vector<std::string> journal_;
    bool fail_journal_{false};
};

/// The interactive demo: "transfer from to cents [fail]" with the strong guarantee.
inline int run(std::istream& in, std::ostream& out) {
    out << "Day 50 – Exception Safety & RAII\n";
    Ledger ledger;
    ledger.open("alice", 10'000);
    ledger.open("bob", 5'000);
    while (auto line = prompt_line(in, out, "from to cents [fail]> ")) {
        std::istringstream words(*line);
        std::string from;
        std::string to;
        long long cents = 0;
        std::string fail;
        if (!(words >> from >> to >> cents)) break;
        words >> fail;
        if (fail == "fail") ledger.fail_next_journal_write();
        try {
            ledger.transfer_strong(from, to, cents);
            out << "  done\n";
        } catch (const std::exception& error) {
            out << "  rolled back: " << error.what() << '\n';
        }
        out << "  alice " << ledger.balance("alice") << ", bob " << ledger.balance("bob") << ", total "
            << ledger.total() << '\n';
    }
    out << ledger.journal().size() << " journal entr" << (ledger.journal().size() == 1 ? "y" : "ies") << '\n';
    return 0;
}

}  // namespace cppm::day50
