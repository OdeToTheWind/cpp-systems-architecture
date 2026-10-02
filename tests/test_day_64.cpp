// Tests for Day 64 – Templates & Generic Programming.
#include <sstream>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <vector>

#include "cppm/testing.hpp"
#include "day_64_templates/lesson.hpp"

using namespace cppm::day64;

TEST_CASE("function templates deduce their parameters") {
    CHECK_EQ(largest(std::vector<int>{3, 9, 2}), 9);
    CHECK_EQ(largest(std::vector<std::string>{"pear", "apple", "zucchini"}), "zucchini");
    CHECK_THROWS_AS(largest(std::vector<int>{}), std::invalid_argument);
    static_assert(std::is_same_v<decltype(add(1, 2.5)), double>);
    CHECK_NEAR(add(1, 2.5), 3.5, 1e-12);
    CHECK_EQ(add(std::string("wind "), "NE"), "wind NE");
}

TEST_CASE("the ring buffer keeps the newest values in order") {
    RingBuffer<int, 3> buffer;
    for (int v : {1, 2, 3, 4, 5}) buffer.push(v);
    CHECK_EQ(buffer.size(), 3u);
    CHECK(buffer.to_vector() == std::vector<int>{3, 4, 5});
    CHECK_EQ(buffer.latest(), 5);
    CHECK_THROWS_AS(buffer[3], std::out_of_range);
    static_assert(RingBuffer<int, 3>::capacity() == 3);
}

TEST_CASE("the same template works for strings") {
    RingBuffer<std::string, 2> status;
    status.push("ok");
    status.push("low battery");
    status.push("ok");
    CHECK(status.to_vector() == std::vector<std::string>{"low battery", "ok"});
}

TEST_CASE("class template argument deduction uses the guide") {
    Reading wind{"wind", 4.5};
    Reading count{"gusts", 3};
    static_assert(std::is_same_v<decltype(wind), Reading<double>>);
    static_assert(std::is_same_v<decltype(count), Reading<int>>);
    CHECK_EQ(wind.sensor, "wind");
}

TEST_CASE("specialisations name types, including nested ones") {
    CHECK_EQ(type_name<int>(), "int");
    CHECK_EQ(type_name<std::vector<std::vector<double>>>(), "vector<vector<double>>");
    CHECK_EQ((type_name<RingBuffer<int, 4>>()), "RingBuffer<int, 4>");
}

TEST_CASE("summarise works with any container through dependent names") {
    const auto v = summarise(std::vector<int>{4, -2, 7});
    CHECK(v.has_value());
    CHECK_EQ(v->min, -2);
    CHECK_EQ(v->max, 7);
    CHECK_NEAR(v->mean, 3.0, 1e-12);
    static_assert(std::is_same_v<decltype(v->min), int>);
    RingBuffer<double, 4> temps;
    temps.push(20.5);
    temps.push(22.5);
    CHECK_EQ(describe(*summarise(temps)), "2 x double: min 20.5, max 22.5, mean 21.5");
    CHECK(!summarise(std::vector<double>{}).has_value());
}

TEST_CASE("run summarises the last five readings of each sensor") {
    std::istringstream in("temp 18\ntemp 19\ntemp 20\ntemp 21\ntemp 22\ntemp 23\nwind 10\nrain 3\nstop\n");
    std::ostringstream out;
    CHECK_EQ(run(in, out), 0);
    CHECK(out.str().find("temperature: 5 x double: min 19, max 23, mean 21") != std::string::npos);
    CHECK(out.str().find("wind: 1 x int: min 10, max 10, mean 10") != std::string::npos);
    CHECK(out.str().find("unknown sensor rain") != std::string::npos);
    CHECK(out.str().find("RingBuffer<double, 5> and RingBuffer<int, 5>") != std::string::npos);
}
