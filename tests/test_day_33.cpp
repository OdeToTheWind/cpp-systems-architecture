// Tests for Day 33 – Namespaces.
#include <sstream>
#include <stdexcept>
#include <type_traits>

#include "cppm/testing.hpp"
#include "day_33_namespaces/lesson.hpp"

using namespace cppm::day33;

TEST_CASE("the inline namespace makes v2 the default") {
    CHECK_EQ(weather::api_version(), 2);
    CHECK_EQ(weather::v1::api_version(), 1);
    static_assert(std::is_same_v<weather::Reading, weather::v2::Reading>);
    static_assert(!std::is_same_v<weather::Reading, weather::v1::Reading>);
}

TEST_CASE("both parser versions remain usable by their qualified names") {
    const auto current = weather::parse("21.5,4");
    CHECK_NEAR(current.temperature_c, 21.5, 1e-12);
    CHECK_NEAR(current.wind_ms, 4.0, 1e-12);
    CHECK_NEAR(weather::v1::parse("70.7").temperature_f, 70.7, 1e-12);
    CHECK_THROWS_AS(weather::parse("21.5"), std::invalid_argument);
    CHECK_THROWS_AS(weather::v1::parse("warm"), std::invalid_argument);
}

TEST_CASE("nested namespaces group the unit helpers") {
    CHECK_NEAR(weather::units::metric::to_kmh(10), 36.0, 1e-12);
    CHECK_NEAR(weather::units::imperial::to_fahrenheit(100), 212.0, 1e-12);
    CHECK_NEAR(weather::units::imperial::to_celsius(32), 0.0, 1e-12);
}

TEST_CASE("a namespace alias refers to the same functions") {
    namespace wu = weather::units;
    CHECK_NEAR(wu::imperial::to_fahrenheit(0), 32.0, 1e-12);
    CHECK_EQ(describe_reading("07", {20, 5}), "WX-07: 20 C (68 F), wind 18 km/h");
}

TEST_CASE("two functions called mean coexist in different namespaces") {
    const std::vector<double> day{2, 3, 4, 15};
    CHECK_NEAR(stats::mean(day), 6.0, 1e-12);
    CHECK_NEAR(climatology::mean(day), 8.5, 1e-12);
    const auto [arithmetic, midpoint] = daily_summary(day);
    CHECK_NEAR(arithmetic, 6.0, 1e-12);
    CHECK_NEAR(midpoint, 8.5, 1e-12);
    CHECK_THROWS_AS(stats::mean({}), std::invalid_argument);
}

TEST_CASE("a using-declaration imports one function into one scope") {
    CHECK_NEAR(wind_in_kmh({0, 2.5}), 9.0, 1e-12);
}

TEST_CASE("run parses both API versions and summarises the day") {
    std::istringstream in("v2 20,5\nv1 50\nv3 1\nv2 oops\n\n");
    std::ostringstream out;
    CHECK_EQ(run(in, out), 0);
    const auto text = out.str();
    CHECK(text.find("Default API version: 2") != std::string::npos);
    CHECK(text.find("WX-07: 20 C (68 F), wind 18 km/h") != std::string::npos);
    CHECK(text.find("legacy station: 10 C") != std::string::npos);
    CHECK(text.find("start the line with v1 or v2") != std::string::npos);
    CHECK(text.find("v2 expects") != std::string::npos);
    CHECK(text.find("stats::mean 15 C, climatology::mean 15 C") != std::string::npos);
}
