/**
 * @file
 * Day 10 – Randomisation.
 *
 * Scenario: a *tabletop role-playing game master's toolkit*: dice notation such as `3d6+2`,
 * loot that drops with a given chance, randomly generated non-player characters and a
 * shuffled initiative order – all reproducible from a seed so a session can be replayed.
 *
 * Deliverables (syllabus):
 * - std::mt19937 engines and seeding
 * - Uniform, normal and bernoulli distributions
 * - Shuffling
 * - Reproducible randomness
 */
#pragma once

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <istream>
#include <map>
#include <optional>
#include <ostream>
#include <random>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

#include "cppm/lesson.hpp"

namespace cppm::day10 {

inline constexpr Deliverable DELIVERABLES[] = {
    {"seeding a std::mt19937 engine (fixed or random_device)", "make_engine"},
    {"parsing and validating dice notation", "parse_dice"},
    {"uniform_int_distribution for fair dice", "roll"},
    {"bernoulli_distribution for drop chances", "count_loot_drops"},
    {"normal_distribution for natural variation", "npc_heights"},
    {"std::shuffle for a fair turn order", "initiative_order"},
};

/// A seed makes every sequence reproducible; without one, std::random_device supplies entropy.
inline std::mt19937 make_engine(std::optional<std::uint32_t> seed = std::nullopt) {
    if (seed) {
        return std::mt19937(*seed);
    }
    std::random_device device;
    return std::mt19937(device());
}

/// `count`d`sides`+`modifier`, e.g. 3d6+2.
struct DiceSpec {
    int count{1};
    int sides{6};
    int modifier{0};
};

/// Parse "NdS", "NdS+M" or "NdS-M" (N 1–100, S 2–1000); std::nullopt for anything else.
inline std::optional<DiceSpec> parse_dice(std::string_view text) {
    std::istringstream in{std::string(text)};
    DiceSpec spec;
    char d{};
    if (!(in >> spec.count >> d >> spec.sides) || (d != 'd' && d != 'D')) {
        return std::nullopt;
    }
    char sign{};
    if (in >> sign) {
        if ((sign != '+' && sign != '-') || !(in >> spec.modifier) || spec.modifier < 0) {
            return std::nullopt;
        }
        if (sign == '-') {
            spec.modifier = -spec.modifier;
        }
    }
    in >> std::ws;
    if (!in.eof() || spec.count < 1 || spec.count > 100 || spec.sides < 2 || spec.sides > 1000) {
        return std::nullopt;
    }
    return spec;
}

/// Roll every die with a uniform distribution over [1, sides] and add the modifier.
inline int roll(const DiceSpec& spec, std::mt19937& engine) {
    std::uniform_int_distribution<int> die(1, spec.sides);  // both bounds are inclusive
    int total = spec.modifier;
    for (int i = 0; i < spec.count; ++i) {
        total += die(engine);
    }
    return total;
}

/// How many of @p attempts drop loot when each has probability @p chance in [0, 1].
inline int count_loot_drops(double chance, int attempts, std::mt19937& engine) {
    if (!(chance >= 0.0 && chance <= 1.0)) {  // also rejects NaN
        throw std::invalid_argument("chance must be between 0 and 1");
    }
    if (attempts < 0) {
        throw std::invalid_argument("attempts must not be negative");
    }
    std::bernoulli_distribution drop(chance);
    int drops = 0;
    for (int i = 0; i < attempts; ++i) {
        drops += drop(engine) ? 1 : 0;
    }
    return drops;
}

/// Heights in cm from a normal distribution (mean 170, sd 10), clamped to a plausible 140–210.
inline std::vector<int> npc_heights(int count, std::mt19937& engine) {
    std::normal_distribution<double> height(170.0, 10.0);
    std::vector<int> heights;
    heights.reserve(static_cast<std::size_t>(std::max(count, 0)));
    for (int i = 0; i < count; ++i) {
        heights.push_back(static_cast<int>(std::lround(std::clamp(height(engine), 140.0, 210.0))));
    }
    return heights;
}

/// A fair turn order: every permutation is equally likely (Fisher–Yates inside std::shuffle).
inline std::vector<std::string> initiative_order(std::vector<std::string> players, std::mt19937& engine) {
    std::shuffle(players.begin(), players.end(), engine);
    return players;
}

/// Roll @p spec @p times and count how often each total appears.
inline std::map<int, int> histogram(const DiceSpec& spec, int times, std::mt19937& engine) {
    std::map<int, int> counts;
    for (int i = 0; i < times; ++i) {
        ++counts[roll(spec, engine)];
    }
    return counts;
}

/// The interactive demo: an optional seed, then dice expressions until a blank line.
inline int run(std::istream& in, std::ostream& out) {
    out << "Day 10 – Randomisation\n";
    auto seed_line = prompt_line(in, out, "Seed (blank for a random session): ");
    std::optional<std::uint32_t> seed;
    if (seed_line && !seed_line->empty()) {
        try {
            seed = static_cast<std::uint32_t>(std::stoul(*seed_line));
        } catch (const std::exception&) {
            out << "  not a number – using a random seed\n";
        }
    }
    auto engine = make_engine(seed);
    out << "Initiative:";
    for (const auto& name : initiative_order({"Aria", "Brom", "Cyra", "Dax"}, engine)) {
        out << ' ' << name;
    }
    out << "\nNPC heights:";
    for (int h : npc_heights(3, engine)) {
        out << ' ' << h << "cm";
    }
    out << "\nLoot drops from 10 chests at 25%: " << count_loot_drops(0.25, 10, engine) << '\n';
    while (seed_line) {
        auto line = prompt_line(in, out, "dice (e.g. 3d6+2)> ");
        if (!line || line->empty()) {
            break;
        }
        if (auto spec = parse_dice(*line)) {
            out << "  " << *line << " -> " << roll(*spec, engine) << '\n';
        } else {
            out << "  use NdS, NdS+M or NdS-M (1-100 dice, 2-1000 sides)\n";
        }
    }
    return 0;
}

}  // namespace cppm::day10
