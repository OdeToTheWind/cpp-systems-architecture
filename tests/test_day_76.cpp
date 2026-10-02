// Tests for Day 76 – Atomics & Memory Order. Totals must be exact for every interleaving.
#include <algorithm>
#include <atomic>
#include <cstdint>
#include <mutex>
#include <sstream>
#include <string>
#include <thread>
#include <vector>

#include "cppm/testing.hpp"
#include "day_76_atomics/lesson.hpp"

using namespace cppm::day76;

TEST_CASE("update_max only ever raises the value") {
    std::atomic<std::uint64_t> max{10};
    CHECK(update_max(max, 15));
    CHECK(!update_max(max, 12));
    CHECK(!update_max(max, 15));
    CHECK_EQ(max.load(), 15u);
}

TEST_CASE("concurrent maxima end at the true maximum") {
    std::atomic<std::uint64_t> max{0};
    std::vector<std::thread> threads;
    for (std::uint64_t t = 0; t < 8; ++t) {
        threads.emplace_back([&max, t] {
            for (std::uint64_t i = 0; i < 10000; ++i) update_max(max, (i * 31 + t) % 99991);
        });
    }
    for (auto& th : threads) th.join();
    std::uint64_t expected = 0;
    for (std::uint64_t t = 0; t < 8; ++t) {
        for (std::uint64_t i = 0; i < 10000; ++i) expected = std::max(expected, (i * 31 + t) % 99991);
    }
    CHECK_EQ(max.load(), expected);
}

TEST_CASE("relaxed counters give exact totals across threads") {
    Metrics metrics;
    simulate_traffic(metrics, 8, 5000);
    CHECK_EQ(metrics.requests(), 40000u);
    CHECK_EQ(metrics.errors(), 800u);  // every 50th request
    CHECK(metrics.slowest() <= 999u);
    CHECK(metrics.slowest() >= 900u);
    CHECK(metrics.mean_latency() > 100.0);
}

TEST_CASE("an empty metrics object reads as zero") {
    const Metrics metrics;
    CHECK_EQ(metrics.requests(), 0u);
    CHECK_NEAR(metrics.mean_latency(), 0.0, 1e-12);
}

TEST_CASE("the spin lock provides mutual exclusion") {
    SpinLock lock;
    long counter = 0;  // deliberately not atomic: only the lock protects it
    std::vector<std::thread> threads;
    for (int t = 0; t < 4; ++t) {
        threads.emplace_back([&] {
            for (int i = 0; i < 20000; ++i) {
                const std::lock_guard guard(lock);
                ++counter;
            }
        });
    }
    for (auto& th : threads) th.join();
    CHECK_EQ(counter, 80000);
    CHECK(lock.try_lock());
    CHECK(!lock.try_lock());
    lock.unlock();
}

TEST_CASE("release/acquire publishes a complete value") {
    Mailbox mailbox;
    CHECK(!mailbox.try_read().has_value());
    Config seen;
    std::thread reader([&] {
        while (true) {
            if (const auto config = mailbox.try_read()) {
                seen = *config;  // the join below makes this visible to the test thread
                return;
            }
            std::this_thread::yield();
        }
    });
    mailbox.publish({"welcome", 512});
    reader.join();
    CHECK_EQ(seen.banner, "welcome");  // never a half-written value
    CHECK_EQ(seen.max_connections, 512);
}

TEST_CASE("run prints the dashboard") {
    std::istringstream in("4 1000\nstop\n");
    std::ostringstream out;
    CHECK_EQ(run(in, out), 0);
    CHECK(out.str().find("4000 request(s), 80 error(s)") != std::string::npos);
}
