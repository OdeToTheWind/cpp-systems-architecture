// Tests for Day 74 – Concurrency: Futures & Thread Pools.
#include <atomic>
#include <future>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

#include "cppm/testing.hpp"
#include "day_74_futures_thread_pools/lesson.hpp"

using namespace cppm::day74;

TEST_CASE("std::async results match the sequential computation") {
    const std::vector<std::string> files{"Wikipedia", "", std::string(10000, 'x')};
    const auto sums = checksums_async(files);
    CHECK_EQ(sums.size(), 3u);
    CHECK_EQ(sums[0], 0x11E60398u);  // the Adler-32 reference value for "Wikipedia"
    CHECK_EQ(sums[1], 1u);
    CHECK_EQ(sums[2], adler32(files[2]));
}

TEST_CASE("thumbnails keep their aspect ratio") {
    const auto t = make_thumbnail({"cat", 4000, 3000}, 256);
    CHECK_EQ(t.width, 256);
    CHECK_EQ(t.height, 192);
    CHECK_EQ(t.name, "cat@256");
    CHECK_EQ(make_thumbnail({"icon", 32, 16}, 64).width, 32);  // never upscaled
    CHECK_EQ(make_thumbnail({"strip", 10000, 1}, 64).height, 1);
    CHECK_THROWS_AS(make_thumbnail({"bad", 0, 10}, 64), std::invalid_argument);
}

TEST_CASE("a promise delivers a value or an exception to the future") {
    CHECK_EQ(fetch_metadata({"dog", 800, 600}).get(), "dog 800x600");
    auto failing = fetch_metadata({"", 1, 1});
    CHECK_THROWS_AS(failing.get(), std::runtime_error);
}

TEST_CASE("exceptions thrown in std::async are rethrown by get") {
    auto future = std::async(std::launch::async, [] { return make_thumbnail({"x", -1, 5}, 10); });
    CHECK_THROWS_AS(future.get(), std::invalid_argument);
}

TEST_CASE("the pool runs every task on a fixed number of threads") {
    std::atomic<int> sum{0};
    {
        ThreadPool pool(3);
        std::vector<std::future<int>> results;
        for (int i = 1; i <= 100; ++i) {
            results.push_back(pool.submit([i, &sum] {
                sum += i;
                return i * i;
            }));
        }
        long squares = 0;
        for (auto& r : results) squares += r.get();
        CHECK_EQ(squares, 338350L);
        CHECK_EQ(pool.size(), 3u);
    }  // destructor joins the workers
    CHECK_EQ(sum.load(), 5050);
    CHECK_THROWS_AS(ThreadPool(0), std::invalid_argument);
}

TEST_CASE("queued work still finishes when the pool is destroyed") {
    std::atomic<int> done{0};
    {
        ThreadPool pool(1);
        for (int i = 0; i < 20; ++i) static_cast<void>(pool.submit([&done] { ++done; }));
    }
    CHECK_EQ(done.load(), 20);
}

TEST_CASE("uploads collect thumbnails and errors in order") {
    ThreadPool pool(2);
    const auto result = process_uploads(pool, {{"a", 1000, 500}, {"b", 0, 0}, {"c", 300, 300}}, {100, 200});
    CHECK_EQ(result.thumbnails.size(), 4u);
    CHECK_EQ(result.thumbnails[0].name, "a@100");
    CHECK_EQ(result.thumbnails[3].name, "c@200");
    CHECK_EQ(result.errors.size(), 2u);
    CHECK_EQ(result.errors[0], "b: empty image");
    std::istringstream in("beach 4000 2000\nnull 0 0\n\n");
    std::ostringstream out;
    CHECK_EQ(run(in, out), 0);
    CHECK(out.str().find("beach@256 256x128") != std::string::npos);
    CHECK(out.str().find("2 thumbnail(s), 2 error(s) on 4 worker thread(s)") != std::string::npos);
}
