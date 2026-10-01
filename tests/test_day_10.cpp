// Tests for Day 10 – Randomisation. Every test uses a fixed seed, so results are reproducible.
#include <algorithm>
#include <sstream>
#include <stdexcept>

#include "cppm/testing.hpp"
#include "day_10_randomisation/lesson.hpp"

using namespace cppm::day10;

TEST_CASE("the same seed produces the same sequence") {
    auto a = make_engine(42);
    auto b = make_engine(42);
    for (int i = 0; i < 5; ++i) {
        CHECK_EQ(a(), b());
    }
    auto c = make_engine(43);
    CHECK(make_engine(42)() != c());
}

TEST_CASE("parse_dice accepts valid notation and rejects nonsense") {
    const auto spec = parse_dice("3d6+2");
    REQUIRE(spec.has_value());
    CHECK_EQ(spec->count, 3);
    CHECK_EQ(spec->sides, 6);
    CHECK_EQ(spec->modifier, 2);
    CHECK_EQ(parse_dice("2D20-1").value_or(DiceSpec{}).modifier, -1);
    CHECK(parse_dice("d6") == std::nullopt);
    CHECK(parse_dice("0d6") == std::nullopt);
    CHECK(parse_dice("3d1") == std::nullopt);
    CHECK(parse_dice("3d6+") == std::nullopt);
    CHECK(parse_dice("3d6 extra") == std::nullopt);
}

TEST_CASE("rolls always stay within the possible range") {
    auto engine = make_engine(7);
    const DiceSpec spec{3, 6, 2};
    for (int i = 0; i < 1000; ++i) {
        const int total = roll(spec, engine);
        CHECK(total >= 5 && total <= 20);
    }
}

TEST_CASE("a fair die shows every face roughly equally often") {
    auto engine = make_engine(2026);
    const auto counts = histogram({1, 6, 0}, 6000, engine);
    CHECK_EQ(counts.size(), 6u);
    for (const auto& [face, count] : counts) {
        CHECK(count > 850 && count < 1150);
    }
}

TEST_CASE("loot drops follow the probability and reject invalid chances") {
    auto engine = make_engine(1);
    CHECK_EQ(count_loot_drops(0.0, 100, engine), 0);
    CHECK_EQ(count_loot_drops(1.0, 100, engine), 100);
    const int drops = count_loot_drops(0.25, 4000, engine);
    CHECK(drops > 900 && drops < 1100);
    CHECK_THROWS_AS(count_loot_drops(1.5, 1, engine), std::invalid_argument);
    CHECK_THROWS_AS(count_loot_drops(0.5, -1, engine), std::invalid_argument);
}

TEST_CASE("normal heights centre on the mean and stay clamped") {
    auto engine = make_engine(99);
    const auto heights = npc_heights(2000, engine);
    long long sum = 0;
    for (int h : heights) {
        CHECK(h >= 140 && h <= 210);
        sum += h;
    }
    const double mean = static_cast<double>(sum) / static_cast<double>(heights.size());
    CHECK(mean > 168.0 && mean < 172.0);
}

TEST_CASE("shuffling keeps every player exactly once and is reproducible") {
    const std::vector<std::string> players{"Aria", "Brom", "Cyra", "Dax"};
    auto e1 = make_engine(5);
    auto e2 = make_engine(5);
    auto order = initiative_order(players, e1);
    CHECK(order == initiative_order(players, e2));
    std::sort(order.begin(), order.end());
    CHECK(order == players);
}

TEST_CASE("run replays the same session for the same seed") {
    std::istringstream first("1234\n2d6\nbad\n\n");
    std::istringstream second("1234\n2d6\nbad\n\n");
    std::ostringstream out1;
    std::ostringstream out2;
    CHECK_EQ(run(first, out1), 0);
    run(second, out2);
    CHECK_EQ(out1.str(), out2.str());
    CHECK(out1.str().find("use NdS") != std::string::npos);
}
