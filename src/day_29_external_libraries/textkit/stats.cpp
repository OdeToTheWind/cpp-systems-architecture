// textkit/stats.cpp – compiled once into libtextkit.a (textkit.lib on Windows).
#include "textkit/stats.hpp"

#include <algorithm>
#include <cctype>
#include <map>
#include <string>

#include "textkit/version.hpp"

namespace textkit {
namespace {

std::vector<std::string> words_of(std::string_view text) {
    std::vector<std::string> words;
    std::string current;
    for (const char c : text) {
        const auto u = static_cast<unsigned char>(c);
        if (std::isalnum(u) || c == '\'') {
            current += static_cast<char>(std::tolower(u));
        } else if (!current.empty()) {
            words.push_back(current);
            current.clear();
        }
    }
    if (!current.empty()) {
        words.push_back(current);
    }
    return words;
}

}  // namespace

const char* version() {
    return "1.4.2";
}

Stats analyse(std::string_view text) {
    const auto words = words_of(text);
    std::size_t letters = 0;
    for (const auto& word : words) {
        letters += word.size();
    }
    const auto sentences = static_cast<std::size_t>(
        std::count_if(text.begin(), text.end(), [](char c) { return c == '.' || c == '!' || c == '?'; }));
    const double average = words.empty() ? 0.0 : static_cast<double>(letters) / static_cast<double>(words.size());
    return {text.size(), words.size(), sentences, average};
}

std::vector<std::pair<std::string, std::size_t>> top_words(std::string_view text, std::size_t n) {
    std::map<std::string, std::size_t> counts;
    for (const auto& word : words_of(text)) {
        ++counts[word];
    }
    std::vector<std::pair<std::string, std::size_t>> ranking(counts.begin(), counts.end());
    std::stable_sort(ranking.begin(), ranking.end(), [](const auto& a, const auto& b) { return a.second > b.second; });
    if (ranking.size() > n) {
        ranking.resize(n);
    }
    return ranking;
}

}  // namespace textkit
