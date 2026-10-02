// Tests for Day 92 – Capstone: Multi-module Library with a Full Test Suite.
// Organised by module: model, ports (the fakes themselves), then the service rules.
#include <sstream>
#include <stdexcept>
#include <string>

#include "cppm/testing.hpp"
#include "day_92_library_system/lesson.hpp"

using namespace cppm::day92;

// ---- model ----
TEST_CASE("model: dates convert to and from the calendar") {
    CHECK_EQ(Date::from_ymd(1970, 1, 1).days(), 0);
    CHECK_EQ(Date::from_ymd(2026, 6, 12).str(), "2026-06-12");
    CHECK_EQ((Date::from_ymd(2026, 6, 12) + 20).str(), "2026-07-02");
    CHECK_EQ((Date::from_ymd(2024, 2, 28) + 1).str(), "2024-02-29");
    CHECK_EQ(Date::from_ymd(2026, 3, 1) - Date::from_ymd(2026, 2, 1), 28);
    CHECK_THROWS_AS(Date::from_ymd(2026, 13, 1), std::invalid_argument);
}

// ---- ports ----
TEST_CASE("ports: the fake repository behaves like a store") {
    InMemoryRepository repo;
    repo.add_book({"1", "A", 2});
    CHECK(repo.book("1").has_value());
    CHECK(!repo.book("2").has_value());
    const int a = repo.add_loan({0, "1", "m", Date(10), 0});
    const int b = repo.add_loan({0, "1", "n", Date(10), 0});
    CHECK(a != b);
    CHECK_EQ(repo.loans_of_book("1").size(), 2u);
    repo.remove_loan(a);
    CHECK_EQ(repo.loans_of_member("m").size(), 0u);
    FixedClock clock(Date(5));
    clock.advance(3);
    CHECK_EQ(clock.today().days(), 8);
}

// ---- service ----
TEST_CASE("service: borrowing sets the due date and respects stock") {
    DemoLibrary lib;
    const auto loan = lib.service.borrow("m1", "978-0441172719");
    CHECK_EQ(loan.due.str(), "2026-06-26");
    CHECK_EQ(lib.service.available("978-0441172719"), 1);
    lib.service.borrow("m2", "978-0441172719");
    CHECK_THROWS_AS(lib.service.borrow("m1", "978-0441172719"), LendingError);
    CHECK_THROWS_AS(lib.service.borrow("nobody", "978-0441172719"), LendingError);
    CHECK_THROWS_AS(lib.service.borrow("m1", "000"), LendingError);
}

TEST_CASE("service: limits and overdue books block new loans") {
    DemoLibrary lib;
    lib.repo.add_book({"x", "X", 5});
    for (int i = 0; i < 3; ++i) lib.service.borrow("m1", "x");
    CHECK_THROWS_AS(lib.service.borrow("m1", "x"), LendingError);
    lib.clock.advance(15);
    CHECK_THROWS_AS(lib.service.borrow("m1", "978-0441172719"), LendingError);  // overdue
}

TEST_CASE("service: fines grow per day up to a cap") {
    DemoLibrary lib;
    const auto loan = lib.service.borrow("m1", "978-0441172719");
    lib.clock.advance(14);
    CHECK_EQ(lib.service.fine_for(loan), 0);  // due today: not late yet
    lib.clock.advance(3);
    CHECK_EQ(lib.service.fine_for(loan), 75);
    lib.clock.advance(100);
    CHECK_EQ(lib.service.return_book(loan.id).fine_cents, 1000);
    CHECK_THROWS_AS(lib.service.return_book(loan.id), LendingError);
}

TEST_CASE("service: renewals extend from the due date until a limit or a reservation") {
    DemoLibrary lib;
    const auto loan = lib.service.borrow("m1", "978-0141439518");
    lib.clock.advance(10);
    CHECK_EQ(lib.service.renew(loan.id).due.str(), "2026-07-10");
    CHECK_EQ(lib.service.renew(loan.id).due.str(), "2026-07-24");
    CHECK_THROWS_AS(lib.service.renew(loan.id), LendingError);
    DemoLibrary other;
    const auto l2 = other.service.borrow("m1", "978-0141439518");
    other.service.reserve("m2", "978-0141439518");
    CHECK_THROWS_AS(other.service.renew(l2.id), LendingError);
}

TEST_CASE("service: reservations queue, notify on return and hold the copy") {
    DemoLibrary lib;
    const auto loan = lib.service.borrow("m1", "978-0141439518");
    CHECK_THROWS_AS(lib.service.reserve("m1", "978-0141439518"), LendingError);  // already has it
    CHECK_EQ(lib.service.reserve("m2", "978-0141439518"), 1);
    CHECK_THROWS_AS(lib.service.reserve("m2", "978-0141439518"), LendingError);
    const auto result = lib.service.return_book(loan.id);
    CHECK_EQ(result.next_reader, "m2");
    CHECK_EQ(lib.notifier.sent.size(), 1u);
    CHECK_EQ(lib.notifier.sent[0].text, "'Pride and Prejudice' is ready for you");
    CHECK_THROWS_AS(lib.service.borrow("m1", "978-0141439518"), LendingError);  // held for m2
    lib.service.borrow("m2", "978-0141439518");
    CHECK(lib.repo.reservations("978-0141439518").empty());
    std::istringstream in("borrow m1 978-0141439518\nreserve m2 978-0141439518\nwait 20\nreturn 1\nstop\n");
    std::ostringstream out;
    CHECK_EQ(run(in, out), 0);
    CHECK(out.str().find("returned; fine 150 cents") != std::string::npos);
    CHECK(out.str().find("email to ben@example.org: 'Pride and Prejudice' is ready for you") != std::string::npos);
}
