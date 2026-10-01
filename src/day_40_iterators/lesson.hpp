/**
 * @file
 * Day 40 – Iterators.
 *
 * Scenario: a *radio station's "recently played" board*. The last N tracks live in a
 * fixed-size ring buffer with its own iterator, so the board works with range-based for and
 * with every standard algorithm; the station's library is cleaned with erase loops that never
 * use an invalidated iterator.
 *
 * Deliverables (syllabus):
 * - Iterator categories
 * - begin and end
 * - Writing a custom iterator
 * - Iterator invalidation
 */
#pragma once

#include <algorithm>
#include <array>
#include <cstddef>
#include <forward_list>
#include <istream>
#include <iterator>
#include <list>
#include <ostream>
#include <sstream>
#include <string>
#include <string_view>
#include <vector>

#include "cppm/lesson.hpp"

namespace cppm::day40 {

inline constexpr Deliverable DELIVERABLES[] = {
    {"naming the iterator category of any container", "category_of"},
    {"a container with begin() and end()", "RingBuffer"},
    {"a custom forward iterator", "RingBuffer::Iterator"},
    {"erasing while iterating without invalid iterators", "remove_short_tracks"},
    {"algorithms working through iterators", "longest_recent_track"},
};

/// The strongest iterator category that @p Iterator models (C++20 concepts).
template <typename Iterator>
std::string_view category_of() {
    if constexpr (std::contiguous_iterator<Iterator>) return "contiguous";
    else if constexpr (std::random_access_iterator<Iterator>) return "random access";
    else if constexpr (std::bidirectional_iterator<Iterator>) return "bidirectional";
    else if constexpr (std::forward_iterator<Iterator>) return "forward";
    else if constexpr (std::input_iterator<Iterator>) return "input";
    else return "output or none";
}

struct Track {
    std::string title;
    int seconds;
    bool operator==(const Track&) const = default;
};

/// Keeps the last Capacity items; pushing into a full buffer overwrites the oldest.
template <typename T, std::size_t Capacity>
class RingBuffer {
public:
    static_assert(Capacity > 0);

    /// Walks from the oldest to the newest element.
    class Iterator {
    public:
        using iterator_category = std::forward_iterator_tag;
        using value_type = T;
        using difference_type = std::ptrdiff_t;
        using pointer = const T*;
        using reference = const T&;

        Iterator() = default;
        Iterator(const RingBuffer* buffer, std::size_t position) : buffer_(buffer), position_(position) {}

        reference operator*() const { return buffer_->at_logical(position_); }
        pointer operator->() const { return &**this; }
        Iterator& operator++() {
            ++position_;
            return *this;
        }
        Iterator operator++(int) {
            Iterator before = *this;
            ++*this;
            return before;
        }
        bool operator==(const Iterator&) const = default;

    private:
        const RingBuffer* buffer_{nullptr};
        std::size_t position_{0};  // 0 = oldest element
    };

    void push(T value) {
        items_[(start_ + size_) % Capacity] = std::move(value);
        if (size_ < Capacity) {
            ++size_;
        } else {
            start_ = (start_ + 1) % Capacity;  // overwrite the oldest
        }
    }
    std::size_t size() const { return size_; }
    Iterator begin() const { return Iterator(this, 0); }
    Iterator end() const { return Iterator(this, size_); }
    const T& at_logical(std::size_t position) const { return items_[(start_ + position) % Capacity]; }

private:
    std::array<T, Capacity> items_{};
    std::size_t start_{0};
    std::size_t size_{0};
};

static_assert(std::forward_iterator<RingBuffer<int, 3>::Iterator>, "the custom iterator models forward_iterator");

/// Remove tracks shorter than @p min_seconds from a list *while iterating it*.
/// `erase` invalidates the erased iterator but returns the next valid one – use that, never `++it` afterwards.
inline int remove_short_tracks(std::list<Track>& library, int min_seconds) {
    int removed = 0;
    for (auto it = library.begin(); it != library.end();) {
        if (it->seconds < min_seconds) {
            it = library.erase(it);
            ++removed;
        } else {
            ++it;
        }
    }
    return removed;
}

/// The same job for a vector, where erase in a loop is O(n²): C++20 std::erase_if does it in one pass.
inline std::size_t remove_short_tracks(std::vector<Track>& library, int min_seconds) {
    return std::erase_if(library, [min_seconds](const Track& t) { return t.seconds < min_seconds; });
}

/// Standard algorithms accept the custom iterator like any other.
template <std::size_t N>
std::string longest_recent_track(const RingBuffer<Track, N>& recent) {
    const auto it = std::max_element(recent.begin(), recent.end(),
                                     [](const Track& a, const Track& b) { return a.seconds < b.seconds; });
    return it == recent.end() ? "" : it->title;
}

/// The interactive demo: "title seconds" lines played into a board that keeps the last 3.
inline int run(std::istream& in, std::ostream& out) {
    out << "Day 40 – Iterators\n"
        << "vector: " << category_of<std::vector<int>::iterator>() << ", list: " << category_of<std::list<int>::iterator>()
        << ", forward_list: " << category_of<std::forward_list<int>::iterator>()
        << ", istream: " << category_of<std::istream_iterator<int>>() << '\n';
    RingBuffer<Track, 3> recent;
    while (auto line = prompt_line(in, out, "title seconds> ")) {
        std::istringstream words(*line);
        Track track;
        if (!(words >> track.title >> track.seconds)) {
            break;
        }
        recent.push(track);
        out << "  board:";
        for (const Track& t : recent) {  // range-for uses begin()/end()
            out << ' ' << t.title;
        }
        out << '\n';
    }
    out << "Longest recent track: " << longest_recent_track(recent) << '\n';
    return 0;
}

}  // namespace cppm::day40
