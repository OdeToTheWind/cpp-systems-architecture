// Tests for Day 40 – Iterators.
#include <algorithm>
#include <forward_list>
#include <iterator>
#include <list>
#include <numeric>
#include <sstream>
#include <vector>

#include "cppm/testing.hpp"
#include "day_40_iterators/lesson.hpp"

using namespace cppm::day40;

TEST_CASE("category_of names each standard container's iterator") {
    CHECK_EQ(category_of<std::vector<int>::iterator>(), "contiguous");
    CHECK_EQ(category_of<std::list<int>::iterator>(), "bidirectional");
    CHECK_EQ(category_of<std::forward_list<int>::iterator>(), "forward");
    CHECK_EQ(category_of<std::istream_iterator<int>>(), "input");
    CHECK_EQ((category_of<RingBuffer<int, 2>::Iterator>()), "forward");  // extra () protect the comma from the macro
}

TEST_CASE("the ring buffer keeps only the newest items, oldest first") {
    RingBuffer<int, 3> buffer;
    CHECK(buffer.begin() == buffer.end());
    for (int i = 1; i <= 5; ++i) {
        buffer.push(i);
    }
    CHECK_EQ(buffer.size(), 3u);
    const std::vector<int> contents(buffer.begin(), buffer.end());
    CHECK(contents == std::vector<int>{3, 4, 5});
}

TEST_CASE("the custom iterator works with range-for and algorithms") {
    RingBuffer<int, 4> buffer;
    for (int i : {5, 1, 4}) {
        buffer.push(i);
    }
    int sum = 0;
    for (int value : buffer) {
        sum += value;
    }
    CHECK_EQ(sum, 10);
    CHECK_EQ(std::accumulate(buffer.begin(), buffer.end(), 0), 10);
    CHECK_EQ(*std::min_element(buffer.begin(), buffer.end()), 1);
    CHECK_EQ(std::distance(buffer.begin(), buffer.end()), 3);
    auto it = buffer.begin();
    CHECK_EQ(*it++, 5);
    CHECK_EQ(*it, 1);
}

TEST_CASE("erasing from a list while iterating uses erase's return value") {
    std::list<Track> library{{"intro", 20}, {"song", 200}, {"jingle", 15}, {"ballad", 300}, {"sting", 5}};
    CHECK_EQ(remove_short_tracks(library, 30), 3);
    REQUIRE_EQ(library.size(), 2u);
    CHECK_EQ(library.front().title, "song");
    CHECK_EQ(library.back().title, "ballad");
}

TEST_CASE("erase_if removes from a vector in one pass") {
    std::vector<Track> library{{"a", 10}, {"b", 100}, {"c", 10}};
    CHECK_EQ(remove_short_tracks(library, 30), 2u);
    CHECK(library == std::vector<Track>{{"b", 100}});
}

TEST_CASE("longest_recent_track only considers what the board still holds") {
    RingBuffer<Track, 2> recent;
    CHECK_EQ(longest_recent_track(recent), "");
    recent.push({"epic", 900});
    recent.push({"short", 60});
    recent.push({"medium", 240});
    CHECK_EQ(longest_recent_track(recent), "medium");
}

TEST_CASE("run shows the board after every play") {
    std::istringstream in("a 100\nb 300\nc 200\nd 50\nstop\n");
    std::ostringstream out;
    CHECK_EQ(run(in, out), 0);
    const auto text = out.str();
    CHECK(text.find("vector: contiguous, list: bidirectional") != std::string::npos);
    CHECK(text.find("board: b c d") != std::string::npos);
    CHECK(text.find("Longest recent track: b") != std::string::npos);
}
