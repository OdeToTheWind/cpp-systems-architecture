/**
 * @file
 * Day 80 – Profiling & Performance.
 *
 * Scenario: the nightly *delivery-route analytics* job of a courier company has grown from
 * minutes to hours. Before changing anything, it gets a benchmark harness that reports medians
 * rather than single noisy runs. Then three classic wins are measured: a better algorithm
 * (duplicate parcel IDs), a better data layout (structure of arrays) and a cache-friendly loop
 * order.
 *
 * Deliverables (syllabus):
 * - Measuring with std::chrono
 * - Benchmark harnesses
 * - Data layout and cache locality
 * - Algorithmic wins
 */
#pragma once

#include <algorithm>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <istream>
#include <ostream>
#include <random>
#include <sstream>
#include <stdexcept>
#include <string>
#include <unordered_set>
#include <vector>

#include "cppm/lesson.hpp"

namespace cppm::day80 {

inline constexpr Deliverable DELIVERABLES[] = {
    {"a harness with warm-up runs, repetitions and a median", "benchmark"},
    {"stopping the optimiser from deleting measured work", "keep"},
    {"an O(n^2) and an O(n) duplicate check with operation counts", "has_duplicate_hashed"},
    {"array-of-structs versus struct-of-arrays layouts", "StopsSoA"},
    {"row-major versus column-major traversal", "sum_column_major"},
};

using Nanos = std::chrono::nanoseconds;
using ClockFn = std::function<Nanos()>;

/// The real clock: steady_clock never jumps backwards, unlike system_clock.
inline Nanos steady_now() {
    return std::chrono::duration_cast<Nanos>(std::chrono::steady_clock::now().time_since_epoch());
}

struct BenchResult {
    Nanos min;
    Nanos median;
    Nanos max;
    int runs;
};

/// Time @p work @p repetitions times after @p warmup untimed runs and summarise. The median is
/// robust against the occasional slow run caused by the OS; the clock is injectable for tests.
inline BenchResult benchmark(const std::function<void()>& work, int repetitions, int warmup = 1,
                             const ClockFn& clock = steady_now) {
    if (repetitions <= 0) throw std::invalid_argument("need at least one repetition");
    for (int i = 0; i < warmup; ++i) work();  // fill caches, trigger lazy initialisation
    std::vector<Nanos> times;
    for (int i = 0; i < repetitions; ++i) {
        const Nanos start = clock();
        work();
        times.push_back(clock() - start);
    }
    std::sort(times.begin(), times.end());
    const auto n = times.size();
    const Nanos median = n % 2 ? times[n / 2] : (times[n / 2 - 1] + times[n / 2]) / 2;
    return {times.front(), median, times.back(), repetitions};
}

/// Writes to a volatile object are observable behaviour, so the optimiser must keep them.
inline volatile std::uintptr_t benchmark_sink = 0;

/// Make a result observable so the compiler cannot remove the computation that produced it.
template <typename T>
void keep(const T& value) {
    benchmark_sink = static_cast<std::uintptr_t>(value);
}

/// Naive duplicate check: compares every pair. @p comparisons reports the work done.
inline bool has_duplicate_naive(const std::vector<std::uint64_t>& ids, std::uint64_t& comparisons) {
    comparisons = 0;
    for (std::size_t i = 0; i < ids.size(); ++i) {
        for (std::size_t j = i + 1; j < ids.size(); ++j) {
            ++comparisons;
            if (ids[i] == ids[j]) return true;
        }
    }
    return false;
}

/// One pass with a hash set: expected O(n). @p lookups reports the work done.
inline bool has_duplicate_hashed(const std::vector<std::uint64_t>& ids, std::uint64_t& lookups) {
    lookups = 0;
    std::unordered_set<std::uint64_t> seen;
    seen.reserve(ids.size());
    for (const auto id : ids) {
        ++lookups;
        if (!seen.insert(id).second) return true;
    }
    return false;
}

/// Array of structs: natural to write, but summing one field drags every other field through the cache.
struct StopAoS {
    double lat;
    double lon;
    std::int32_t parcels;
    char notes[52];  // rarely-read data padding each record to 72 bytes
};

/// Struct of arrays: each field is contiguous, so a loop over one field reads only that field.
struct StopsSoA {
    std::vector<double> lat;
    std::vector<double> lon;
    std::vector<std::int32_t> parcels;
    std::vector<std::string> notes;
    std::size_t size() const { return parcels.size(); }
    void push_back(double la, double lo, std::int32_t p) {
        lat.push_back(la);
        lon.push_back(lo);
        parcels.push_back(p);
        notes.emplace_back();
    }
};

inline std::int64_t total_parcels(const std::vector<StopAoS>& stops) {
    std::int64_t total = 0;
    for (const auto& s : stops) total += s.parcels;
    return total;
}

inline std::int64_t total_parcels(const StopsSoA& stops) {
    std::int64_t total = 0;
    for (const auto p : stops.parcels) total += p;
    return total;
}

/// A rows x cols matrix stored row by row. Walking along a row touches consecutive memory.
inline std::int64_t sum_row_major(const std::vector<std::int32_t>& m, std::size_t rows, std::size_t cols) {
    std::int64_t total = 0;
    for (std::size_t r = 0; r < rows; ++r) {
        for (std::size_t c = 0; c < cols; ++c) total += m[r * cols + c];
    }
    return total;
}

/// Same sum, column by column: every step jumps cols * 4 bytes, missing the cache for large matrices.
inline std::int64_t sum_column_major(const std::vector<std::int32_t>& m, std::size_t rows, std::size_t cols) {
    std::int64_t total = 0;
    for (std::size_t c = 0; c < cols; ++c) {
        for (std::size_t r = 0; r < rows; ++r) total += m[r * cols + c];
    }
    return total;
}

/// Deterministic test data: unique parcel IDs from a fixed seed, optionally with one repeat.
inline std::vector<std::uint64_t> parcel_ids(std::size_t n, bool with_duplicate, std::uint32_t seed = 80) {
    std::mt19937_64 rng(seed);
    std::vector<std::uint64_t> ids(n);
    for (std::size_t i = 0; i < n; ++i) ids[i] = (rng() << 20) | i;  // low bits keep them unique
    if (with_duplicate && n > 1) ids[n - 1] = ids[n / 2];
    return ids;
}

inline std::string micros(Nanos d) {
    return std::to_string(d.count() / 1000) + " us";
}

/// The interactive demo: enter n to time both duplicate checks and both loop orders on real data.
inline int run(std::istream& in, std::ostream& out) {
    out << "Day 80 – Profiling & Performance\n";
    while (auto line = prompt_line(in, out, "n (e.g. 2000)> ")) {
        std::istringstream words(*line);
        std::size_t n = 0;
        if (!(words >> n) || n == 0 || n > 20000) break;
        const auto ids = parcel_ids(n, false);
        std::uint64_t naive_ops = 0;
        std::uint64_t hashed_ops = 0;
        const auto naive = benchmark([&] { keep(has_duplicate_naive(ids, naive_ops)); }, 5);
        const auto hashed = benchmark([&] { keep(has_duplicate_hashed(ids, hashed_ops)); }, 5);
        out << "  naive:  " << naive_ops << " comparisons, median " << micros(naive.median) << '\n';
        out << "  hashed: " << hashed_ops << " lookups, median " << micros(hashed.median) << '\n';
        const std::vector<std::int32_t> grid(n * 256, 1);
        const auto rows = benchmark([&] { keep(sum_row_major(grid, n, 256)); }, 5);
        const auto cols = benchmark([&] { keep(sum_column_major(grid, n, 256)); }, 5);
        out << "  row-major " << micros(rows.median) << ", column-major " << micros(cols.median) << '\n';
    }
    return 0;
}

}  // namespace cppm::day80
