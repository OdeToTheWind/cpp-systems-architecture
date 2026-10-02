/**
 * @file
 * Day 95 – Capstone: Performance Module.
 *
 * Scenario: *nearest available driver* in a ride-hailing app. Scanning every driver for every
 * ride request is simple and correct, but with 50,000 drivers and thousands of requests a second
 * it is too slow. A uniform grid index answers the same question by looking only at nearby cells.
 * The fast version is trusted only because it is cross-checked against the naive one on many
 * random cases, and the speed-up is measured, not assumed.
 *
 * Deliverables (syllabus):
 * - A naive baseline
 * - A spatial index
 * - Benchmarks
 * - Cross-checking fast against slow
 */
#pragma once

#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <istream>
#include <optional>
#include <ostream>
#include <random>
#include <sstream>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>

#include "cppm/lesson.hpp"

namespace cppm::day95 {

inline constexpr Deliverable DELIVERABLES[] = {
    {"the obviously-correct linear scan", "nearest_naive"},
    {"bucketing points into grid cells", "GridIndex"},
    {"ring search with a provable stopping rule", "GridIndex::nearest"},
    {"keeping the index current as drivers move", "GridIndex::move"},
    {"randomised cross-checking against the baseline", "cross_check"},
};

struct Driver {
    int id;
    double x;  // km east of the city centre
    double y;  // km north
};

inline double dist2(double ax, double ay, double bx, double by) {
    const double dx = ax - bx;
    const double dy = ay - by;
    return dx * dx + dy * dy;
}

/// Closer is better; on an exact tie the smaller id wins, so both implementations agree.
inline bool better(double d, int id, double best_d, int best_id) { return d < best_d || (d == best_d && id < best_id); }

/// O(n) per query. @p distance_checks counts the work.
inline std::optional<int> nearest_naive(const std::vector<Driver>& drivers, double x, double y, std::uint64_t& distance_checks) {
    std::optional<int> best;
    double best_d = 0;
    for (const auto& d : drivers) {
        ++distance_checks;
        const double dd = dist2(d.x, d.y, x, y);
        if (!best || better(dd, d.id, best_d, *best)) {
            best = d.id;
            best_d = dd;
        }
    }
    return best;
}

/// Drivers bucketed into square cells of side @p cell_km. A query looks at the query's cell,
/// then rings of cells around it, and stops once no unvisited cell can hold anything closer.
class GridIndex {
public:
    explicit GridIndex(double cell_km) : cell_(cell_km) {
        if (!(cell_km > 0)) throw std::invalid_argument("cell size must be positive");
    }

    void insert(const Driver& d) {
        if (positions_.contains(d.id)) throw std::invalid_argument("driver " + std::to_string(d.id) + " already indexed");
        positions_[d.id] = d;
        cells_[key(cell_of(d.x), cell_of(d.y))].push_back(d.id);
    }
    void remove(int id) {
        const auto it = positions_.find(id);
        if (it == positions_.end()) return;
        auto& bucket = cells_[key(cell_of(it->second.x), cell_of(it->second.y))];
        bucket.erase(std::find(bucket.begin(), bucket.end(), id));
        positions_.erase(it);
    }
    /// Drivers move constantly; only a change of cell touches the buckets.
    void move(int id, double x, double y) {
        const auto it = positions_.find(id);
        if (it == positions_.end()) throw std::invalid_argument("unknown driver " + std::to_string(id));
        if (cell_of(it->second.x) == cell_of(x) && cell_of(it->second.y) == cell_of(y)) {
            it->second.x = x;
            it->second.y = y;
            return;
        }
        remove(id);
        insert({id, x, y});
    }

    std::optional<int> nearest(double x, double y, std::uint64_t& distance_checks) const {
        if (positions_.empty()) return std::nullopt;
        const std::int64_t cx = cell_of(x);
        const std::int64_t cy = cell_of(y);
        std::optional<int> best;
        double best_d = 0;
        for (std::int64_t r = 0;; ++r) {
            // Everything in ring r or beyond is at least (r - 1) * cell away from the query point,
            // because the query may sit anywhere inside its own cell.
            if (best && r > 0) {
                const double reach = static_cast<double>(r - 1) * cell_;
                if (reach * reach > best_d) break;
            }
            if (r > max_ring()) break;
            for (std::int64_t gx = cx - r; gx <= cx + r; ++gx) {
                for (std::int64_t gy = cy - r; gy <= cy + r; ++gy) {
                    if (std::max(std::llabs(gx - cx), std::llabs(gy - cy)) != r) continue;  // only the ring's border
                    const auto cell = cells_.find(key(gx, gy));
                    if (cell == cells_.end()) continue;
                    for (const int id : cell->second) {
                        ++distance_checks;
                        const Driver& d = positions_.at(id);
                        const double dd = dist2(d.x, d.y, x, y);
                        if (!best || better(dd, id, best_d, *best)) {
                            best = id;
                            best_d = dd;
                        }
                    }
                }
            }
        }
        return best;
    }
    std::size_t size() const { return positions_.size(); }

private:
    std::int64_t cell_of(double v) const { return static_cast<std::int64_t>(std::floor(v / cell_)); }
    static std::uint64_t key(std::int64_t gx, std::int64_t gy) {
        return (static_cast<std::uint64_t>(gx) << 32) ^ (static_cast<std::uint64_t>(gy) & 0xFFFFFFFFu);
    }
    std::int64_t max_ring() const { return 1 << 20; }  // a safety bound far beyond any city

    double cell_;
    std::unordered_map<int, Driver> positions_;
    std::unordered_map<std::uint64_t, std::vector<int>> cells_;
};

/// @p n drivers spread over a 40 x 40 km city, clustered around a few hubs like real traffic.
inline std::vector<Driver> random_drivers(int n, std::uint32_t seed) {
    std::mt19937 rng(seed);
    std::normal_distribution<double> spread(0.0, 3.0);
    std::uniform_int_distribution<int> hub(0, 3);
    const double hubs[4][2] = {{0, 0}, {8, 5}, {-10, 7}, {4, -12}};
    std::vector<Driver> drivers;
    for (int i = 0; i < n; ++i) {
        const auto h = static_cast<std::size_t>(hub(rng));
        drivers.push_back({i, std::clamp(hubs[h][0] + spread(rng), -20.0, 20.0), std::clamp(hubs[h][1] + spread(rng), -20.0, 20.0)});
    }
    return drivers;
}

struct CrossCheck {
    int queries = 0;
    int mismatches = 0;
    std::uint64_t naive_checks = 0;
    std::uint64_t grid_checks = 0;
};

/// Run @p queries random requests through both implementations and count disagreements.
inline CrossCheck cross_check(int drivers, int queries, double cell_km, std::uint32_t seed) {
    const auto fleet = random_drivers(drivers, seed);
    GridIndex index(cell_km);
    for (const auto& d : fleet) index.insert(d);
    std::mt19937 rng(seed + 1);
    std::uniform_real_distribution<double> coord(-25.0, 25.0);  // includes requests outside the city
    CrossCheck result;
    for (int q = 0; q < queries; ++q) {
        const double x = coord(rng);
        const double y = coord(rng);
        if (nearest_naive(fleet, x, y, result.naive_checks) != index.nearest(x, y, result.grid_checks)) ++result.mismatches;
        ++result.queries;
    }
    return result;
}

/// Median wall time of @p reps runs of @p work, in microseconds (demo only; tests never time).
template <typename Work>
long long median_micros(Work work, int reps) {
    std::vector<long long> times;
    for (int i = 0; i < reps; ++i) {
        const auto start = std::chrono::steady_clock::now();
        work();
        times.push_back(std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now() - start).count());
    }
    std::sort(times.begin(), times.end());
    return times[times.size() / 2];
}

/// The interactive demo: "<drivers> <queries> <cell_km>" cross-checks and times both versions.
inline int run(std::istream& in, std::ostream& out) {
    out << "Day 95 – Capstone: Performance Module\n";
    while (auto line = prompt_line(in, out, "drivers queries cell_km> ")) {
        std::istringstream words(*line);
        int drivers = 0;
        int queries = 0;
        double cell = 0;
        if (!(words >> drivers >> queries >> cell) || drivers <= 0 || queries <= 0 || cell <= 0) break;
        const auto check = cross_check(drivers, queries, cell, 95);
        out << "  " << check.mismatches << " mismatch(es) in " << check.queries << " queries; distance checks naive "
            << check.naive_checks << " vs grid " << check.grid_checks << '\n';
        const auto fleet = random_drivers(drivers, 95);
        GridIndex index(cell);
        for (const auto& d : fleet) index.insert(d);
        std::uint64_t sink = 0;
        const auto naive_us = median_micros([&] { nearest_naive(fleet, 1.0, 2.0, sink); }, 5);
        const auto grid_us = median_micros([&] { index.nearest(1.0, 2.0, sink); }, 5);
        out << "  one query: naive " << naive_us << " us, grid " << grid_us << " us\n";
    }
    return 0;
}

}  // namespace cppm::day95
