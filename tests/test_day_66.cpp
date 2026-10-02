// Tests for Day 66 – Ranges & Views.
#include <sstream>
#include <string>
#include <utility>
#include <vector>

#include "cppm/testing.hpp"
#include "day_66_ranges/lesson.hpp"

using namespace cppm::day66;

namespace {
std::vector<Order> sample() {
    return {{1, "ana", 7500, true, "gift,express"},
            {2, "ben", 2000, true, "express"},
            {3, "ana", 9900, false, "gift"},
            {4, "cleo", 12000, true, " gift , bulk"},
            {5, "ben", 6000, true, ""},
            {6, "dev", 6000, true, "bulk"}};
}
}  // namespace

TEST_CASE("pipelines filter, project and filter again") {
    CHECK(paid_amounts(sample()) == std::vector<long>{7500, 2000, 12000, 6000, 6000});
    CHECK(paid_amounts(sample(), 7000) == std::vector<long>{7500, 12000});
    CHECK(paid_amounts({}).empty());
}

TEST_CASE("projections sort by a member and keep ties stable") {
    auto orders = sample();
    sort_by_total(orders);
    std::vector<int> ids;
    for (const auto& o : orders) ids.push_back(o.id);
    CHECK(ids == std::vector<int>{4, 3, 1, 5, 6, 2});
}

TEST_CASE("top customers are ranked by paid total and truncated") {
    const auto top = top_customers(sample(), 2);
    CHECK_EQ(top.size(), 2u);
    CHECK(top[0] == std::pair<std::string, long>{"cleo", 12000});
    CHECK(top[1] == std::pair<std::string, long>{"ben", 8000});
    CHECK_EQ(top_customers(sample(), 10).size(), 4u);
}

TEST_CASE("views evaluate only what is consumed") {
    CHECK_EQ(count_evaluations(3), 3);
    CHECK_EQ(count_evaluations(1000), 1000);
    CHECK_EQ(count_evaluations(0), 0);
}

TEST_CASE("splitting tags trims spaces and drops empty parts") {
    CHECK(split_tags(" gift , express,,gift ") == std::vector<std::string>{"gift", "express", "gift"});
    CHECK(split_tags("").empty());
    const auto counts = tag_counts(sample());
    CHECK(counts.front() == std::pair<std::string, int>{"gift", 3});
}

TEST_CASE("max_element with a projection finds the largest paid order") {
    const auto orders = sample();
    CHECK_EQ(largest_paid(orders)->id, 4);
    CHECK(largest_paid({{1, "x", 10, false, ""}}) == nullptr);
    CHECK_EQ(euros(12005), "€120.05");
}

TEST_CASE("run reports totals, top customers and tags") {
    std::istringstream in("1 ana 7500 1 gift\n2 ben 9000 1 gift,bulk\n3 ana 100 0 x\nend\n");
    std::ostringstream out;
    CHECK_EQ(run(in, out), 0);
    const auto text = out.str();
    CHECK(text.find("2 paid order(s) of at least €50 worth €165.00") != std::string::npos);
    CHECK(text.find("top: ben €90.00") < text.find("top: ana"));
    CHECK(text.find("tag gift x2") != std::string::npos);
    CHECK(text.find("largest paid order: #2") != std::string::npos);
}
