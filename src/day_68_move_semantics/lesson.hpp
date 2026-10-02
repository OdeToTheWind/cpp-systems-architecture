/**
 * @file
 * Day 68 – Move Semantics.
 *
 * Scenario: the sample buffers of a *podcast audio editor*. An hour of stereo audio is hundreds
 * of megabytes, so whether a buffer is copied or moved decides whether an edit is instant or
 * slow. The buffer manages its own memory with the rule of five and counts every allocation,
 * copy and move, so each claim about move semantics can be checked by a test.
 *
 * Deliverables (syllabus):
 * - Lvalues and rvalues
 * - Move constructors and move assignment
 * - The rule of five
 * - std::move vs copy
 */
#pragma once

#include <algorithm>
#include <cstddef>
#include <istream>
#include <ostream>
#include <sstream>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>
#include <vector>

#include "cppm/lesson.hpp"

namespace cppm::day68 {

inline constexpr Deliverable DELIVERABLES[] = {
    {"telling lvalues from rvalues with overloads", "category"},
    {"a buffer that follows the rule of five", "AudioBuffer"},
    {"a sink parameter taken by value and moved in", "Track::set_samples"},
    {"returning large objects by value", "mixdown"},
    {"noexcept moves letting a vector move instead of copy", "grow_library"},
};

/// What happened to buffers since the last reset – the evidence for every test.
struct Stats {
    int allocations = 0;
    int copies = 0;
    int moves = 0;
};
inline Stats stats;

/// Overloads that report the value category of their argument.
inline std::string_view category(const std::string&) { return "lvalue"; }
inline std::string_view category(std::string&&) { return "rvalue"; }

/// A heap buffer of 16-bit samples, managed by hand to show what the rule of five means.
class AudioBuffer {
public:
    AudioBuffer() = default;
    explicit AudioBuffer(std::size_t size, short fill = 0) : size_(size), data_(size ? new short[size] : nullptr) {
        std::fill_n(data_, size_, fill);
        if (size_) ++stats.allocations;
    }
    ~AudioBuffer() { delete[] data_; }  // 1. destructor

    AudioBuffer(const AudioBuffer& other) : AudioBuffer(other.size_) {  // 2. copy constructor: deep copy
        std::copy_n(other.data_, size_, data_);
        ++stats.copies;
    }
    AudioBuffer& operator=(const AudioBuffer& other) {  // 3. copy assignment: copy-and-swap
        if (this != &other) {
            AudioBuffer copy(other);
            swap(copy);
        }
        return *this;
    }
    AudioBuffer(AudioBuffer&& other) noexcept  // 4. move constructor: steal the pointer
        : size_(std::exchange(other.size_, 0)), data_(std::exchange(other.data_, nullptr)) {
        ++stats.moves;
    }
    AudioBuffer& operator=(AudioBuffer&& other) noexcept {  // 5. move assignment
        if (this != &other) {
            delete[] data_;
            size_ = std::exchange(other.size_, 0);
            data_ = std::exchange(other.data_, nullptr);
            ++stats.moves;
        }
        return *this;
    }

    void swap(AudioBuffer& other) noexcept {
        std::swap(size_, other.size_);
        std::swap(data_, other.data_);
    }
    std::size_t size() const { return size_; }
    bool empty() const { return size_ == 0; }  // a moved-from buffer is valid and empty
    short& operator[](std::size_t i) { return data_[i]; }
    short operator[](std::size_t i) const { return data_[i]; }

private:
    std::size_t size_ = 0;
    short* data_ = nullptr;
};

class Track {
public:
    /// Sink parameter: take by value, then move into place. Callers that pass an rvalue pay
    /// for no copy at all; callers that pass an lvalue pay for exactly one.
    void set_samples(AudioBuffer samples) { samples_ = std::move(samples); }
    const AudioBuffer& samples() const { return samples_; }
    /// Hand the samples to the caller, leaving the track empty.
    AudioBuffer release() { return std::exchange(samples_, AudioBuffer{}); }

private:
    AudioBuffer samples_;
};

/// Average two tracks into a new buffer. Returned by value: the local is moved or elided, never copied.
inline AudioBuffer mixdown(const AudioBuffer& a, const AudioBuffer& b) {
    AudioBuffer out(std::max(a.size(), b.size()));
    for (std::size_t i = 0; i < out.size(); ++i) {
        const int left = i < a.size() ? a[i] : 0;
        const int right = i < b.size() ? b[i] : 0;
        out[i] = static_cast<short>((left + right) / 2);
    }
    return out;
}

/// Push @p count buffers into a vector without reserving, forcing several reallocations.
/// Because the move constructor is noexcept, the vector moves old elements instead of copying.
inline std::vector<AudioBuffer> grow_library(int count, std::size_t samples) {
    std::vector<AudioBuffer> library;
    for (int i = 0; i < count; ++i) library.emplace_back(samples, static_cast<short>(i));
    return library;
}

static_assert(std::is_nothrow_move_constructible_v<AudioBuffer>);

/// The interactive demo: commands "record <n>", "copy", "move", "mix" show the counters.
inline int run(std::istream& in, std::ostream& out) {
    out << "Day 68 – Move Semantics\n";
    stats = {};
    Track narration;
    Track music;
    while (auto line = prompt_line(in, out, "record <n> | copy | move | mix> ")) {
        std::istringstream words(*line);
        std::string command;
        if (!(words >> command)) break;
        if (command == "record") {
            std::size_t n = 0;
            words >> n;
            narration.set_samples(AudioBuffer(n, 100));  // a temporary: moved in
        } else if (command == "copy") {
            music.set_samples(narration.samples());  // an lvalue: copied once
        } else if (command == "move") {
            music.set_samples(narration.release());
        } else if (command == "mix") {
            const AudioBuffer mixed = mixdown(narration.samples(), music.samples());
            out << "  mixed " << mixed.size() << " sample(s)\n";
        }
        out << "  narration " << narration.samples().size() << ", music " << music.samples().size() << " | alloc "
            << stats.allocations << ", copies " << stats.copies << ", moves " << stats.moves << '\n';
    }
    return 0;
}

}  // namespace cppm::day68
