// Tests for Day 91 – Capstone: Type-safe Configuration. The environment is a plain map.
#include <chrono>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

#include "cppm/testing.hpp"
#include "day_91_typed_config/lesson.hpp"

using namespace cppm::day91;
using namespace std::chrono_literals;

namespace {
Env good() {
    return {{"DISPATCH_DATABASE_URL", "postgres://db.internal/dispatch"},
            {"DISPATCH_REGIONS", "LIS, OPO,,FAO"},
            {"DISPATCH_MAPS_API_KEY", "mk_live_12345678"},
            {"PATH", "/usr/bin"}};
}
}  // namespace

TEST_CASE("parsers accept good text and explain bad text") {
    CHECK_EQ(parse_int("12", 1, 64), 12);
    CHECK_THROWS_AS(parse_int("12x", 1, 64), std::invalid_argument);
    CHECK_THROWS_AS(parse_int("0", 1, 64), std::invalid_argument);
    CHECK(parse_bool("Yes"));
    CHECK(!parse_bool("off"));
    CHECK(parse_duration("500ms") == 500ms);
    CHECK(parse_duration("5m") == 300000ms);
    CHECK_THROWS_AS(parse_duration("5 m"), std::invalid_argument);
    CHECK_THROWS_AS(parse_duration("m"), std::invalid_argument);
    CHECK(parse_list(" a, b ,") == std::vector<std::string>{"a", "b"});
    CHECK_THROWS_AS(parse_url("db.internal"), std::invalid_argument);
}

TEST_CASE("a valid environment fills every typed member, with defaults") {
    const auto config = dispatch_schema().load(good());
    CHECK_EQ(config.database_url, "postgres://db.internal/dispatch");
    CHECK_EQ(config.workers, 4);
    CHECK(config.courier_timeout == 45s);
    CHECK(!config.dry_run);
    CHECK(config.mode == Mode::live);
    CHECK(config.regions == std::vector<std::string>{"LIS", "OPO", "FAO"});
    CHECK_EQ(config.maps_api_key.reveal(), "mk_live_12345678");
}

TEST_CASE("overrides replace defaults") {
    Env env = good();
    env["DISPATCH_WORKERS"] = "16";
    env["DISPATCH_MODE"] = "shadow";
    env["DISPATCH_COURIER_TIMEOUT"] = "2m";
    const auto config = dispatch_schema().load(env);
    CHECK_EQ(config.workers, 16);
    CHECK(config.mode == Mode::shadow);
    CHECK(config.courier_timeout == 120s);
}

TEST_CASE("every problem is reported in one go") {
    Env env{{"DISPATCH_WORKERS", "100"}, {"DISPATCH_DRY_RUN", "maybe"}, {"DISPATCH_MAPS_API_KEY", "short"}, {"DISPATCH_REGION", "LIS"}};
    try {
        dispatch_schema().load(env);
        CHECK(false);
    } catch (const ConfigError& e) {
        const auto& p = e.problems();
        CHECK_EQ(p.size(), 6u);
        CHECK_EQ(p[0], "DISPATCH_DATABASE_URL: required (PostgreSQL connection URL)");
        CHECK_EQ(p[1], "DISPATCH_WORKERS: must be between 1 and 64");
        CHECK_EQ(p[2], "DISPATCH_DRY_RUN: expected true/false, got 'maybe'");
        CHECK_EQ(p[4], "DISPATCH_MAPS_API_KEY: must be at least 8 characters");
        CHECK_EQ(p[5], "DISPATCH_REGION: unknown setting");  // probably meant REGIONS
        CHECK(std::string(e.what()).find("short") == std::string::npos);  // the bad secret is not echoed
    }
}

TEST_CASE("descriptions redact secrets and mark defaults") {
    const auto text = dispatch_schema().describe(good());
    CHECK(text.find("DISPATCH_MAPS_API_KEY=***\n") != std::string::npos);
    CHECK(text.find("mk_live") == std::string::npos);
    CHECK(text.find("DISPATCH_WORKERS=4 (default)\n") != std::string::npos);
}

TEST_CASE("the schema documents itself") {
    const auto example = dispatch_schema().env_example();
    CHECK(example.find("# worker threads, 1-64\nDISPATCH_WORKERS=4\n") != std::string::npos);
    CHECK(example.find("# routing API key (required)\nDISPATCH_MAPS_API_KEY=\n") != std::string::npos);
    CHECK(example.find("# comma-separated city codes (required)\nDISPATCH_REGIONS=\n") != std::string::npos);
}

TEST_CASE("run loads or explains") {
    std::istringstream ok("DISPATCH_DATABASE_URL=postgres://x/y\nDISPATCH_REGIONS=LIS\nDISPATCH_MAPS_API_KEY=abcdefghij\n\n");
    std::ostringstream out;
    CHECK_EQ(run(ok, out), 0);
    CHECK(out.str().find("loaded: 4 worker(s), timeout 45000 ms, 1 region(s)") != std::string::npos);
    std::istringstream bad("DISPATCH_WORKERS=zero\n\n");
    std::ostringstream out2;
    run(bad, out2);
    CHECK(out2.str().find("4 configuration problem(s):") != std::string::npos);
    CHECK(out2.str().find("example:\n# PostgreSQL connection URL (required)") != std::string::npos);
}
