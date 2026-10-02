/**
 * @file
 * Day 73 – Concurrency: Threads & Mutexes.
 *
 * Scenario: a *print shop's job server*. Several front desks submit print jobs into a bounded
 * queue, several printers take jobs from it, and the shop counts pages per printer. The queue
 * blocks producers when it is full and consumers when it is empty – a classic producer-consumer
 * design built from std::thread, std::mutex and std::condition_variable.
 *
 * Deliverables (syllabus):
 * - std::thread
 * - mutex and lock_guard
 * - Condition variables
 * - Producer-consumer queues
 */
#pragma once

#include <condition_variable>
#include <cstddef>
#include <deque>
#include <istream>
#include <map>
#include <mutex>
#include <optional>
#include <ostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <thread>
#include <utility>
#include <vector>

#include "cppm/lesson.hpp"

namespace cppm::day73 {

inline constexpr Deliverable DELIVERABLES[] = {
    {"splitting work across std::thread and merging under a lock_guard", "parallel_word_count"},
    {"locking two mutexes without deadlock", "transfer"},
    {"a bounded queue whose push waits on a condition variable", "BoundedQueue::push"},
    {"a pop that ends cleanly when the queue is closed", "BoundedQueue::pop"},
    {"producers and consumers sharing the queue", "run_print_shop"},
};

/// Count words in parallel: each thread counts its own slice into a private map (no sharing,
/// no locking), then merges into the shared result while holding the mutex.
inline std::map<std::string, int> parallel_word_count(const std::vector<std::string>& lines, unsigned threads) {
    if (threads == 0) throw std::invalid_argument("need at least one thread");
    std::map<std::string, int> total;
    std::mutex total_mutex;
    std::vector<std::thread> workers;
    for (unsigned t = 0; t < threads; ++t) {
        workers.emplace_back([&, t] {
            std::map<std::string, int> local;
            for (std::size_t i = t; i < lines.size(); i += threads) {
                std::istringstream words(lines[i]);
                for (std::string w; words >> w;) ++local[w];
            }
            const std::lock_guard<std::mutex> lock(total_mutex);  // unlocked automatically, even on exceptions
            for (const auto& [word, n] : local) total[word] += n;
        });
    }
    for (auto& worker : workers) worker.join();  // never let a joinable std::thread be destroyed
    return total;
}

struct Account {
    std::mutex mutex;
    long cents = 0;
};

/// Move money between two accounts. std::scoped_lock locks both mutexes with a deadlock-avoidance
/// algorithm, so transfer(a, b) and transfer(b, a) running at the same time cannot deadlock.
inline bool transfer(Account& from, Account& to, long cents) {
    if (&from == &to) return true;
    const std::scoped_lock lock(from.mutex, to.mutex);
    if (from.cents < cents) return false;
    from.cents -= cents;
    to.cents += cents;
    return true;
}

/// A thread-safe FIFO with a capacity. push() waits while full; pop() waits while empty and
/// returns nullopt once the queue is closed and drained.
template <typename T>
class BoundedQueue {
public:
    explicit BoundedQueue(std::size_t capacity) : capacity_(capacity) {
        if (capacity == 0) throw std::invalid_argument("capacity must be positive");
    }
    /// Returns false if the queue was closed (the item is not added).
    bool push(T item) {
        std::unique_lock lock(mutex_);
        not_full_.wait(lock, [this] { return items_.size() < capacity_ || closed_; });  // predicate handles spurious wake-ups
        if (closed_) return false;
        items_.push_back(std::move(item));
        high_water_ = std::max(high_water_, items_.size());
        lock.unlock();
        not_empty_.notify_one();
        return true;
    }
    std::optional<T> pop() {
        std::unique_lock lock(mutex_);
        not_empty_.wait(lock, [this] { return !items_.empty() || closed_; });
        if (items_.empty()) return std::nullopt;  // closed and drained
        T item = std::move(items_.front());
        items_.pop_front();
        lock.unlock();
        not_full_.notify_one();
        return item;
    }
    /// No more pushes; consumers finish what is queued and then stop.
    void close() {
        {
            const std::lock_guard lock(mutex_);
            closed_ = true;
        }
        not_empty_.notify_all();
        not_full_.notify_all();
    }
    std::size_t high_water() const {
        const std::lock_guard lock(mutex_);
        return high_water_;
    }

private:
    mutable std::mutex mutex_;
    std::condition_variable not_full_;
    std::condition_variable not_empty_;
    std::deque<T> items_;
    std::size_t capacity_;
    std::size_t high_water_ = 0;
    bool closed_ = false;
};

struct Job {
    int id;
    int pages;
};

struct ShopReport {
    std::map<int, int> pages_per_printer;  // printer number -> pages
    int jobs_printed = 0;
    int total_pages = 0;
    std::size_t max_queue = 0;
};

/// @p desks producer threads submit @p jobs round-robin; @p printers consumer threads print them.
inline ShopReport run_print_shop(const std::vector<Job>& jobs, int desks, int printers, std::size_t capacity) {
    if (desks <= 0 || printers <= 0) throw std::invalid_argument("need desks and printers");
    BoundedQueue<Job> queue(capacity);
    ShopReport report;
    std::mutex report_mutex;
    std::vector<std::thread> consumers;
    for (int p = 1; p <= printers; ++p) {
        consumers.emplace_back([&, p] {
            while (auto job = queue.pop()) {
                const std::lock_guard lock(report_mutex);
                report.pages_per_printer[p] += job->pages;
                ++report.jobs_printed;
                report.total_pages += job->pages;
            }
        });
    }
    std::vector<std::thread> producers;
    for (int d = 0; d < desks; ++d) {
        producers.emplace_back([&, d] {
            for (std::size_t i = static_cast<std::size_t>(d); i < jobs.size(); i += static_cast<std::size_t>(desks)) queue.push(jobs[i]);
        });
    }
    for (auto& t : producers) t.join();
    queue.close();  // only after every producer is done
    for (auto& t : consumers) t.join();
    report.max_queue = queue.high_water();
    return report;
}

/// The interactive demo: "jobs desks printers" runs the shop with jobs of 1..10 pages.
inline int run(std::istream& in, std::ostream& out) {
    out << "Day 73 – Concurrency: Threads & Mutexes\n";
    while (auto line = prompt_line(in, out, "jobs desks printers> ")) {
        std::istringstream words(*line);
        int count = 0;
        int desks = 0;
        int printers = 0;
        if (!(words >> count >> desks >> printers)) break;
        std::vector<Job> jobs;
        for (int i = 0; i < count; ++i) jobs.push_back({i + 1, i % 10 + 1});
        try {
            const auto report = run_print_shop(jobs, desks, printers, 4);
            out << "  printed " << report.jobs_printed << " job(s), " << report.total_pages << " page(s); queue never above "
                << report.max_queue << '\n';
        } catch (const std::invalid_argument& error) {
            out << "  " << error.what() << '\n';
        }
    }
    return 0;
}

}  // namespace cppm::day73
