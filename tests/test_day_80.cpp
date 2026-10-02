// Tests for Day 80 – Profiling & Performance. Timings are never asserted (they depend on the
// machine); a fake clock tests the harness, and operation counts show the complexity.
#include <chrono>
#include <cstdint>
#include <numeric>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

#include "cppm/testing.hpp"
#include "day_80_performance/lesson.hpp"

using namespace cppm::day80;
using namespace std::chrono_literals;

TEST_CASE("the harness warms up, repeats and reports a median") {
    std::vector<Nanos> durations{50ns, 10ns, 30ns, 1000ns, 20ns};  // one outlier
    std::size_t run_index = 0;
    Nanos now{0};
    int calls = 0;
    const auto result = benchmark(
        [&] {
            ++calls;
            if (calls > 2) now += durations[run_index++];  // the first two calls are warm-up
        },
        5, 2, [&] { return now; });
    CHECK_EQ(calls, 7);
    CHECK(result.min == 10ns);
    CHECK(result.median == 30ns);  // the 1000 ns outlier does not move the median
    CHECK(result.max == 1000ns);
    CHECK_THROWS_AS(benchmark([] {}, 0), std::invalid_argument);
}

TEST_CASE("an even number of runs averages the middle two") {
    Nanos now{0};
    std::vector<Nanos> durations{10ns, 40ns, 20ns, 30ns};
    std::size_t i = 0;
    const auto result = benchmark([&] { now += durations[i++]; }, 4, 0, [&] { return now; });
    CHECK(result.median == 25ns);
}

TEST_CASE("both duplicate checks agree") {
    for (const bool dup : {false, true}) {
        const auto ids = parcel_ids(500, dup);
        std::uint64_t a = 0;
        std::uint64_t b = 0;
        CHECK_EQ(has_duplicate_naive(ids, a), dup);
        CHECK_EQ(has_duplicate_hashed(ids, b), dup);
    }
}

TEST_CASE("the naive check grows quadratically, the hashed one linearly") {
    std::uint64_t naive_small = 0;
    std::uint64_t naive_big = 0;
    std::uint64_t hashed_small = 0;
    std::uint64_t hashed_big = 0;
    has_duplicate_naive(parcel_ids(1000, false), naive_small);
    has_duplicate_naive(parcel_ids(2000, false), naive_big);
    has_duplicate_hashed(parcel_ids(1000, false), hashed_small);
    has_duplicate_hashed(parcel_ids(2000, false), hashed_big);
    CHECK_EQ(naive_small, 499500u);  // n(n-1)/2
    CHECK_EQ(naive_big, 1999000u);   // twice the input, four times the work
    CHECK_EQ(hashed_small, 1000u);
    CHECK_EQ(hashed_big, 2000u);     // twice the input, twice the work
}

TEST_CASE("layouts give the same answer; SoA stores the hot field contiguously") {
    std::vector<StopAoS> aos;
    StopsSoA soa;
    for (int i = 0; i < 1000; ++i) {
        aos.push_back({1.0 * i, 2.0 * i, i % 7, {}});
        soa.push_back(1.0 * i, 2.0 * i, i % 7);
    }
    CHECK_EQ(total_parcels(aos), total_parcels(soa));
    CHECK(sizeof(StopAoS) >= 72u);  // reading one int pulls a whole record into cache
    CHECK_EQ(&soa.parcels[1] - &soa.parcels[0], 1);
}

TEST_CASE("loop order changes speed, not results") {
    std::vector<std::int32_t> m(300 * 200);
    std::iota(m.begin(), m.end(), 0);
    const std::int64_t expected = 59999LL * 60000 / 2;
    CHECK_EQ(sum_row_major(m, 300, 200), expected);
    CHECK_EQ(sum_column_major(m, 300, 200), expected);
}

TEST_CASE("run prints operation counts for the requested size") {
    std::istringstream in("1000\nstop\n");
    std::ostringstream out;
    CHECK_EQ(run(in, out), 0);
    CHECK(out.str().find("naive:  499500 comparisons") != std::string::npos);
    CHECK(out.str().find("hashed: 1000 lookups") != std::string::npos);
    CHECK(out.str().find("row-major") != std::string::npos);
}
