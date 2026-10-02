// Tests for Day 75 – Coroutines.
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

#include "cppm/testing.hpp"
#include "day_75_coroutines/lesson.hpp"

using namespace cppm::day75;

namespace {
Generator<int> countdown(int from) {
    while (from > 0) co_yield from--;
}
Generator<int> failing() {
    co_yield 1;
    throw std::runtime_error("script error");
}
int started = 0;
Generator<int> lazy() {
    ++started;
    co_yield 42;
}
}  // namespace

TEST_CASE("a generator yields values one at a time in a range-for") {
    std::vector<int> seen;
    for (int v : countdown(3)) seen.push_back(v);
    CHECK(seen == std::vector<int>{3, 2, 1});
    CHECK(take(countdown(0), 5).empty());
}

TEST_CASE("generators start lazily") {
    started = 0;
    auto gen = lazy();
    CHECK_EQ(started, 0);  // initial_suspend: nothing ran yet
    CHECK(take(std::move(gen), 1) == std::vector<int>{42});
    CHECK_EQ(started, 1);
}

TEST_CASE("an infinite sequence is fine when only a prefix is taken") {
    CHECK(take(entity_ids("npc", 7), 3) == std::vector<std::string>{"npc-7", "npc-8", "npc-9"});
}

TEST_CASE("lazy pipelines read only as much input as needed") {
    int read = 0;
    const std::string script = "# intro\nHello\n\nHow are you?\nBye\n";
    CHECK(take(dialogue_lines(script, &read), 2) == std::vector<std::string>{"Hello", "How are you?"});
    CHECK_EQ(read, 4);  // the last line was never read
    CHECK_EQ(take(dialogue_lines(script), 10).size(), 3u);
}

TEST_CASE("exceptions inside a generator reach the consumer") {
    std::vector<int> seen;
    CHECK_THROWS_AS(
        [&] {
            for (int v : failing()) seen.push_back(v);
        }(),
        std::runtime_error);
    CHECK(seen == std::vector<int>{1});
}

TEST_CASE("co_await pauses a scene until its event is posted") {
    Director director;
    Scene scene = tavern_scene(director, "Ash");
    CHECK(director.script().empty());  // not started
    scene.start();
    CHECK_EQ(director.script().size(), 1u);
    CHECK_EQ(director.post("pay"), 0u);  // wrong event: nobody waiting
    CHECK_EQ(director.post("order"), 1u);
    CHECK(!scene.finished());
    CHECK_EQ(director.post("pay"), 1u);
    CHECK(scene.finished());
    CHECK_EQ(director.script().back(), "Barkeep: Much obliged. Mind the stairs on your way out.");
    scene.rethrow_if_failed();
}

TEST_CASE("several scenes wait independently and run drives one") {
    Director director;
    Scene a = tavern_scene(director, "A");
    Scene b = tavern_scene(director, "B");
    a.start();
    b.start();
    CHECK_EQ(director.post("order"), 2u);
    CHECK_EQ(director.script().size(), 4u);
    std::istringstream in("dance\norder\npay\n");
    std::ostringstream out;
    CHECK_EQ(run(in, out), 0);
    CHECK(out.str().find("spawned npc-1003") != std::string::npos);
    CHECK(out.str().find("nobody is waiting for 'dance'") != std::string::npos);
    CHECK(out.str().find("scene complete") != std::string::npos);
}
