// Tests for Day 45 – STL Algorithms.
#include <sstream>
#include <string>
#include <vector>

#include "cppm/testing.hpp"
#include "day_45_stl_algorithms/lesson.hpp"

using namespace cppm::day45;

namespace {
std::vector<Record> season() {
    return {
        {"ash", 3, 1, 0, 9, 3},     // 10 points, +6
        {"bree", 3, 1, 0, 8, 2},    // 10 points, +6, fewer scored
        {"cole", 1, 0, 3, 4, 9},    // 3 points
        {"dax", 3, 1, 0, 10, 4},    // 10 points, +6, most scored
        {"eve", 1, 0, 0, 2, 0},     // only one match
        {"cheater", 4, 0, 0, 20, 0},
    };
}
std::vector<std::string> names(const std::vector<Record>& table) {
    std::vector<std::string> result;
    for (const auto& r : table) result.push_back(r.player);
    return result;
}
}  // namespace

TEST_CASE("transform computes points in order") {
    CHECK(points_of(season()) == std::vector<int>{10, 10, 3, 10, 3, 12});
    CHECK(points_of({}).empty());
}

TEST_CASE("copy_if keeps players with enough matches") {
    CHECK(names(qualified(season(), 3)) == std::vector<std::string>{"ash", "bree", "cole", "dax", "cheater"});
    CHECK(qualified(season(), 100).empty());
}

TEST_CASE("accumulate totals the points") {
    CHECK_EQ(total_points(season()), 48);
    CHECK_EQ(total_points({}), 0);
}

TEST_CASE("erase-remove deletes every banned player in one pass") {
    auto table = season();
    CHECK_EQ(remove_disqualified(table, {"cheater", "nobody"}), 1u);
    CHECK_EQ(table.size(), 5u);
    CHECK_EQ(remove_disqualified(table, {}), 0u);
}

TEST_CASE("rank applies every tie-breaker in order") {
    auto table = qualified(season(), 3);
    remove_disqualified(table, {"cheater"});
    rank(table);
    CHECK(names(table) == std::vector<std::string>{"dax", "ash", "bree", "cole"});
}

TEST_CASE("names break complete ties alphabetically") {
    std::vector<Record> tied{{"zed", 1, 0, 0, 1, 0}, {"amy", 1, 0, 0, 1, 0}};
    rank(tied);
    CHECK(names(tied) == std::vector<std::string>{"amy", "zed"});
}

TEST_CASE("stable_partition separates zones and keeps the ranking inside them") {
    std::vector<Record> table{{"a", 1, 0, 0, 0, 0}, {"b", 4, 0, 0, 0, 0}, {"c", 0, 0, 1, 0, 0}, {"d", 5, 0, 0, 0, 0}};
    const auto promoted = promotion_zone(table, 10);
    CHECK_EQ(promoted, 2u);
    CHECK(names(table) == std::vector<std::string>{"b", "d", "a", "c"});
}

TEST_CASE("run prints the ranked, filtered league") {
    std::istringstream in("ash 3 1 0 9 3\nbree 3 1 0 8 2\ncole 1 0 3 4 9\ndax 3 1 0 10 4\neve 1 0 0 2 0\n"
                          "cheater 4 0 0 20 0\nEND\n");
    std::ostringstream out;
    CHECK_EQ(run(in, out), 0);
    const auto text = out.str();
    CHECK(text.find("removed 1, 4 qualified, 33 points in total") != std::string::npos);
    CHECK(text.find(" 1. dax        10  ^ promoted") != std::string::npos);
    CHECK(text.find(" 4. cole        3\n") != std::string::npos);
}
