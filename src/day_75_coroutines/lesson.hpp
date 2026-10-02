/**
 * @file
 * Day 75 – Coroutines.
 *
 * Scenario: the scripting layer of a *story-driven game*. Level designers want to write a
 * cutscene top to bottom – "say this, wait until the player orders, say that" – instead of
 * splitting it into callbacks. Coroutines make that possible: generators produce lazy
 * sequences (dialogue lines, an endless ID stream) and awaitable scenes pause until a game
 * event resumes them.
 *
 * Deliverables (syllabus):
 * - co_yield generators
 * - co_await basics
 * - Promise types
 * - Lazy sequences
 */
#pragma once

#include <coroutine>
#include <cstddef>
#include <exception>
#include <istream>
#include <iterator>
#include <map>
#include <optional>
#include <ostream>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

#include "cppm/lesson.hpp"

namespace cppm::day75 {

inline constexpr Deliverable DELIVERABLES[] = {
    {"a generator type with its promise_type", "Generator"},
    {"an infinite lazy sequence with co_yield", "entity_ids"},
    {"a lazy pipeline that reads only what it needs", "dialogue_lines"},
    {"an awaitable that suspends until an event", "Director::wait_for"},
    {"a scene coroutine written top to bottom", "tavern_scene"},
};

/// A minimal std::generator (C++23) stand-in: a coroutine that co_yields values of type T and
/// can be used in a range-for. Each ++ resumes the coroutine until its next co_yield.
template <typename T>
class Generator {
public:
    struct promise_type {
        std::optional<T> current;
        std::exception_ptr error;
        Generator get_return_object() { return Generator(std::coroutine_handle<promise_type>::from_promise(*this)); }
        std::suspend_always initial_suspend() noexcept { return {}; }  // lazy: nothing runs until first use
        std::suspend_always final_suspend() noexcept { return {}; }
        std::suspend_always yield_value(T value) {
            current = std::move(value);
            return {};
        }
        void return_void() {}
        void unhandled_exception() { error = std::current_exception(); }
    };

    class iterator {
    public:
        using value_type = T;
        using difference_type = std::ptrdiff_t;
        iterator() = default;
        explicit iterator(std::coroutine_handle<promise_type> h) : handle_(h) { advance(); }
        const T& operator*() const { return *handle_.promise().current; }
        iterator& operator++() {
            advance();
            return *this;
        }
        void operator++(int) { advance(); }
        bool operator==(std::default_sentinel_t) const { return !handle_ || handle_.done(); }

    private:
        void advance() {
            handle_.resume();
            if (handle_.done() && handle_.promise().error) std::rethrow_exception(handle_.promise().error);
        }
        std::coroutine_handle<promise_type> handle_;
    };

    Generator(Generator&& other) noexcept : handle_(std::exchange(other.handle_, {})) {}
    Generator& operator=(Generator&& other) noexcept {
        if (this != &other) {
            if (handle_) handle_.destroy();
            handle_ = std::exchange(other.handle_, {});
        }
        return *this;
    }
    Generator(const Generator&) = delete;
    Generator& operator=(const Generator&) = delete;
    ~Generator() {
        if (handle_) handle_.destroy();  // destroys the frame even if it never finished
    }

    iterator begin() { return iterator(handle_); }
    std::default_sentinel_t end() { return {}; }

private:
    explicit Generator(std::coroutine_handle<promise_type> h) : handle_(h) {}
    std::coroutine_handle<promise_type> handle_;
};

/// The first @p n values of any generator (which may be infinite).
template <typename T>
std::vector<T> take(Generator<T> gen, std::size_t n) {
    std::vector<T> out;
    if (n == 0) return out;
    for (const T& value : gen) {
        out.push_back(value);
        if (out.size() == n) break;
    }
    return out;
}

/// An endless stream of IDs such as "npc-1001", "npc-1002", … – only computed when asked for.
inline Generator<std::string> entity_ids(std::string prefix, int first) {
    for (int id = first;; ++id) co_yield prefix + "-" + std::to_string(id);
}

/// Lines of a dialogue script, skipping blanks and "#" comments; @p counter records how many
/// raw lines were actually read, to show the laziness. Takes the script by value: a coroutine
/// that held a reference could outlive the string it refers to.
inline Generator<std::string> dialogue_lines(std::string script, int* counter = nullptr) {
    std::istringstream in(script);
    for (std::string line; std::getline(in, line);) {
        if (counter) ++*counter;
        if (line.empty() || line[0] == '#') continue;
        co_yield line;
    }
}

class Director;

/// The coroutine type of a scene. It starts suspended; start() runs it to its first co_await.
class Scene {
public:
    struct promise_type {
        std::exception_ptr error;
        Scene get_return_object() { return Scene(std::coroutine_handle<promise_type>::from_promise(*this)); }
        std::suspend_always initial_suspend() noexcept { return {}; }
        std::suspend_always final_suspend() noexcept { return {}; }
        void return_void() {}
        void unhandled_exception() { error = std::current_exception(); }
    };
    Scene(Scene&& other) noexcept : handle_(std::exchange(other.handle_, {})) {}
    Scene(const Scene&) = delete;
    Scene& operator=(const Scene&) = delete;
    Scene& operator=(Scene&&) = delete;
    ~Scene() {
        if (handle_) handle_.destroy();
    }
    void start() { handle_.resume(); }
    bool finished() const { return handle_.done(); }
    void rethrow_if_failed() const {
        if (handle_.done() && handle_.promise().error) std::rethrow_exception(handle_.promise().error);
    }

private:
    explicit Scene(std::coroutine_handle<promise_type> h) : handle_(h) {}
    std::coroutine_handle<promise_type> handle_;
};

/// Runs scenes: keeps the script output and resumes scenes waiting for a posted event.
class Director {
public:
    /// The awaitable returned by wait_for. co_await calls await_ready (false: always pause),
    /// then await_suspend with the paused coroutine, which the director parks under the event.
    struct EventAwaiter {
        Director& director;
        std::string event;
        bool await_ready() const noexcept { return false; }
        void await_suspend(std::coroutine_handle<> paused) { director.waiting_[event].push_back(paused); }
        void await_resume() const noexcept {}
    };

    EventAwaiter wait_for(std::string event) { return EventAwaiter{*this, std::move(event)}; }
    void say(const std::string& line) { script_.push_back(line); }

    /// Resume every scene waiting for @p event; returns how many were resumed.
    std::size_t post(const std::string& event) {
        auto it = waiting_.find(event);
        if (it == waiting_.end()) return 0;
        auto ready = std::move(it->second);
        waiting_.erase(it);  // erase first: a resumed scene may wait for the same event again
        for (auto handle : ready) handle.resume();
        return ready.size();
    }
    const std::vector<std::string>& script() const { return script_; }

private:
    std::map<std::string, std::vector<std::coroutine_handle<>>> waiting_;
    std::vector<std::string> script_;
};

/// A cutscene as straight-line code. It pauses at each co_await until the event is posted.
inline Scene tavern_scene(Director& director, std::string player) {
    director.say("Barkeep: Welcome, " + player + ". What'll it be?");
    co_await director.wait_for("order");
    director.say("Barkeep: One mug of cider, coming up. That's 3 silver.");
    co_await director.wait_for("pay");
    director.say("Barkeep: Much obliged. Mind the stairs on your way out.");
}

/// The interactive demo: type events (order, pay, …) to drive the scene.
inline int run(std::istream& in, std::ostream& out) {
    out << "Day 75 – Coroutines\n";
    for (const auto& id : take(entity_ids("npc", 1001), 3)) out << "spawned " << id << '\n';
    Director director;
    Scene scene = tavern_scene(director, "Robin");
    scene.start();
    std::size_t shown = 0;
    auto flush = [&] {
        for (; shown < director.script().size(); ++shown) out << "  " << director.script()[shown] << '\n';
    };
    flush();
    while (!scene.finished()) {
        auto line = prompt_line(in, out, "event> ");
        if (!line) break;
        if (director.post(*line) == 0) out << "  (nobody is waiting for '" << *line << "')\n";
        flush();
    }
    out << (scene.finished() ? "scene complete\n" : "scene left unfinished\n");
    return 0;
}

}  // namespace cppm::day75
