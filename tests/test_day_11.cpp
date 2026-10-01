// Tests for Day 11 – Error Handling.
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

#include "cppm/testing.hpp"
#include "day_11_error_handling/lesson.hpp"

using namespace cppm::day11;

TEST_CASE("parse_reading accepts a well-formed line") {
    const auto r = parse_reading("gh-1,21.5,64");
    CHECK_EQ(r.sensor, "gh-1");
    CHECK_NEAR(r.temperature_c, 21.5, 1e-12);
    CHECK_NEAR(r.humidity_pct, 64.0, 1e-12);
}

TEST_CASE("malformed lines throw std::invalid_argument") {
    CHECK_THROWS_AS(parse_reading("gh-1,21.5"), std::invalid_argument);
    CHECK_THROWS_AS(parse_reading("gh-1,warm,50"), std::invalid_argument);
    CHECK_THROWS_AS(parse_reading("gh-1,21.5x,50"), std::invalid_argument);
    CHECK_THROWS_AS(parse_reading(",21,50"), std::invalid_argument);
}

TEST_CASE("impossible values throw the custom SensorError with the sensor id") {
    CHECK_THROWS_AS(parse_reading("gh-2,95,50"), SensorError);
    try {
        parse_reading("gh-3,20,140");
        CHECK(false);
    } catch (const SensorError& error) {
        CHECK_EQ(error.sensor(), "gh-3");
        CHECK_EQ(std::string(error.what()), "gh-3: humidity must be 0-100 %");
    }
}

TEST_CASE("SensorError can be caught as any of its base classes") {
    CHECK_THROWS_AS(parse_reading("gh-2,95,50"), std::runtime_error);
    CHECK_THROWS_AS(parse_reading("gh-2,95,50"), std::exception);
    CHECK_THROWS_AS(parse_reading("gh-1,1e999,50"), std::out_of_range);
}

TEST_CASE("summarise_log keeps going after bad lines and counts them") {
    std::istringstream log("# header\ngh-1,20,50\ngh-2,99,50\nbroken\ngh-2,80,10\ngh-1,1e999,4\ngh-3,22,60\n");
    const auto summary = summarise_log(log);
    CHECK_EQ(summary.readings.size(), 2u);
    CHECK_EQ(summary.malformed, 2);
    CHECK_EQ(summary.faulty_sensors.at("gh-2"), 2);
    CHECK_NEAR(summary.average_temperature(), 21.0, 1e-12);
}

TEST_CASE("averaging an empty log is a logic error") {
    const LogSummary empty;
    CHECK_THROWS_AS(empty.average_temperature(), std::logic_error);
}

TEST_CASE("load_with_audit logs the failure and rethrows the original exception") {
    std::istringstream log("only,garbage\n");
    std::vector<std::string> audit;
    CHECK_THROWS_AS(load_with_audit(log, audit), std::runtime_error);
    REQUIRE_EQ(audit.size(), 1u);
    CHECK_EQ(audit[0], "load failed: log contained no valid readings");
    std::istringstream good("gh-1,20,50\n");
    CHECK_EQ(load_with_audit(good, audit).readings.size(), 1u);
}

TEST_CASE("reading_at is bounds-checked") {
    std::istringstream log("gh-1,20,50\n");
    const auto summary = summarise_log(log);
    CHECK_EQ(reading_at(summary, 0).sensor, "gh-1");
    CHECK_THROWS_AS(reading_at(summary, 1), std::out_of_range);
}

TEST_CASE("run prints a summary, or the error and the audit trail") {
    std::istringstream good("gh-1,20,50\ngh-9,-50,50\n\n");
    std::ostringstream out;
    CHECK_EQ(run(good, out), 0);
    CHECK(out.str().find("1 valid, 0 malformed") != std::string::npos);
    CHECK(out.str().find("faulty sensor gh-9") != std::string::npos);
    std::istringstream bad("nonsense\n\n");
    std::ostringstream out2;
    run(bad, out2);
    CHECK(out2.str().find("[audit] load failed") != std::string::npos);
}
