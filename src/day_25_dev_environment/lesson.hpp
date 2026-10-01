/**
 * @file
 * Day 25 – Local Development Environment Setup.
 *
 * Scenario: a *project doctor* that examines a C++ checkout – this repository by default –
 * and reports whether the local setup follows best practice: a CMake build, presets for
 * development and sanitizers, strict warnings, a formatter configuration, an ignored build
 * folder, and a modern compiler and language standard.
 *
 * Deliverables (syllabus):
 * - Compilers
 * - CMake presets
 * - Warnings as errors
 * - Sanitizers
 * - A reproducible project layout
 */
#pragma once

#include <filesystem>
#include <fstream>
#include <istream>
#include <ostream>
#include <regex>
#include <set>
#include <sstream>
#include <string>
#include <string_view>
#include <vector>

#include "cppm/lesson.hpp"

namespace cppm::day25 {

inline constexpr Deliverable DELIVERABLES[] = {
    {"identifying the compiler and standard from predefined macros", "detect_compiler"},
    {"reading CMake preset names", "preset_names"},
    {"checking that warnings can be made errors", "check_warnings"},
    {"checking for a sanitizer configuration", "check_sanitizers"},
    {"checking the project layout", "check_layout"},
    {"one report combining every check", "diagnose"},
};

/// What compiled this program.
struct CompilerInfo {
    std::string name;
    std::string version;
    long standard;  // the value of __cplusplus, e.g. 202002L for C++20
};

/// Uses predefined macros, which every compiler sets. Note: MSVC reports __cplusplus correctly
/// only with /Zc:__cplusplus, which this project's CMakeLists.txt enables.
inline CompilerInfo detect_compiler() {
#if defined(__clang__)
    return {"Clang", std::to_string(__clang_major__) + "." + std::to_string(__clang_minor__), __cplusplus};
#elif defined(__GNUC__)
    return {"GCC", std::to_string(__GNUC__) + "." + std::to_string(__GNUC_MINOR__), __cplusplus};
#elif defined(_MSC_VER)
    return {"MSVC", std::to_string(_MSC_VER), _MSVC_LANG};
#else
    return {"unknown", "?", __cplusplus};
#endif
}

/// 201703L -> "C++17", 202002L -> "C++20", 202302L -> "C++23".
inline std::string standard_name(long value) {
    if (value >= 202302L) return "C++23";
    if (value > 202002L) return "C++23 (partial)";
    if (value >= 202002L) return "C++20";
    if (value >= 201703L) return "C++17";
    if (value >= 201402L) return "C++14";
    return "C++11 or older";
}

enum class Severity { ok, warning, error };

/// One line of the doctor's report.
struct Finding {
    Severity severity;
    std::string message;
};

inline std::string read_file(const std::filesystem::path& path) {
    std::ifstream in(path, std::ios::binary);
    std::ostringstream text;
    text << in.rdbuf();
    return text.str();
}

/// Every `"name": "..."` in a CMakePresets.json file (a regex is enough for this check).
inline std::set<std::string> preset_names(std::string_view presets_json) {
    std::set<std::string> names;
    static const std::regex name_field(R"re("name"\s*:\s*"([^"]+)")re");
    const std::string text(presets_json);
    for (auto it = std::sregex_iterator(text.begin(), text.end(), name_field); it != std::sregex_iterator(); ++it) {
        names.insert((*it)[1]);
    }
    return names;
}

inline void add(std::vector<Finding>& findings, bool good, Severity if_bad, const std::string& ok_text,
                const std::string& bad_text) {
    findings.push_back(good ? Finding{Severity::ok, ok_text} : Finding{if_bad, bad_text});
}

/// -Wall/-Wextra (or /W4) must be on, and an option must exist to turn warnings into errors.
inline std::vector<Finding> check_warnings(std::string_view cmake_text) {
    std::vector<Finding> findings;
    const bool strict = (cmake_text.find("-Wall") != std::string_view::npos &&
                         cmake_text.find("-Wextra") != std::string_view::npos) ||
                        cmake_text.find("/W4") != std::string_view::npos;
    const bool werror = cmake_text.find("-Werror") != std::string_view::npos ||
                        cmake_text.find("/WX") != std::string_view::npos ||
                        cmake_text.find("COMPILE_WARNING_AS_ERROR") != std::string_view::npos;
    add(findings, strict, Severity::error, "strict warnings enabled", "enable -Wall -Wextra (or /W4)");
    add(findings, werror, Severity::warning, "warnings can be treated as errors", "add a -Werror / /WX option");
    return findings;
}

/// AddressSanitizer and UBSan must be available through CMake and a preset.
inline std::vector<Finding> check_sanitizers(std::string_view cmake_text, const std::set<std::string>& presets) {
    std::vector<Finding> findings;
    const bool flags = cmake_text.find("-fsanitize=") != std::string_view::npos;
    add(findings, flags, Severity::warning, "sanitizer flags configured", "add -fsanitize=address,undefined");
    add(findings, presets.contains("sanitize"), Severity::warning, "a 'sanitize' preset exists",
        "add a 'sanitize' configure preset");
    return findings;
}

/// The files and folders a reproducible C++ project should have.
inline std::vector<Finding> check_layout(const std::filesystem::path& root) {
    std::vector<Finding> findings;
    add(findings, std::filesystem::exists(root / "CMakeLists.txt"), Severity::error, "CMakeLists.txt found",
        "no CMakeLists.txt – the project cannot be configured");
    add(findings, std::filesystem::exists(root / ".clang-format"), Severity::warning, ".clang-format found",
        "add .clang-format so every editor formats the same way");
    add(findings, std::filesystem::is_directory(root / "tests"), Severity::warning, "tests/ folder found",
        "add a tests/ folder");
    const std::string gitignore = read_file(root / ".gitignore");
    add(findings, gitignore.find("build") != std::string::npos, Severity::warning, "build output is git-ignored",
        "ignore build/ in .gitignore");
    return findings;
}

/// Run every check against the project at @p root.
inline std::vector<Finding> diagnose(const std::filesystem::path& root, const CompilerInfo& compiler) {
    std::vector<Finding> findings;
    add(findings, compiler.standard >= 202002L, Severity::error, "compiling as " + standard_name(compiler.standard),
        "this course needs C++20, the compiler reports " + standard_name(compiler.standard));
    for (auto& finding : check_layout(root)) findings.push_back(finding);
    const std::string cmake = read_file(root / "CMakeLists.txt");
    const auto presets = preset_names(read_file(root / "CMakePresets.json"));
    add(findings, presets.contains("dev"), Severity::warning, "a 'dev' preset exists", "add CMakePresets.json with a 'dev' preset");
    for (auto& finding : check_warnings(cmake)) findings.push_back(finding);
    for (auto& finding : check_sanitizers(cmake, presets)) findings.push_back(finding);
    return findings;
}

/// The interactive demo: diagnose a folder (default: the current directory).
inline int run(std::istream& in, std::ostream& out) {
    const auto compiler = detect_compiler();
    out << "Day 25 – Local Development Environment Setup\n"
        << "Compiler: " << compiler.name << ' ' << compiler.version << ", " << standard_name(compiler.standard) << '\n';
    auto folder = prompt_line(in, out, "Project folder (blank = current directory): ");
    const std::filesystem::path root = (folder && !folder->empty()) ? std::filesystem::path(*folder)
                                                                    : std::filesystem::current_path();
    int problems = 0;
    for (const auto& finding : diagnose(root, compiler)) {
        const char* tag = finding.severity == Severity::ok ? "  ok   " : finding.severity == Severity::warning ? "  WARN " : "  FAIL ";
        problems += finding.severity == Severity::ok ? 0 : 1;
        out << tag << finding.message << '\n';
    }
    out << (problems == 0 ? "Healthy project.\n" : std::to_string(problems) + " thing(s) to improve.\n");
    return 0;
}

}  // namespace cppm::day25
