/**
 * @file
 * Day 79 – Build Systems & Packaging.
 *
 * Scenario: *shipping an internal library*. Three in-house apps convert units with copy-pasted
 * code; today it becomes "unitconv", a proper CMake package (see unitconv/CMakeLists.txt) with
 * targets and usage requirements, install and export rules, a generated version header and a
 * CPack configuration. This file models the rules the build system applies – semantic-version
 * compatibility, build order, and how PUBLIC/PRIVATE/INTERFACE requirements propagate – so they
 * can be tested, and it uses the real library through its public API.
 *
 * Deliverables (syllabus):
 * - CMake targets and usage requirements
 * - Install and export rules
 * - Versioning
 * - CPack
 */
#pragma once

#include <compare>
#include <cstdio>
#include <functional>
#include <istream>
#include <map>
#include <ostream>
#include <set>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

#include "cppm/lesson.hpp"
#include "unitconv/convert.hpp"
#include "unitconv/version.hpp"

namespace cppm::day79 {

inline constexpr Deliverable DELIVERABLES[] = {
    {"parsing MAJOR.MINOR.PATCH versions", "parse_version"},
    {"the SameMajorVersion rule used by find_package", "compatible"},
    {"a dependency-ordered build with cycle detection", "build_order"},
    {"PUBLIC, PRIVATE and INTERFACE usage requirements", "effective_includes"},
    {"CPack's package file naming", "package_file_name"},
};

struct Version {
    int major = 0;
    int minor = 0;
    int patch = 0;
    auto operator<=>(const Version&) const = default;
    std::string str() const {
        return std::to_string(major) + "." + std::to_string(minor) + "." + std::to_string(patch);
    }
};

/// "2.1.0" or "2.1" (patch 0). Leading zeros and extra parts are rejected.
inline Version parse_version(const std::string& text) {
    std::vector<int> parts;
    std::istringstream in(text);
    for (std::string piece; std::getline(in, piece, '.');) {
        if (piece.empty() || piece.size() > 6 || (piece.size() > 1 && piece[0] == '0') ||
            piece.find_first_not_of("0123456789") != std::string::npos) {
            throw std::invalid_argument("bad version '" + text + "'");
        }
        parts.push_back(std::stoi(piece));
    }
    if (parts.size() < 2 || parts.size() > 3 || text.back() == '.')
        throw std::invalid_argument("bad version '" + text + "'");
    return {parts[0], parts[1], parts.size() == 3 ? parts[2] : 0};
}

/// find_package(unitconv 2.0) with COMPATIBILITY SameMajorVersion: the major versions must match
/// (a new major may break the API) and the available version must be at least the requested one.
inline bool compatible(const Version& requested, const Version& available) {
    return requested.major == available.major && available >= requested;
}

/// Targets and what they link to. Returns an order in which every target comes after its
/// dependencies – the order a build tool must compile them in. Throws on a cycle.
inline std::vector<std::string> build_order(const std::map<std::string, std::vector<std::string>>& deps) {
    std::vector<std::string> order;
    std::map<std::string, int> state;  // 0 unvisited, 1 visiting, 2 done
    std::function<void(const std::string&, std::vector<std::string>&)> visit = [&](const std::string& target,
                                                                                   std::vector<std::string>& path) {
        if (state[target] == 2) return;
        path.push_back(target);
        if (state[target] == 1) {
            std::string cycle;
            for (const auto& t : path) cycle += (cycle.empty() ? "" : " -> ") + t;
            throw std::runtime_error("dependency cycle: " + cycle);
        }
        state[target] = 1;
        if (const auto it = deps.find(target); it != deps.end()) {
            for (const auto& dep : it->second) visit(dep, path);
        }
        state[target] = 2;
        path.pop_back();
        order.push_back(target);
    };
    for (const auto& [target, list] : deps) {
        std::vector<std::string> path;
        visit(target, path);
    }
    return order;
}

enum class Scope { PRIVATE, PUBLIC, INTERFACE };

struct Requirement {
    std::string include_dir;
    Scope scope;
};

struct Target {
    std::vector<Requirement> includes;
    std::vector<std::pair<std::string, Scope>> links;  // linked target and the link scope
};

/// The include directories a target compiles with. PRIVATE and PUBLIC apply to the target
/// itself; PUBLIC and INTERFACE propagate to targets linking it, but only through PUBLIC or
/// INTERFACE links further up – a PRIVATE link stops propagation.
inline std::set<std::string> effective_includes(const std::map<std::string, Target>& targets, const std::string& name) {
    std::function<std::set<std::string>(const std::string&)> interface_of = [&](const std::string& t) {
        std::set<std::string> out;
        const auto& target = targets.at(t);
        for (const auto& r : target.includes) {
            if (r.scope != Scope::PRIVATE) out.insert(r.include_dir);
        }
        for (const auto& [dep, scope] : target.links) {
            if (scope != Scope::PRIVATE) out.merge(interface_of(dep));
        }
        return out;
    };
    const auto& target = targets.at(name);
    std::set<std::string> out;
    for (const auto& r : target.includes) {
        if (r.scope != Scope::INTERFACE) out.insert(r.include_dir);
    }
    for (const auto& [dep, scope] : target.links) {
        if (scope != Scope::INTERFACE) out.merge(interface_of(dep));
    }
    return out;
}

/// CPACK_PACKAGE_FILE_NAME defaults to <name>-<version>-<system>, plus the generator's extension.
inline std::string package_file_name(const std::string& name, const Version& version, const std::string& system,
                                     const std::string& generator) {
    const std::map<std::string, std::string> extensions{
        {"TGZ", ".tar.gz"}, {"ZIP", ".zip"}, {"DEB", ".deb"}, {"NSIS", ".exe"}};
    const auto it = extensions.find(generator);
    if (it == extensions.end()) throw std::invalid_argument("unknown CPack generator " + generator);
    return name + "-" + version.str() + "-" + system + it->second;
}

/// The version this program was compiled against (from the generated header).
inline Version header_version() {
    return {UNITCONV_VERSION_MAJOR, UNITCONV_VERSION_MINOR, UNITCONV_VERSION_PATCH};
}

/// The interactive demo: "value from to" conversions, or "need <version>" to check compatibility.
inline int run(std::istream& in, std::ostream& out) {
    out << "Day 79 – Build Systems & Packaging\n";
    out << "unitconv " << unitconv::library_version() << " (header " << UNITCONV_VERSION << "), package "
        << package_file_name("unitconv", header_version(), "Linux", "TGZ") << '\n';
    while (auto line = prompt_line(in, out, "<value> <from> <to> | need <version>> ")) {
        std::istringstream words(*line);
        std::string first;
        if (!(words >> first)) break;
        try {
            if (first == "need") {
                std::string wanted;
                words >> wanted;
                out << "  find_package(unitconv " << wanted
                    << "): " << (compatible(parse_version(wanted), header_version()) ? "found" : "not compatible")
                    << '\n';
                continue;
            }
            std::string from;
            std::string to;
            words >> from >> to;
            const double value = std::stod(first);
            char result[64];
            std::snprintf(result, sizeof result, "%.4g", unitconv::convert(value, from, to));
            out << "  " << first << ' ' << from << " = " << result << ' ' << to << '\n';
        } catch (const std::exception& error) {
            out << "  " << error.what() << '\n';
        }
    }
    return 0;
}

}  // namespace cppm::day79
