// Tests for Day 30 – Getters and Setters.
#include <limits>
#include <sstream>
#include <stdexcept>

#include "cppm/testing.hpp"
#include "day_30_getters_setters/lesson.hpp"

using namespace cppm::day30;

TEST_CASE("a new controller starts with safe defaults") {
    const AquariumController tank;
    CHECK_NEAR(tank.temperature_c(), 25.0, 1e-12);
    CHECK_NEAR(tank.temperature_f(), 77.0, 1e-12);
    CHECK_NEAR(tank.ph(), 7.0, 1e-12);
    CHECK_EQ(tank.light_hours(), 10);
    CHECK(tank.history().empty());
}

TEST_CASE("Celsius and Fahrenheit setters agree") {
    AquariumController tank;
    tank.set_temperature_f(80.6);
    CHECK_NEAR(tank.temperature_c(), 27.0, 1e-12);
    tank.set_temperature_c(26.5);
    CHECK_NEAR(tank.temperature_f(), 79.7, 1e-9);
}

TEST_CASE("the internal unit keeps decimal temperatures exact") {
    AquariumController tank;
    tank.set_temperature_c(25.3);
    CHECK_EQ(tank.temperature_c(), 25.3);
}

TEST_CASE("unsafe values are refused and leave the state unchanged") {
    AquariumController tank;
    CHECK_THROWS_AS(tank.set_temperature_c(35), std::out_of_range);
    CHECK_THROWS_AS(tank.set_temperature_f(32), std::out_of_range);
    CHECK_THROWS_AS(tank.set_temperature_c(std::numeric_limits<double>::quiet_NaN()), std::out_of_range);
    CHECK_THROWS_AS(tank.set_ph(5.5), std::out_of_range);
    CHECK_THROWS_AS(tank.set_light_hours(15), std::out_of_range);
    CHECK_NEAR(tank.temperature_c(), 25.0, 1e-12);
    CHECK(tank.history().empty());
}

TEST_CASE("boundary values are accepted") {
    AquariumController tank;
    CHECK_NOTHROW(tank.set_temperature_c(22.0));
    CHECK_NOTHROW(tank.set_temperature_c(30.0));
    CHECK_NOTHROW(tank.set_ph(6.0));
    CHECK_NOTHROW(tank.set_light_hours(0));
}

TEST_CASE("every accepted change is recorded") {
    AquariumController tank;
    tank.set_temperature_c(26);
    tank.set_ph(7.24);
    tank.set_light_hours(8);
    REQUIRE_EQ(tank.history().size(), 3u);
    CHECK_EQ(tank.history()[0], "temperature 26.0 C");
    CHECK_EQ(tank.history()[1], "pH 7.2");
    CHECK_EQ(tank.history()[2], "light 8 h");
}

TEST_CASE("run applies settings and reports refusals") {
    std::istringstream in("c 26.5\nf 100\nph 7.4\nlight\nshow\nsalt 3\n\n");
    std::ostringstream out;
    CHECK_EQ(run(in, out), 0);
    const auto text = out.str();
    CHECK(text.find("refused: temperature must stay between 22 and 30 C") != std::string::npos);
    CHECK(text.find("a number is required") != std::string::npos);
    CHECK(text.find("26.5 C / 79.7 F, pH 7.4, light 10 h") != std::string::npos);
    CHECK(text.find("unknown setting") != std::string::npos);
    CHECK(text.find("2 change(s) recorded") != std::string::npos);
}
