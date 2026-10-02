/**
 * @file
 * Day 45 – STL Algorithms.
 *
 * Scenario: an *e-sports league table*. Season results are turned into points, players under
 * the minimum number of matches are filtered out, disqualified players are removed, the table
 * is sorted with proper tie-breakers and split into promotion and relegation zones – each step
 * one standard algorithm instead of a hand-written loop.
 *
 * Deliverables (syllabus):
 * - transform, copy_if and accumulate
 * - sort with custom comparators
 * - partition
 * - erase-remove
 */
#pragma once

#include <algorithm>
#include <iomanip>
#include <istream>
#include <iterator>
#include <numeric>
#include <ostream>
#include <set>
#include <sstream>
#include <string>
#include <tuple>
#include <vector>

#include "cppm/lesson.hpp"

namespace cppm::day45 {

inline constexpr Deliverable DELIVERABLES[] = {
    {"std::transform turns records into derived values", "points_of"},
    {"std::copy_if keeps only qualifying players", "qualified"},
    {"std::accumulate folds a range into one value", "total_points"},
    {"erase-remove (and std::erase_if) deletes in one pass", "remove_disqualified"},
    {"std::sort with a multi-key comparator", "rank"},
    {"std::stable_partition splits into zones", "promotion_zone"},
};

struct Record {
    std::string player;
    int wins;
    int draws;
    int losses;
    int scored;
    int conceded;
    int points() const { return 3 * wins + draws; }
    int matches() const { return wins + draws + losses; }
    int difference() const { return scored - conceded; }
};

/// Points for every record, in the same order.
inline std::vector<int> points_of(const std::vector<Record>& table) {
    std::vector<int> points(table.size());
    std::transform(table.begin(), table.end(), points.begin(), [](const Record& r) { return r.points(); });
    return points;
}

/// Records with at least @p min_matches played.
inline std::vector<Record> qualified(const std::vector<Record>& table, int min_matches) {
    std::vector<Record> result;
    std::copy_if(table.begin(), table.end(), std::back_inserter(result),
                 [min_matches](const Record& r) { return r.matches() >= min_matches; });
    return result;
}

inline int total_points(const std::vector<Record>& table) {
    return std::accumulate(table.begin(), table.end(), 0, [](int sum, const Record& r) { return sum + r.points(); });
}

/// The erase-remove idiom: remove_if moves the survivors forward, erase cuts off the tail.
inline std::size_t remove_disqualified(std::vector<Record>& table, const std::set<std::string>& banned) {
    const auto old_size = table.size();
    table.erase(std::remove_if(table.begin(), table.end(), [&](const Record& r) { return banned.contains(r.player); }),
                table.end());
    return old_size - table.size();
}

/// Points, then goal difference, then goals scored (all descending), then name (ascending).
inline void rank(std::vector<Record>& table) {
    std::sort(table.begin(), table.end(), [](const Record& a, const Record& b) {
        // tuples compare lexicographically; negate to get descending order for the numbers
        return std::tuple(-a.points(), -a.difference(), -a.scored, a.player) <
               std::tuple(-b.points(), -b.difference(), -b.scored, b.player);
    });
}

/// Move everyone with at least @p points to the front, keeping the ranking order inside each zone.
/// Returns how many are in the promotion zone.
inline std::size_t promotion_zone(std::vector<Record>& ranked, int points) {
    const auto boundary =
        std::stable_partition(ranked.begin(), ranked.end(), [points](const Record& r) { return r.points() >= points; });
    return static_cast<std::size_t>(std::distance(ranked.begin(), boundary));
}

/// The interactive demo: "player wins draws losses scored conceded" lines, then END.
inline int run(std::istream& in, std::ostream& out) {
    out << "Day 45 – STL Algorithms\nResults as 'player W D L scored conceded', then END\n";
    std::vector<Record> table;
    while (auto line = prompt_line(in, out, "")) {
        if (*line == "END") break;
        std::istringstream words(*line);
        Record r{};
        if (words >> r.player >> r.wins >> r.draws >> r.losses >> r.scored >> r.conceded) {
            table.push_back(r);
        }
    }
    const auto removed = remove_disqualified(table, {"cheater"});
    auto league = qualified(table, 3);
    rank(league);
    const auto promoted = promotion_zone(league, 10);
    out << "removed " << removed << ", " << league.size() << " qualified, " << total_points(league)
        << " points in total\n";
    for (std::size_t i = 0; i < league.size(); ++i) {
        out << std::setw(2) << i + 1 << ". " << std::left << std::setw(10) << league[i].player << std::right
            << std::setw(3) << league[i].points() << (i < promoted ? "  ^ promoted" : "") << '\n';
    }
    return 0;
}

}  // namespace cppm::day45
