/**
 * @file
 * Day 13 – For Loops.
 *
 * Scenario: a *marathon timing station*. Chip mats record each runner's split at every
 * checkpoint; loops total the times, rank the finishers while skipping runners who did not
 * finish, find the first runner under a target time, and print a pace chart.
 *
 * Deliverables (syllabus):
 * - Counted and range-based for loops
 * - Nested loops
 * - break and continue
 * - Index-safe iteration
 */
#pragma once

#include <algorithm>
#include <cstddef>
#include <iomanip>
#include <istream>
#include <optional>
#include <ostream>
#include <sstream>
#include <string>
#include <vector>

#include "cppm/lesson.hpp"

namespace cppm::day13 {

inline constexpr Deliverable DELIVERABLES[] = {
    {"range-based for over a runner's splits", "total_seconds"},
    {"continue to skip runners who did not finish", "rank_finishers"},
    {"break as soon as the answer is found", "first_under"},
    {"nested counted loops build a table", "pace_chart"},
    {"counting down with an unsigned index without underflow", "splits_in_reverse"},
};

/// One runner; a split of 0 or less means the chip did not register (did not finish).
struct Runner {
    std::string name;
    std::vector<int> split_seconds;
};

/// Sum of all splits, read with a range-based for loop (no index, no off-by-one).
inline int total_seconds(const Runner& runner) {
    int total = 0;
    for (const int split : runner.split_seconds) {
        total += split;
    }
    return total;
}

/// A runner finished if every one of @p checkpoints splits is positive.
inline bool finished(const Runner& runner, std::size_t checkpoints) {
    if (runner.split_seconds.size() != checkpoints) {
        return false;
    }
    for (const int split : runner.split_seconds) {
        if (split <= 0) {
            return false;
        }
    }
    return true;
}

/// Finishers ordered by total time; `continue` skips everyone else.
inline std::vector<std::pair<std::string, int>> rank_finishers(const std::vector<Runner>& runners,
                                                               std::size_t checkpoints) {
    std::vector<std::pair<std::string, int>> ranking;
    for (const auto& runner : runners) {
        if (!finished(runner, checkpoints)) {
            continue;  // DNF: nothing more to do for this runner
        }
        ranking.emplace_back(runner.name, total_seconds(runner));
    }
    std::stable_sort(ranking.begin(), ranking.end(), [](const auto& a, const auto& b) { return a.second < b.second; });
    return ranking;
}

/// Name of the first runner (in start order) with a total under @p limit; stops scanning at once.
inline std::optional<std::string> first_under(const std::vector<Runner>& runners, int limit,
                                              std::size_t checkpoints) {
    std::optional<std::string> found;
    for (std::size_t i = 0; i < runners.size(); ++i) {  // size_t matches runners.size()
        if (finished(runners[i], checkpoints) && total_seconds(runners[i]) < limit) {
            found = runners[i].name;
            break;
        }
    }
    return found;
}

/// Finish time in seconds for every (distance, pace) pair: an outer and an inner counted loop.
inline std::vector<std::vector<int>> pace_chart(const std::vector<int>& distances_km,
                                                const std::vector<int>& paces_sec_per_km) {
    std::vector<std::vector<int>> chart(distances_km.size(), std::vector<int>(paces_sec_per_km.size()));
    for (std::size_t row = 0; row < distances_km.size(); ++row) {
        for (std::size_t col = 0; col < paces_sec_per_km.size(); ++col) {
            chart[row][col] = distances_km[row] * paces_sec_per_km[col];
        }
    }
    return chart;
}

/// The splits from last to first. `i-- > 0` tests before decrementing, so an unsigned index never wraps.
inline std::vector<int> splits_in_reverse(const Runner& runner) {
    std::vector<int> reversed;
    for (std::size_t i = runner.split_seconds.size(); i-- > 0;) {
        reversed.push_back(runner.split_seconds[i]);
    }
    return reversed;
}

/// 3725 -> "1:02:05".
inline std::string format_time(int seconds) {
    std::ostringstream out;
    out << seconds / 3600 << ':' << std::setw(2) << std::setfill('0') << seconds % 3600 / 60 << ':'
        << std::setw(2) << seconds % 60;
    return out.str();
}

/// The interactive demo: "name split1 split2 split3" per runner, then the results board.
inline int run(std::istream& in, std::ostream& out) {
    constexpr std::size_t checkpoints = 3;
    out << "Day 13 – For Loops\nEnter 'name split1 split2 split3' in seconds (0 = missed mat), blank to finish\n";
    std::vector<Runner> runners;
    while (auto line = prompt_line(in, out, "runner> ")) {
        if (line->empty()) {
            break;
        }
        std::istringstream words(*line);
        Runner runner;
        words >> runner.name;
        for (int split = 0; words >> split;) {
            runner.split_seconds.push_back(split);
        }
        if (runner.name.empty()) {
            continue;
        }
        runners.push_back(runner);
    }
    int place = 1;
    for (const auto& [name, seconds] : rank_finishers(runners, checkpoints)) {
        out << "  " << place++ << ". " << std::left << std::setw(10) << name << format_time(seconds) << '\n';
    }
    if (auto fast = first_under(runners, 3 * 3600, checkpoints)) {
        out << "First sub-3-hour runner in start order: " << *fast << '\n';
    }
    const std::vector<int> paces{240, 300, 360};
    const auto chart = pace_chart({10, 21, 42}, paces);
    out << "Pace chart (km x min/km):\n";
    for (const auto& row : chart) {
        for (const int cell : row) {
            out << "  " << format_time(cell);
        }
        out << '\n';
    }
    return 0;
}

}  // namespace cppm::day13
