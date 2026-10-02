/**
 * @file
 * Day 29 – Using External Libraries.
 *
 * Scenario: a *newsletter editor's text toolkit* built on a library called textkit, which
 * ships in two flavours exactly like real dependencies do: a header-only part (just include
 * it) and a compiled static library (CMake target `textkit`, linked by this lesson). A
 * dependency checker reads a manifest, compares semantic versions and prints the CMake you
 * would write to get each library with find_package or FetchContent.
 *
 * Deliverables (syllabus):
 * - Header-only vs compiled libraries
 * - Linking a static library
 * - find_package and FetchContent
 * - Version checks
 */
#pragma once

#include <compare>
#include <istream>
#include <map>
#include <optional>
#include <ostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

#include "cppm/lesson.hpp"
#include "textkit/case.hpp"
#include "textkit/stats.hpp"
#include "textkit/version.hpp"

namespace cppm::day29 {

inline constexpr Deliverable DELIVERABLES[] = {
    {"a header-only library: include and use", "headline"},
    {"a compiled static library: declare, link, call", "summarise"},
    {"semantic versions that compare correctly", "Version"},
    {"version requirements: >=, ^ and ==", "satisfies"},
    {"checking a manifest against installed versions", "check_manifest"},
    {"the CMake for find_package or FetchContent", "cmake_snippet"},
};

// Compile-time version check: the build fails if headers from an incompatible major version are used.
static_assert(TEXTKIT_VERSION_MAJOR == 1, "this lesson is written for textkit 1.x");

/// Uses only textkit/case.hpp – nothing has to be linked for these functions.
inline std::string headline(std::string_view title) {
    return textkit::title_case(title) + "  [" + textkit::slug(title) + "]";
}

/// Uses textkit/stats.hpp, whose definitions come from the linked `textkit` library.
inline std::string summarise(std::string_view text) {
    const auto stats = textkit::analyse(text);
    std::ostringstream out;
    out << stats.words << " words, " << stats.sentences << " sentence(s), top:";
    for (const auto& [word, count] : textkit::top_words(text, 3)) {
        out << ' ' << word << '(' << count << ')';
    }
    return out.str();
}

/// MAJOR.MINOR.PATCH; the defaulted <=> compares member by member, in order.
struct Version {
    // not `major`/`minor`: some C libraries define macros with those names
    int major_version{0};
    int minor_version{0};
    int patch_version{0};
    auto operator<=>(const Version&) const = default;
};

/// Parse "1", "1.4" or "1.4.2".
inline Version parse_version(std::string_view text) {
    Version v;
    std::istringstream in{std::string(text)};
    char dot{};
    if (!(in >> v.major_version) || v.major_version < 0) {
        throw std::invalid_argument("not a version: " + std::string(text));
    }
    if (in >> dot && (dot != '.' || !(in >> v.minor_version))) {
        throw std::invalid_argument("not a version: " + std::string(text));
    }
    if (in >> dot && (dot != '.' || !(in >> v.patch_version))) {
        throw std::invalid_argument("not a version: " + std::string(text));
    }
    return v;
}

inline std::string to_string(const Version& v) {
    return std::to_string(v.major_version) + '.' + std::to_string(v.minor_version) + '.' +
           std::to_string(v.patch_version);
}

/// ">=1.2" any newer version; "^1.2.0" same major, at least 1.2.0; "==1.0.0" exactly; "1.2" means ">=1.2".
inline bool satisfies(const Version& installed, std::string_view requirement) {
    if (requirement.starts_with(">=")) {
        return installed >= parse_version(requirement.substr(2));
    }
    if (requirement.starts_with("^")) {
        const Version minimum = parse_version(requirement.substr(1));
        return installed.major_version == minimum.major_version && installed >= minimum;
    }
    if (requirement.starts_with("==")) {
        return installed == parse_version(requirement.substr(2));
    }
    return installed >= parse_version(requirement);
}

/// For each required package: "ok", "missing" or "too old/new".
inline std::map<std::string, std::string> check_manifest(const std::map<std::string, std::string>& required,
                                                         const std::map<std::string, std::string>& installed) {
    std::map<std::string, std::string> report;
    for (const auto& [name, requirement] : required) {
        const auto found = installed.find(name);
        if (found == installed.end()) {
            report[name] = "missing";
        } else if (satisfies(parse_version(found->second), requirement)) {
            report[name] = "ok (" + found->second + ")";
        } else {
            report[name] = "installed " + found->second + " does not satisfy " + requirement;
        }
    }
    return report;
}

enum class Acquire { find_package, fetch_content };

/// The CMake a consumer writes for @p package, either from the system or downloaded at configure time.
inline std::string cmake_snippet(const std::string& package, const std::string& version, Acquire how,
                                 const std::string& git_url = "") {
    if (how == Acquire::find_package) {
        return "find_package(" + package + " " + version +
               " CONFIG REQUIRED)\n"
               "target_link_libraries(app PRIVATE " +
               package + "::" + package + ")\n";
    }
    if (git_url.empty()) {
        throw std::invalid_argument("FetchContent needs a repository URL");
    }
    return "include(FetchContent)\n"
           "FetchContent_Declare(" +
           package + " GIT_REPOSITORY " + git_url + " GIT_TAG v" + version +
           ")\n"
           "FetchContent_MakeAvailable(" +
           package +
           ")\n"
           "target_link_libraries(app PRIVATE " +
           package + ")\n";
}

/// The interactive demo: paste a newsletter paragraph; blank line to finish.
inline int run(std::istream& in, std::ostream& out) {
    out << "Day 29 – Using External Libraries\n"
        << "textkit headers " << TEXTKIT_VERSION_MAJOR << '.' << TEXTKIT_VERSION_MINOR << '.' << TEXTKIT_VERSION_PATCH
        << ", linked library " << textkit::version() << '\n';
    out << headline("weekly c++ digest") << '\n';
    while (auto line = prompt_line(in, out, "paragraph> ")) {
        if (line->empty()) {
            break;
        }
        out << "  " << summarise(*line) << '\n';
    }
    for (const auto& [package, status] :
         check_manifest({{"fmt", "^10.0.0"}, {"nlohmann_json", ">=3.11"}, {"textkit", "==1.4.2"}},
                        {{"fmt", "10.2.1"}, {"textkit", textkit::version()}})) {
        out << "  " << package << ": " << status << '\n';
    }
    out << cmake_snippet("nlohmann_json", "3.11.3", Acquire::fetch_content, "https://github.com/nlohmann/json.git");
    return 0;
}

}  // namespace cppm::day29
