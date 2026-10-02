// Tests for Day 65 – Concepts & Constraints. Many checks are static_asserts: concepts are compile-time facts.
#include <array>
#include <list>
#include <map>
#include <ranges>
#include <sstream>
#include <string>
#include <vector>

#include "cppm/testing.hpp"
#include "day_65_concepts/lesson.hpp"

using namespace cppm::day65;

namespace {
struct Unordered {
    int x;
};
struct NamedThing {
    const char* label() const { return "thing"; }
};
struct WrongLabel {
    int label() const { return 1; }
};
template <typename T>
concept CanClamp = requires(T t) { clamp_to(t, t, t); };
template <typename T>
concept CanFormat = requires(T t) { format_value(t); };
}  // namespace

TEST_CASE("standard concepts constrain clamp_to") {
    CHECK_EQ(clamp_to(15, 0, 10), 10);
    CHECK_EQ(clamp_to(-1.5, 0.0, 1.0), 0.0);
    CHECK_EQ(clamp_to(std::string("m"), std::string("a"), std::string("k")), "k");
    static_assert(CanClamp<int>);
    static_assert(!CanClamp<Unordered>);
}

TEST_CASE("the Numeric concept excludes bool and char") {
    static_assert(Numeric<int> && Numeric<unsigned long> && Numeric<float>);
    static_assert(!Numeric<bool> && !Numeric<char> && !Numeric<std::string>);
    CHECK(Numeric<double>);
}

TEST_CASE("Labelled checks the return type of label()") {
    static_assert(Labelled<Server> && Labelled<NamedThing>);
    static_assert(!Labelled<WrongLabel> && !Labelled<int>);
    CHECK_EQ(format_value(NamedThing{}), "[thing]");
}

TEST_CASE("Series accepts sized ranges of numbers only") {
    static_assert(Series<std::vector<int>> && Series<std::array<double, 3>> && Series<std::list<long>>);
    static_assert(!Series<std::vector<std::string>> && !Series<std::vector<bool>> && !Series<int>);
    static_assert(!Series<std::map<int, int>>);
    CHECK_NEAR(average(std::array<float, 2>{1.0f, 2.0f}), 1.5, 1e-9);
}

TEST_CASE("the most constrained overload wins") {
    CHECK_EQ(format_value(1234567), "1,234,567");
    CHECK_EQ(format_value(-1000), "-1,000");
    CHECK_EQ(format_value(999), "999");
    CHECK_EQ(format_value(12000u), "12,000 (count)");
    CHECK_EQ(format_value(2.26), "2.3");
    CHECK_EQ(format_value(Server{"db", 2}), "[db@2]");
    static_assert(!CanFormat<bool> && !CanFormat<Unordered>);
}

TEST_CASE("sparklines scale between the minimum and maximum") {
    CHECK_EQ(sparkline(std::vector<int>{0, 7}), "▁█");
    CHECK_EQ(sparkline(std::vector<int>{5, 5}), "▄▄");
    CHECK_EQ(sparkline(std::vector<double>{}), "");
    CHECK_EQ(sparkline(std::vector<int>{1, 2, 3, 4, 5, 6, 7, 8}), "▁▂▃▄▅▆▇█");
}

TEST_CASE("run draws a line per series") {
    std::istringstream in("100 200 300\n\n");
    std::ostringstream out;
    CHECK_EQ(run(in, out), 0);
    CHECK(out.str().find("orders today: 1,234,567, queue: 42 (count), cpu: 73.4%, host: [web-1@3]") !=
          std::string::npos);
    CHECK(out.str().find("▁▅█  avg 200.0 ms, max 300 ms") != std::string::npos);
}
