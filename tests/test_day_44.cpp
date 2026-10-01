// Tests for Day 44 – Tabular Data Analysis.
#include <sstream>
#include <stdexcept>

#include "cppm/testing.hpp"
#include "day_44_tabular_data/lesson.hpp"

using namespace cppm::day44;

namespace {
Table sample() {
    std::istringstream data("north 10 3\nsouth 30 6\nnorth 20 5\nsouth 1 0.1\nbad line\neast 15 4\n");
    return load_trips(data);
}
}  // namespace

TEST_CASE("loading builds equal-length typed columns and skips bad lines") {
    const Table trips = sample();
    CHECK_EQ(trips.rows(), 5u);
    CHECK_EQ(trips.texts("station")[4], "east");
    CHECK_NEAR(trips.numbers("km")[1], 6.0, 1e-12);
    CHECK_THROWS_AS(trips.numbers("speed"), std::out_of_range);
}

TEST_CASE("columns must have the same length and unique names") {
    Table table;
    table.add_numeric("a", {1, 2, 3});
    CHECK_THROWS_AS(table.add_numeric("b", {1, 2}), std::invalid_argument);
    CHECK_THROWS_AS(table.add_text("a", {"x", "y", "z"}), std::invalid_argument);
}

TEST_CASE("filter keeps matching rows in every column") {
    const Table long_trips = sample().filter([](const Table::Row& r) { return r.number("minutes") >= 15; });
    CHECK_EQ(long_trips.rows(), 3u);
    CHECK(long_trips.texts("station") == std::vector<std::string>{"south", "north", "east"});
    CHECK(long_trips.numbers("km") == std::vector<double>{6, 5, 4});
}

TEST_CASE("derive adds a computed column") {
    Table trips = sample();
    trips.derive("kmh", [](const Table::Row& r) { return r.number("km") / (r.number("minutes") / 60.0); });
    CHECK_NEAR(trips.numbers("kmh")[0], 18.0, 1e-9);
    CHECK_NEAR(trips.numbers("kmh")[1], 12.0, 1e-9);
    CHECK_THROWS_AS(trips.derive("kmh", [](const Table::Row&) { return 0.0; }), std::invalid_argument);
}

TEST_CASE("group_mean aggregates per key") {
    const auto means = group_mean(sample(), "station", "minutes");
    CHECK_EQ(means.size(), 3u);
    CHECK_NEAR(means.at("north"), 15.0, 1e-12);
    CHECK_NEAR(means.at("south"), 15.5, 1e-12);
    CHECK_NEAR(means.at("east"), 15.0, 1e-12);
}

TEST_CASE("describe computes the usual summary statistics") {
    const Summary odd = describe({2, 4, 4, 4, 5, 5, 7, 9});
    CHECK_EQ(odd.count, 8u);
    CHECK_NEAR(odd.mean, 5.0, 1e-12);
    CHECK_NEAR(odd.median, 4.5, 1e-12);
    CHECK_NEAR(odd.stddev, 2.138089935299395, 1e-12);
    CHECK_NEAR(describe({3, 1, 2}).median, 2.0, 1e-12);
    CHECK_NEAR(describe({7}).stddev, 0.0, 1e-12);
    CHECK_THROWS_AS(describe({}), std::invalid_argument);
}

TEST_CASE("run filters, groups and summarises") {
    std::istringstream in("north 10 3\nsouth 30 6\nnorth 20 5\nsouth 1 0.1\nEND\n");
    std::ostringstream out;
    CHECK_EQ(run(in, out), 0);
    const auto text = out.str();
    CHECK(text.find("3 of 4 trips kept") != std::string::npos);
    CHECK(text.find("north: 16.5 km/h") != std::string::npos);
    CHECK(text.find("south: 12.0 km/h") != std::string::npos);
    CHECK(text.find("median 20.0") != std::string::npos);
    std::istringstream empty("END\n");
    std::ostringstream out2;
    run(empty, out2);
    CHECK(out2.str().find("no valid trips") != std::string::npos);
}
