// Tests for Day 78 – Unit Testing & Test Doubles. These tests are themselves the lesson:
// a fixture, a table-driven test, and one test per kind of double.
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

#include "cppm/testing.hpp"
#include "day_78_testing/lesson.hpp"

using namespace cppm::day78;

namespace {
/// Fixture: every test gets a fresh, known world in one line.
struct LoyaltyFixture {
    FixedClock clock{2};  // Tuesday
    InMemoryRepository customers;
    RecordingMailer mailer;
    LoyaltyService service{clock, customers, mailer};
    LoyaltyFixture() {
        customers.save({"ana", "ana@example.com", 250});
        customers.save({"ben", "ben@example.com", 995});
    }
};
}  // namespace

TEST_CASE("tier thresholds, table-driven") {
    struct Row {
        int points;
        Tier expected;
    };
    const std::vector<Row> table{{0, Tier::bronze},   {299, Tier::bronze}, {300, Tier::silver},
                                 {999, Tier::silver}, {1000, Tier::gold},  {50000, Tier::gold}};
    for (const auto& row : table) CHECK(tier_for(row.points) == row.expected);
    CHECK_THROWS_AS(tier_for(-1), std::invalid_argument);
}

TEST_CASE("points are one per euro, using the stub clock") {
    LoyaltyFixture f;
    CHECK_EQ(f.service.award("ana", 1299), 12);
    CHECK_EQ(f.customers.find("ana")->points, 262);
    f.clock.set(5);  // Friday
    CHECK_EQ(f.service.award("ana", 1000), 20);
}

TEST_CASE("the fake repository records saves") {
    LoyaltyFixture f;
    const int before = f.customers.saves();
    f.service.award("ana", 500);
    CHECK_EQ(f.customers.saves(), before + 1);
    CHECK_THROWS_AS(f.service.award("zoe", 100), std::out_of_range);
    CHECK_EQ(f.customers.saves(), before + 1);  // nothing saved for an unknown customer
}

TEST_CASE("the mock verifies that an upgrade sends exactly one email") {
    LoyaltyFixture f;
    f.service.award("ben", 300);  // 995 + 3 = 998: no upgrade
    CHECK(f.mailer.calls().empty());
    f.service.award("ben", 500);  // 1003: gold
    CHECK(f.mailer.sent_once_to("ben@example.com"));
    CHECK(f.mailer.calls()[0] == RecordingMailer::Call{"ben@example.com", "You reached gold!"});
}

TEST_CASE("a skipped tier still sends one email for the final tier") {
    LoyaltyFixture f;
    f.clock.set(5);
    f.service.award("ana", 40000);  // 250 + 800 = 1050: bronze straight to gold
    CHECK_EQ(f.mailer.calls().size(), 1u);
    CHECK_EQ(f.mailer.calls()[0].subject, "You reached gold!");
}

TEST_CASE("failures in a dependency surface, and points are already saved") {
    LoyaltyFixture f;
    f.mailer.fail_next();
    CHECK_THROWS_AS(f.service.award("ben", 1000), std::runtime_error);
    CHECK_EQ(f.customers.find("ben")->points, 1005);  // documents current behaviour: save happens before email
    CHECK_THROWS_AS(f.service.award("ben", -5), std::invalid_argument);
}

TEST_CASE("run drives the service with doubles") {
    std::istringstream in("1 c1 6000\n5 c2 1000\n3 nobody 100\nstop\n");
    std::ostringstream out;
    CHECK_EQ(run(in, out), 0);
    const auto text = out.str();
    CHECK(text.find("+60 -> 310 (silver)") != std::string::npos);
    CHECK(text.find("+20 -> 1010 (gold)") != std::string::npos);
    CHECK(text.find("unknown customer nobody") != std::string::npos);
    CHECK(text.find("email to ana@example.com: You reached silver!") != std::string::npos);
}
