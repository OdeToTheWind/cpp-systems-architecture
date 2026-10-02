// Tests for Day 77 – Logging & Configuration. A fake clock gives stable timestamps.
#include <memory>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

#include "cppm/testing.hpp"
#include "day_77_logging_config/lesson.hpp"

using namespace cppm::day77;

namespace {
Logger fixed_logger() {
    return Logger("pay", [] { return "12:00:00"; });
}
}  // namespace

TEST_CASE("levels are ordered and parsed case-insensitively") {
    CHECK(Level::debug < Level::info && Level::warning < Level::error);
    CHECK(parse_level("Warn") == Level::warning);
    CHECK(parse_level("error") == Level::error);
    CHECK_THROWS_AS(parse_level("loud"), std::invalid_argument);
}

TEST_CASE("formatters produce text and key=value lines") {
    const Record r{Level::warning, "12:00:00", "pay", "card declined", {{"code", "51"}, {"merchant", "Cafe Lua"}}};
    CHECK_EQ(TextFormatter{}.format(r), "12:00:00 WARNING [pay] card declined code=51 merchant=Cafe Lua");
    CHECK_EQ(KeyValueFormatter{}.format(r),
             "time=12:00:00 level=WARNING logger=pay msg=\"card declined\" code=51 merchant=\"Cafe Lua\"");
}

TEST_CASE("each sink filters by its own minimum level") {
    auto all = std::make_shared<MemorySink>(Level::debug, std::make_shared<TextFormatter>());
    auto errors = std::make_shared<MemorySink>(Level::error, std::make_shared<KeyValueFormatter>());
    Logger log = fixed_logger();
    log.add_sink(all);
    log.add_sink(errors);
    log.log(Level::debug, "connecting");
    log.info("connected");
    log.error("timeout");
    CHECK_EQ(all->lines().size(), 3u);
    CHECK_EQ(errors->lines().size(), 1u);
    CHECK_EQ(errors->lines()[0], "time=12:00:00 level=ERROR logger=pay msg=timeout");
}

TEST_CASE("INI files are parsed into section.key settings") {
    std::istringstream file("; payment gateway\n[database]\nhost = db.internal\n pool_size=8 \n\n[log]\n# comment\nlevel=debug\n"
                            "verbose = yes\nempty =\n");
    const Config config = parse_ini(file);
    CHECK_EQ(config.get_or("database.host", ""), "db.internal");
    CHECK_EQ(config.get_int("database.pool_size", 1), 8);
    CHECK(config.get_bool("log.verbose", false));
    CHECK_EQ(config.get_or("log.empty", "x"), "");
    CHECK_EQ(config.get_int("missing.key", 42), 42);
    std::istringstream inline_comments("limit = 5000 ; per payment\nurl = http://x/#top\n");
    const Config c2 = parse_ini(inline_comments);
    CHECK_EQ(c2.get_int("limit", 0), 5000);
    CHECK_EQ(c2.get_or("url", ""), "http://x/#top");  // no whitespace before '#': part of the value
}

TEST_CASE("bad INI lines and bad values are reported precisely") {
    std::istringstream bad("[ok]\nkey value\n");
    try {
        parse_ini(bad);
        CHECK(false);
    } catch (const std::runtime_error& e) {
        CHECK_EQ(std::string(e.what()), "line 2: expected key = value");
    }
    std::istringstream header("[broken\n");
    CHECK_THROWS_AS(parse_ini(header), std::runtime_error);
    Config config;
    config.set("db.pool", "8x");
    config.set("db.flag", "maybe");
    CHECK_THROWS_AS(config.get_int("db.pool", 0), std::invalid_argument);
    CHECK_THROWS_AS(config.get_bool("db.flag", false), std::invalid_argument);
}

TEST_CASE("environment variables override file values") {
    std::istringstream file("[database]\npool_size = 8\nhost = localhost\n");
    Config config = parse_ini(file);
    const auto overridden = config.apply_env({{"PAYGATE_DATABASE_POOL_SIZE", "32"}, {"OTHER_DATABASE_HOST", "x"}}, "PAYGATE");
    CHECK(overridden == std::vector<std::string>{"database.pool_size"});
    CHECK_EQ(config.get_int("database.pool_size", 0), 32);
    CHECK_EQ(config.get_or("database.host", ""), "localhost");
}

TEST_CASE("run applies the override and logs by level") {
    std::istringstream in("200 EUR\n1500 EUR\n10 USD\nstop\n");
    std::ostringstream out;
    CHECK_EQ(run(in, out), 0);
    const auto text = out.str();
    CHECK(text.find("override: gateway.max_amount") != std::string::npos);
    CHECK(text.find("t+1s INFO [gateway] payment accepted amount=200") != std::string::npos);
    CHECK(text.find("ERROR [gateway] amount over limit amount=1500 limit=1000") != std::string::npos);
    CHECK(text.find("WARNING [gateway] currency not supported currency=USD") != std::string::npos);
    CHECK(text.find("DEBUG") == std::string::npos);
}
