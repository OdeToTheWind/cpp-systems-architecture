// Tests for Day 50 – Exception Safety & RAII. Failures are injected to prove each guarantee.
#include <sstream>
#include <stdexcept>
#include <type_traits>

#include "cppm/testing.hpp"
#include "day_50_exception_safety/lesson.hpp"

using namespace cppm::day50;

namespace {
Ledger fresh() {
    Ledger ledger;
    ledger.open("alice", 10'000);
    ledger.open("bob", 5'000);
    return ledger;
}
}  // namespace

TEST_CASE("a scope guard runs its rollback unless dismissed") {
    int value = 1;
    {
        ScopeGuard guard([&] { value = 0; });
        value = 2;
    }
    CHECK_EQ(value, 0);
    {
        ScopeGuard guard([&] { value = 0; });
        value = 3;
        guard.dismiss();
    }
    CHECK_EQ(value, 3);
}

TEST_CASE("swap is declared noexcept") {
    static_assert(noexcept(std::declval<Ledger&>().swap(std::declval<Ledger&>())));
    Ledger a = fresh();
    Ledger b;
    a.swap(b);
    CHECK_EQ(b.total(), 15'000);
    CHECK_EQ(a.total(), 0);
}

TEST_CASE("all three transfers work when nothing fails") {
    for (int version = 0; version < 3; ++version) {
        Ledger ledger = fresh();
        if (version == 0) ledger.transfer_unsafe("alice", "bob", 2'500);
        if (version == 1) ledger.transfer_basic("alice", "bob", 2'500);
        if (version == 2) ledger.transfer_strong("alice", "bob", 2'500);
        CHECK_EQ(ledger.balance("alice"), 7'500);
        CHECK_EQ(ledger.balance("bob"), 7'500);
        CHECK_EQ(ledger.journal().size(), 1u);
    }
}

TEST_CASE("the unsafe transfer loses money when the journal write fails") {
    Ledger ledger = fresh();
    ledger.fail_next_journal_write();
    CHECK_THROWS_AS(ledger.transfer_unsafe("alice", "bob", 2'500), std::runtime_error);
    CHECK_EQ(ledger.total(), 12'500);  // 25.00 vanished
}

TEST_CASE("the basic guarantee keeps the ledger consistent") {
    Ledger ledger = fresh();
    ledger.fail_next_journal_write();
    CHECK_THROWS_AS(ledger.transfer_basic("alice", "bob", 2'500), std::runtime_error);
    CHECK_EQ(ledger.total(), 15'000);
    CHECK_EQ(ledger.balance("alice"), 10'000);
}

TEST_CASE("the strong guarantee leaves no trace of a failed transfer") {
    Ledger ledger = fresh();
    ledger.transfer_strong("alice", "bob", 1'000);
    ledger.fail_next_journal_write();
    CHECK_THROWS_AS(ledger.transfer_strong("alice", "bob", 2'500), std::runtime_error);
    CHECK_EQ(ledger.balance("alice"), 9'000);
    CHECK_EQ(ledger.balance("bob"), 6'000);
    CHECK_EQ(ledger.journal().size(), 1u);
    ledger.transfer_strong("alice", "bob", 500);  // the failure hook was used up
    CHECK_EQ(ledger.journal().size(), 2u);
}

TEST_CASE("validation errors change nothing in any version") {
    Ledger ledger = fresh();
    CHECK_THROWS_AS(ledger.transfer_strong("alice", "carol", 1), std::invalid_argument);
    CHECK_THROWS_AS(ledger.transfer_basic("alice", "bob", -5), std::invalid_argument);
    CHECK_THROWS_AS(ledger.transfer_unsafe("bob", "alice", 50'000), std::runtime_error);
    CHECK_EQ(ledger.total(), 15'000);
    CHECK(ledger.journal().empty());
}

TEST_CASE("run shows transfers committed or rolled back as a whole") {
    std::istringstream in("alice bob 2500\nalice bob 1000 fail\nbob alice 99999\nstop\n");
    std::ostringstream out;
    CHECK_EQ(run(in, out), 0);
    const auto text = out.str();
    CHECK(text.find("alice 7500, bob 7500, total 15000") != std::string::npos);
    CHECK(text.find("rolled back: journal is full") != std::string::npos);
    CHECK(text.find("rolled back: insufficient funds") != std::string::npos);
    CHECK(text.find("1 journal entry") != std::string::npos);
}
