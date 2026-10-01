// textkit/stats.hpp – the COMPILED part of textkit: declarations only; the code lives in stats.cpp,
// which is built into the static library target `textkit` and must be linked.
#pragma once

#include <cstddef>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace textkit {

/// Counts for one piece of text.
struct Stats {
    std::size_t characters;
    std::size_t words;
    std::size_t sentences;
    double average_word_length;
};

/// Character, word and sentence counts.
Stats analyse(std::string_view text);

/// The @p n most frequent words (lower-cased), most frequent first, ties alphabetical.
std::vector<std::pair<std::string, std::size_t>> top_words(std::string_view text, std::size_t n);

}  // namespace textkit
