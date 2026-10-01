// Tests for Day 17 – Vectors and Maps.
#include <sstream>
#include <string>
#include <vector>

#include "cppm/testing.hpp"
#include "day_17_vectors_maps/lesson.hpp"

using namespace cppm::day17;

TEST_CASE("add inserts new tools and updates existing ones") {
    Inventory inventory;
    inventory.add("drill", 2);
    inventory.add("drill", 1);
    inventory.add("saw", 1);
    inventory.add("ladder", 0);  // ignored
    CHECK_EQ(inventory.quantity("drill"), 3);
    CHECK_EQ(inventory.distinct_tools(), 2u);
}

TEST_CASE("quantity looks up without inserting") {
    Inventory inventory;
    CHECK_EQ(inventory.quantity("ghost"), 0);
    CHECK_EQ(inventory.distinct_tools(), 0u);
}

TEST_CASE("lend and give_back update counts and track borrowing") {
    Inventory inventory;
    inventory.add("drill", 1);
    CHECK(inventory.lend("drill"));
    CHECK(!inventory.lend("drill"));
    CHECK(!inventory.lend("saw"));
    inventory.give_back("drill");
    CHECK_EQ(inventory.quantity("drill"), 1);
    CHECK_EQ(inventory.borrow_counts().at("drill"), 1);
}

TEST_CASE("remove erases by key and reports unknown tools") {
    Inventory inventory;
    inventory.add("saw", 1);
    CHECK(inventory.remove("saw"));
    CHECK(!inventory.remove("saw"));
    CHECK_EQ(inventory.distinct_tools(), 0u);
}

TEST_CASE("report iterates in alphabetical key order") {
    Inventory inventory;
    inventory.add("wrench", 1);
    inventory.add("axe", 2);
    inventory.add("drill", 3);
    CHECK(inventory.report() == std::vector<std::string>{"axe: 2", "drill: 3", "wrench: 1"});
}

TEST_CASE("the waitlist keeps arrival order without duplicates") {
    Waitlist waitlist;
    CHECK(waitlist.join("Ana"));
    CHECK(waitlist.join("Ben"));
    CHECK(!waitlist.join("Ana"));
    CHECK(waitlist.join("Cy"));
    CHECK(waitlist.leave("Ben"));
    CHECK(!waitlist.leave("Ben"));
    CHECK_EQ(waitlist.next().value_or(""), "Ana");
    CHECK(waitlist.people() == std::vector<std::string>{"Cy"});
    waitlist.next();
    CHECK(!waitlist.next().has_value());
}

TEST_CASE("most_borrowed sorts by value and keeps ties alphabetical") {
    const std::map<std::string, int> counts{{"axe", 2}, {"drill", 5}, {"saw", 2}, {"tape", 1}};
    const auto top = most_borrowed(counts, 3);
    REQUIRE_EQ(top.size(), 3u);
    CHECK_EQ(top[0].first, "drill");
    CHECK_EQ(top[1].first, "axe");
    CHECK_EQ(top[2].first, "saw");
    CHECK_EQ(most_borrowed(counts, 10).size(), 4u);
}

TEST_CASE("operator[] on a missing key inserts it") {
    const auto [before, after] = brackets_insert_missing_keys();
    CHECK_EQ(before, 1u);
    CHECK_EQ(after, 2u);
}

TEST_CASE("run executes inventory and waitlist commands") {
    std::istringstream in("add drill 2\nlend drill\nlend drill\nlend drill\nwait Ana\nwait Ana\nnext\nnext\n"
                          "list\ntop\nremove saw\nfly\n\n");
    std::ostringstream out;
    CHECK_EQ(run(in, out), 0);
    const auto text = out.str();
    CHECK(text.find("none available: drill") != std::string::npos);
    CHECK(text.find("already waiting") != std::string::npos);
    CHECK(text.find("next: Ana") != std::string::npos);
    CHECK(text.find("next: (nobody)") != std::string::npos);
    CHECK(text.find("drill: 0") != std::string::npos);
    CHECK(text.find("drill x2") != std::string::npos);
    CHECK(text.find("unknown tool saw") != std::string::npos);
    CHECK(text.find("unknown command") != std::string::npos);
}
