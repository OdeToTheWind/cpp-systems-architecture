/**
 * @file
 * Day 76 – Atomics & Memory Order.
 *
 * Scenario: the *live metrics* of a busy web server. Every request thread bumps counters and
 * records its latency; a dashboard thread reads them. Taking a mutex on every request would make
 * the metrics the bottleneck, so they use atomics: relaxed counters, a compare-exchange loop for
 * the slowest request, a spin lock, and a release/acquire handshake that publishes a config
 * snapshot safely.
 *
 * Deliverables (syllabus):
 * - std::atomic
 * - Compare-exchange
 * - Memory ordering
 * - Lock-free counters and flags
 */
#pragma once

#include <atomic>
#include <cstdint>
#include <istream>
#include <optional>
#include <ostream>
#include <sstream>
#include <string>
#include <thread>
#include <utility>
#include <vector>

#include "cppm/lesson.hpp"

namespace cppm::day76 {

inline constexpr Deliverable DELIVERABLES[] = {
    {"a lock-free counter with relaxed ordering", "Metrics::record"},
    {"a compare-exchange loop that tracks a maximum", "update_max"},
    {"a spin lock built from atomic_flag with acquire and release", "SpinLock"},
    {"publishing data with a release store and an acquire load", "Mailbox::publish"},
    {"running request threads against shared metrics", "simulate_traffic"},
};

/// Raise @p target to @p value if larger. compare_exchange_weak reloads `current` on failure,
/// so the loop retries with the latest value until it wins or the value is no longer larger.
inline bool update_max(std::atomic<std::uint64_t>& target, std::uint64_t value) {
    std::uint64_t current = target.load(std::memory_order_relaxed);
    while (value > current) {
        if (target.compare_exchange_weak(current, value, std::memory_order_relaxed)) return true;
    }
    return false;
}

/// Counters that only need to be *eventually* correct totals use memory_order_relaxed: each
/// increment is atomic, but it orders nothing else, which makes it the cheapest choice.
class Metrics {
  public:
    void record(std::uint64_t latency_us, bool error) {
        requests_.fetch_add(1, std::memory_order_relaxed);
        if (error) errors_.fetch_add(1, std::memory_order_relaxed);
        total_latency_.fetch_add(latency_us, std::memory_order_relaxed);
        update_max(slowest_, latency_us);
    }
    std::uint64_t requests() const { return requests_.load(std::memory_order_relaxed); }
    std::uint64_t errors() const { return errors_.load(std::memory_order_relaxed); }
    std::uint64_t slowest() const { return slowest_.load(std::memory_order_relaxed); }
    double mean_latency() const {
        const auto n = requests();
        return n == 0 ? 0.0
                      : static_cast<double>(total_latency_.load(std::memory_order_relaxed)) / static_cast<double>(n);
    }

  private:
    std::atomic<std::uint64_t> requests_{0};
    std::atomic<std::uint64_t> errors_{0};
    std::atomic<std::uint64_t> total_latency_{0};
    std::atomic<std::uint64_t> slowest_{0};
};

/// A minimal spin lock. test_and_set with *acquire* makes the previous owner's writes visible;
/// clear with *release* publishes ours to the next owner. Meets the Lockable requirements, so
/// it works with std::lock_guard. Spinning wastes CPU – only for very short critical sections.
class SpinLock {
  public:
    void lock() noexcept {
        while (flag_.test_and_set(std::memory_order_acquire)) {
            while (flag_.test(std::memory_order_relaxed))
                std::this_thread::yield();  // wait without hammering the cache line
        }
    }
    bool try_lock() noexcept { return !flag_.test_and_set(std::memory_order_acquire); }
    void unlock() noexcept { flag_.clear(std::memory_order_release); }

  private:
    std::atomic_flag flag_;  // C++20: default-initialised to clear
};

struct Config {
    std::string banner;
    int max_connections;
};

/// One writer publishes a Config once; readers see either nothing or the complete value.
/// The release store on `ready_` orders the plain write to `value_` before it; a reader that
/// sees ready_ == true through an acquire load therefore also sees the finished value.
class Mailbox {
  public:
    void publish(Config config) {
        value_ = std::move(config);                     // 1. plain write
        ready_.store(true, std::memory_order_release);  // 2. publish
    }
    std::optional<Config> try_read() const {
        if (!ready_.load(std::memory_order_acquire)) return std::nullopt;
        return value_;  // safe: happens-after the write in publish()
    }

  private:
    Config value_;
    std::atomic<bool> ready_{false};
};

/// @p threads request threads each record @p per_thread requests with deterministic latencies.
inline void simulate_traffic(Metrics& metrics, int threads, int per_thread) {
    std::vector<std::thread> workers;
    for (int t = 0; t < threads; ++t) {
        workers.emplace_back([&metrics, t, per_thread] {
            for (int i = 0; i < per_thread; ++i) {
                const auto latency = static_cast<std::uint64_t>(100 + (t * 7919 + i * 104729) % 900);
                metrics.record(latency, i % 50 == 0);
            }
        });
    }
    for (auto& w : workers) w.join();
}

/// The interactive demo: "threads requests" simulates traffic and prints the dashboard.
inline int run(std::istream& in, std::ostream& out) {
    out << "Day 76 – Atomics & Memory Order\n";
    out << "lock-free 64-bit atomics on this platform: " << std::boolalpha
        << std::atomic<std::uint64_t>::is_always_lock_free << '\n';
    while (auto line = prompt_line(in, out, "threads requests-per-thread> ")) {
        std::istringstream words(*line);
        int threads = 0;
        int per_thread = 0;
        if (!(words >> threads >> per_thread) || threads <= 0 || per_thread < 0) break;
        Metrics metrics;
        simulate_traffic(metrics, threads, per_thread);
        out << "  " << metrics.requests() << " request(s), " << metrics.errors() << " error(s), slowest "
            << metrics.slowest() << " us\n";
    }
    return 0;
}

}  // namespace cppm::day76
