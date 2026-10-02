// Tests for Day 68 – Move Semantics. The global counters prove when a copy or a move happened.
#include <sstream>
#include <string>
#include <utility>

#include "cppm/testing.hpp"
#include "day_68_move_semantics/lesson.hpp"

using namespace cppm::day68;

TEST_CASE("overloads reveal value categories") {
    std::string name = "intro";
    CHECK_EQ(category(name), "lvalue");
    CHECK_EQ(category(std::string("outro")), "rvalue");
    CHECK_EQ(category(std::move(name)), "rvalue");  // std::move is only a cast
    std::string&& ref = std::string("x");
    CHECK_EQ(category(ref), "lvalue");  // a named rvalue reference is itself an lvalue
}

TEST_CASE("copies are deep and independent") {
    stats = {};
    AudioBuffer a(4, 7);
    AudioBuffer b = a;
    b[0] = 1;
    CHECK_EQ(a[0], 7);
    CHECK_EQ(stats.copies, 1);
    CHECK_EQ(stats.allocations, 2);
    a = b;
    CHECK_EQ(a[0], 1);
    AudioBuffer& alias = a;
    a = alias;  // self-assignment through an alias is safe
    CHECK_EQ(a.size(), 4u);
}

TEST_CASE("moves steal the buffer and leave the source empty") {
    stats = {};
    AudioBuffer a(1000, 3);
    AudioBuffer b = std::move(a);
    CHECK(a.empty());
    CHECK_EQ(b.size(), 1000u);
    CHECK_EQ(stats.allocations, 1);
    CHECK_EQ(stats.copies, 0);
    AudioBuffer c(5);
    c = std::move(b);
    CHECK_EQ(c[999], 3);
    CHECK_EQ(stats.moves, 2);
}

TEST_CASE("std::move on a const object silently copies") {
    stats = {};
    const AudioBuffer frozen(10);
    AudioBuffer target = std::move(frozen);  // const&& binds to the copy constructor
    CHECK_EQ(stats.copies, 1);
    CHECK_EQ(frozen.size(), 10u);
    CHECK_EQ(target.size(), 10u);
}

TEST_CASE("sink parameters copy lvalues once and rvalues never") {
    Track track;
    AudioBuffer take(50);
    stats = {};
    track.set_samples(take);
    CHECK_EQ(stats.copies, 1);
    stats = {};
    track.set_samples(AudioBuffer(60));
    CHECK_EQ(stats.copies, 0);
    CHECK_EQ(track.samples().size(), 60u);
    const AudioBuffer released = track.release();
    CHECK(track.samples().empty());
    CHECK_EQ(released.size(), 60u);
}

TEST_CASE("returning by value and growing vectors never copy") {
    AudioBuffer a(3, 10);
    AudioBuffer b(5, 20);
    stats = {};
    const AudioBuffer mixed = mixdown(a, b);
    CHECK_EQ(stats.copies, 0);
    CHECK_EQ(mixed[0], 15);
    CHECK_EQ(mixed[4], 10);
    stats = {};
    const auto library = grow_library(20, 8);
    CHECK_EQ(library.size(), 20u);
    CHECK_EQ(stats.copies, 0);  // reallocations moved, thanks to noexcept
    CHECK(stats.moves > 0);
    CHECK_EQ(library[19][0], 19);
}

TEST_CASE("run shows copies only where an lvalue is passed") {
    std::istringstream in("record 100\ncopy\nmove\nmix\nstop\n");
    std::ostringstream out;
    CHECK_EQ(run(in, out), 0);
    const auto text = out.str();
    CHECK(text.find("narration 100, music 100 | alloc 2, copies 1") != std::string::npos);
    CHECK(text.find("narration 0, music 100 | alloc 2, copies 1") != std::string::npos);
    CHECK(text.find("mixed 100 sample(s)") != std::string::npos);
}
