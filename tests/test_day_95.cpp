// Tests for Day 95 – Capstone: Performance Module. Correctness by cross-checking; speed by counting work.
#include <cstdint>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

#include "cppm/testing.hpp"
#include "day_95_spatial_index/lesson.hpp"

using namespace cppm::day95;

TEST_CASE("the naive scan finds the closest driver and breaks ties by id") {
    const std::vector<Driver> drivers{{7, 3, 0}, {2, -3, 0}, {5, 0, 10}};
    std::uint64_t checks = 0;
    CHECK_EQ(nearest_naive(drivers, 0, 0, checks).value_or(-1), 2);  // 7 and 2 tie: smaller id
    CHECK_EQ(nearest_naive(drivers, 0, 9, checks).value_or(-1), 5);
    CHECK(!nearest_naive({}, 0, 0, checks).has_value());
    CHECK_EQ(checks, 6u);
}

TEST_CASE("the grid finds neighbours across cell borders") {
    GridIndex index(1.0);
    index.insert({1, 0.99, 0.5});  // cell (0, 0)
    index.insert({2, 1.01, 0.5});  // cell (1, 0)
    index.insert({3, 0.0, 0.0});
    std::uint64_t checks = 0;
    CHECK_EQ(index.nearest(1.005, 0.5, checks).value_or(-1), 2);
    CHECK_EQ(index.nearest(0.995, 0.5, checks).value_or(-1), 1);
    CHECK_EQ(index.nearest(-50, -50, checks).value_or(-1), 3);  // far outside: rings keep growing
    CHECK_THROWS_AS(index.insert({1, 0, 0}), std::invalid_argument);
    CHECK_THROWS_AS(GridIndex(0), std::invalid_argument);
}

TEST_CASE("the stopping rule never misses a closer driver in a further ring") {
    GridIndex index(1.0);
    index.insert({1, 1.95, 0.5});   // ring 1 from the query's cell, 1.45 km away
    index.insert({2, -0.9, -0.9});  // ring 1 too, but further
    index.insert({3, 0.95, 2.95});  // ring 2, about 2.45 km
    std::uint64_t checks = 0;
    CHECK_EQ(index.nearest(0.5, 0.5, checks).value_or(-1), 1);
    GridIndex empty(1.0);
    CHECK(!empty.nearest(0, 0, checks).has_value());
}

TEST_CASE("moving and removing drivers keeps the index consistent") {
    GridIndex index(2.0);
    index.insert({1, 0, 0});
    index.insert({2, 10, 10});
    std::uint64_t checks = 0;
    index.move(2, 0.5, 0.5);  // across many cells
    CHECK_EQ(index.nearest(0.6, 0.6, checks).value_or(-1), 2);
    index.move(2, 0.7, 0.7);  // same cell: no bucket change
    index.remove(2);
    CHECK_EQ(index.nearest(0.6, 0.6, checks).value_or(-1), 1);
    CHECK_EQ(index.size(), 1u);
    CHECK_THROWS_AS(index.move(9, 0, 0), std::invalid_argument);
    index.remove(9);  // removing an unknown driver is harmless
}

TEST_CASE("randomised cross-checks find no disagreement for any cell size") {
    for (const double cell : {0.25, 1.0, 5.0, 50.0}) {
        const auto result = cross_check(2000, 300, cell, 2026);
        CHECK_EQ(result.mismatches, 0);
    }
}

TEST_CASE("the grid does a small fraction of the naive work") {
    const auto result = cross_check(20000, 200, 1.0, 7);
    CHECK_EQ(result.mismatches, 0);
    CHECK_EQ(result.naive_checks, 20000u * 200u);
    CHECK(result.grid_checks * 20 < result.naive_checks);  // over 20x fewer distance computations
}

TEST_CASE("run reports agreement and timings") {
    std::istringstream in("3000 100 1\nstop\n");
    std::ostringstream out;
    CHECK_EQ(run(in, out), 0);
    CHECK(out.str().find("0 mismatch(es) in 100 queries; distance checks naive 300000 vs grid ") != std::string::npos);
    CHECK(out.str().find("one query: naive ") != std::string::npos);
}
