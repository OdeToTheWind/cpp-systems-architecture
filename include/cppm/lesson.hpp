/**
 * @file
 * Shared vocabulary for every day's lesson.
 *
 * Each `src/day_XX_<topic>/lesson.hpp` opens with a doc comment containing a
 * `Scenario:` paragraph and declares a `DELIVERABLES` table: one entry per syllabus
 * skill, naming the function, class or constant that implements it. The tooling in
 * `scripts/` reads that table to generate the reflections, and CI checks that every
 * symbol it names really exists.
 */
#pragma once

#include <istream>
#include <limits>
#include <optional>
#include <ostream>
#include <string>
#include <string_view>

namespace cppm {

/// Maps one syllabus skill to the symbol that demonstrates it.
struct Deliverable {
    std::string_view skill;
    std::string_view symbol;
};

/// Print @p prompt and read one whole line; std::nullopt at end of input.
inline std::optional<std::string> prompt_line(std::istream& in, std::ostream& out, std::string_view prompt) {
    out << prompt;
    std::string line;
    if (!std::getline(in, line)) {
        return std::nullopt;
    }
    if (!line.empty() && line.back() == '\r') {  // tolerate Windows line endings
        line.pop_back();
    }
    return line;
}

}  // namespace cppm
