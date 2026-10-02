/**
 * @file
 * Day 74 – Concurrency: Futures & Thread Pools.
 *
 * Scenario: the *thumbnail service* of a photo-sharing site. Every upload needs several
 * thumbnails and a checksum; the work is independent per image, so it runs in parallel. Results
 * and errors travel back through futures, and a fixed-size thread pool keeps a burst of uploads
 * from starting hundreds of threads.
 *
 * Deliverables (syllabus):
 * - std::async
 * - Promises and futures
 * - Exception propagation
 * - A fixed-size thread pool
 */
#pragma once

#include <algorithm>
#include <condition_variable>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <future>
#include <istream>
#include <memory>
#include <mutex>
#include <ostream>
#include <queue>
#include <sstream>
#include <stdexcept>
#include <string>
#include <thread>
#include <type_traits>
#include <utility>
#include <vector>

#include "cppm/lesson.hpp"

namespace cppm::day74 {

inline constexpr Deliverable DELIVERABLES[] = {
    {"running independent work with std::async", "checksums_async"},
    {"handing a result from one thread to another with a promise", "fetch_metadata"},
    {"exceptions that travel through future::get", "make_thumbnail"},
    {"a fixed-size thread pool that returns futures", "ThreadPool::submit"},
    {"collecting successes and failures from many tasks", "process_uploads"},
};

/// Adler-32: the checksum stored with each upload.
inline std::uint32_t adler32(const std::string& data) {
    std::uint32_t a = 1;
    std::uint32_t b = 0;
    for (const char c : data) {
        a = (a + static_cast<unsigned char>(c)) % 65521;
        b = (b + a) % 65521;
    }
    return (b << 16) | a;
}

/// One std::async per file. launch::async forces a new thread; the futures are collected first
/// and only then waited on – calling get() inside the loop would make it sequential.
inline std::vector<std::uint32_t> checksums_async(const std::vector<std::string>& files) {
    std::vector<std::future<std::uint32_t>> pending;
    for (const auto& file : files) pending.push_back(std::async(std::launch::async, adler32, std::cref(file)));
    std::vector<std::uint32_t> sums;
    for (auto& f : pending) sums.push_back(f.get());
    return sums;
}

struct Image {
    std::string name;
    int width;
    int height;
};

/// A thumbnail at most @p max_side on its longer side, keeping the aspect ratio.
/// Throws for unusable images; the exception reaches whoever calls future::get().
inline Image make_thumbnail(const Image& image, int max_side) {
    if (image.width <= 0 || image.height <= 0) throw std::invalid_argument(image.name + ": empty image");
    if (max_side <= 0) throw std::invalid_argument("max side must be positive");
    const int longer = std::max(image.width, image.height);
    if (longer <= max_side) return {image.name + "@" + std::to_string(max_side), image.width, image.height};
    const auto scale = [&](int side) {
        return std::max(1, static_cast<int>((static_cast<long long>(side) * max_side + longer / 2) / longer));
    };
    return {image.name + "@" + std::to_string(max_side), scale(image.width), scale(image.height)};
}

/// A promise is the writing end of a future. A worker thread reads "metadata" and fulfils the
/// promise with a value – or with the exception it hit.
inline std::future<std::string> fetch_metadata(const Image& image) {
    std::promise<std::string> promise;
    std::future<std::string> result = promise.get_future();
    std::thread([image, promise = std::move(promise)]() mutable {
        try {
            if (image.name.empty()) throw std::runtime_error("image has no name");
            promise.set_value(image.name + " " + std::to_string(image.width) + "x" + std::to_string(image.height));
        } catch (...) {
            promise.set_exception(std::current_exception());  // the caller's get() rethrows it
        }
    }).detach();  // safe: the thread owns everything it touches
    return result;
}

/// N worker threads take tasks from one queue. submit() wraps any callable in a packaged_task
/// and returns its future; the destructor finishes queued work and joins every worker.
class ThreadPool {
  public:
    explicit ThreadPool(std::size_t threads) {
        if (threads == 0) throw std::invalid_argument("a pool needs at least one thread");
        for (std::size_t i = 0; i < threads; ++i) workers_.emplace_back([this] { work(); });
    }
    ThreadPool(const ThreadPool&) = delete;
    ThreadPool& operator=(const ThreadPool&) = delete;
    ~ThreadPool() {
        {
            const std::lock_guard lock(mutex_);
            stopping_ = true;
        }
        wake_.notify_all();
        for (auto& worker : workers_) worker.join();
    }

    template <typename F>
    auto submit(F function) -> std::future<std::invoke_result_t<F>> {
        using Result = std::invoke_result_t<F>;
        // packaged_task is move-only but std::function needs copyable callables, hence shared_ptr.
        auto task = std::make_shared<std::packaged_task<Result()>>(std::move(function));
        auto future = task->get_future();
        {
            const std::lock_guard lock(mutex_);
            if (stopping_) throw std::runtime_error("pool is shutting down");
            tasks_.emplace([task] { (*task)(); });
        }
        wake_.notify_one();
        return future;
    }
    std::size_t size() const { return workers_.size(); }
    std::size_t completed() const {
        const std::lock_guard lock(mutex_);
        return completed_;
    }

  private:
    void work() {
        while (true) {
            std::function<void()> task;
            {
                std::unique_lock lock(mutex_);
                wake_.wait(lock, [this] { return stopping_ || !tasks_.empty(); });
                if (tasks_.empty()) return;  // stopping and nothing left
                task = std::move(tasks_.front());
                tasks_.pop();
            }
            task();  // run outside the lock; a packaged_task stores exceptions, it never throws here
            const std::lock_guard lock(mutex_);
            ++completed_;
        }
    }

    mutable std::mutex mutex_;
    std::condition_variable wake_;
    std::queue<std::function<void()>> tasks_;
    std::vector<std::thread> workers_;
    std::size_t completed_ = 0;
    bool stopping_ = false;
};

struct UploadResult {
    std::vector<Image> thumbnails;
    std::vector<std::string> errors;
};

/// Submit one task per (image, size) pair, then collect them in submission order.
inline UploadResult process_uploads(ThreadPool& pool, const std::vector<Image>& images, const std::vector<int>& sizes) {
    std::vector<std::future<Image>> futures;
    for (const auto& image : images) {
        for (const int size : sizes)
            futures.push_back(pool.submit([image, size] { return make_thumbnail(image, size); }));
    }
    UploadResult result;
    for (auto& f : futures) {
        try {
            result.thumbnails.push_back(f.get());
        } catch (const std::exception& error) {
            result.errors.emplace_back(error.what());
        }
    }
    return result;
}

/// The interactive demo: "name width height" per upload; an empty line processes the batch.
inline int run(std::istream& in, std::ostream& out) {
    out << "Day 74 – Concurrency: Futures & Thread Pools\n";
    std::vector<Image> uploads;
    while (auto line = prompt_line(in, out, "name width height> ")) {
        std::istringstream words(*line);
        Image image{};
        if (!(words >> image.name >> image.width >> image.height)) break;
        uploads.push_back(image);
    }
    ThreadPool pool(4);
    const auto result = process_uploads(pool, uploads, {64, 256});
    for (const auto& t : result.thumbnails) out << "  " << t.name << ' ' << t.width << 'x' << t.height << '\n';
    for (const auto& e : result.errors) out << "  error: " << e << '\n';
    out << result.thumbnails.size() << " thumbnail(s), " << result.errors.size() << " error(s) on " << pool.size()
        << " worker thread(s)\n";
    return 0;
}

}  // namespace cppm::day74
