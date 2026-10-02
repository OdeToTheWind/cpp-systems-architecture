// Tests for Day 73 – Concurrency: Threads & Mutexes. Assertions check totals and invariants,
// which hold for every interleaving, never a particular schedule.
#include <atomic>
#include <sstream>
#include <stdexcept>
#include <string>
#include <thread>
#include <vector>

#include "cppm/testing.hpp"
#include "day_73_threads/lesson.hpp"

using namespace cppm::day73;

TEST_CASE("parallel counting matches a single-threaded count") {
    std::vector<std::string> lines;
    for (int i = 0; i < 200; ++i) lines.push_back(i % 3 == 0 ? "ink paper ink" : "paper toner");
    const auto one = parallel_word_count(lines, 1);
    const auto many = parallel_word_count(lines, 8);
    CHECK(one == many);
    CHECK_EQ(many.at("ink"), 134);
    CHECK_EQ(many.at("paper"), 200);
    CHECK_THROWS_AS(parallel_word_count(lines, 0), std::invalid_argument);
}

TEST_CASE("concurrent opposite transfers neither deadlock nor lose money") {
    Account a;
    Account b;
    a.cents = 100000;
    b.cents = 100000;
    std::thread one([&] {
        for (int i = 0; i < 5000; ++i) transfer(a, b, 7);
    });
    std::thread two([&] {
        for (int i = 0; i < 5000; ++i) transfer(b, a, 5);
    });
    one.join();
    two.join();
    CHECK_EQ(a.cents + b.cents, 200000);
    CHECK_EQ(a.cents, 100000 - 5000 * 2);
    CHECK(!transfer(a, b, 1'000'000));  // insufficient funds
}

TEST_CASE("the queue is FIFO and pop ends after close") {
    BoundedQueue<int> queue(3);
    CHECK(queue.push(1));
    CHECK(queue.push(2));
    queue.close();
    CHECK(!queue.push(3));
    CHECK_EQ(queue.pop().value_or(-1), 1);
    CHECK_EQ(queue.pop().value_or(-1), 2);
    CHECK(!queue.pop().has_value());
    CHECK_THROWS_AS(BoundedQueue<int>(0), std::invalid_argument);
}

TEST_CASE("a full queue blocks the producer until a consumer makes room") {
    BoundedQueue<int> queue(1);
    queue.push(1);
    std::atomic<bool> second_pushed{false};
    std::thread producer([&] {
        queue.push(2);  // blocks: capacity 1
        second_pushed = true;
    });
    CHECK_EQ(queue.pop().value_or(-1), 1);  // makes room
    producer.join();
    CHECK(second_pushed.load());
    CHECK_EQ(queue.pop().value_or(-1), 2);
    CHECK_EQ(queue.high_water(), 1u);
}

TEST_CASE("an empty queue blocks consumers until close wakes them") {
    BoundedQueue<int> queue(2);
    std::atomic<int> finished{0};
    std::vector<std::thread> consumers;
    for (int i = 0; i < 3; ++i) {
        consumers.emplace_back([&] {
            while (queue.pop()) {
            }
            ++finished;
        });
    }
    queue.close();
    for (auto& t : consumers) t.join();
    CHECK_EQ(finished.load(), 3);
}

TEST_CASE("the print shop prints every page exactly once") {
    std::vector<Job> jobs;
    int expected = 0;
    for (int i = 1; i <= 500; ++i) {
        jobs.push_back({i, i % 7 + 1});
        expected += i % 7 + 1;
    }
    const auto report = run_print_shop(jobs, 3, 4, 5);
    CHECK_EQ(report.jobs_printed, 500);
    CHECK_EQ(report.total_pages, expected);
    int sum = 0;
    for (const auto& [printer, pages] : report.pages_per_printer) sum += pages;
    CHECK_EQ(sum, expected);
    CHECK(report.max_queue <= 5u);
    CHECK_THROWS_AS(run_print_shop(jobs, 0, 1, 1), std::invalid_argument);
}

TEST_CASE("run reports the shop totals") {
    std::istringstream in("20 2 3\n5 0 1\nstop\n");
    std::ostringstream out;
    CHECK_EQ(run(in, out), 0);
    CHECK(out.str().find("printed 20 job(s), 110 page(s); queue never above") != std::string::npos);
    CHECK(out.str().find("need desks and printers") != std::string::npos);
}
