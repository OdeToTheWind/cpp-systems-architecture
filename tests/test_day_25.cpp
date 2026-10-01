// Tests for Day 25 – Local Development Environment Setup.
#include <sstream>
#include <string>

#include "cppm/temp_dir.hpp"
#include "cppm/testing.hpp"
#include "day_25_dev_environment/lesson.hpp"

using namespace cppm::day25;

namespace {
int count(const std::vector<Finding>& findings, Severity severity) {
    int n = 0;
    for (const auto& f : findings) n += f.severity == severity ? 1 : 0;
    return n;
}
}  // namespace

TEST_CASE("the compiler compiling these tests reports C++20 or newer") {
    const auto info = detect_compiler();
    CHECK(!info.name.empty());
    CHECK(info.standard >= 202002L);
}

TEST_CASE("standard_name maps __cplusplus values") {
    CHECK_EQ(standard_name(201703L), "C++17");
    CHECK_EQ(standard_name(202002L), "C++20");
    CHECK_EQ(standard_name(202302L), "C++23");
    CHECK_EQ(standard_name(199711L), "C++11 or older");
}

TEST_CASE("preset_names extracts every preset") {
    const auto names = preset_names(R"({"configurePresets": [{"name": "dev"}, {"name" : "sanitize"}]})");
    CHECK(names == std::set<std::string>{"dev", "sanitize"});
    CHECK(preset_names("not json").empty());
}

TEST_CASE("check_warnings needs strict flags and a way to make them errors") {
    CHECK_EQ(count(check_warnings("add_compile_options(-Wall -Wextra -Werror)"), Severity::ok), 2);
    CHECK_EQ(count(check_warnings("/W4 /WX"), Severity::ok), 2);
    const auto weak = check_warnings("add_compile_options(-O2)");
    CHECK_EQ(count(weak, Severity::error), 1);
    CHECK_EQ(count(weak, Severity::warning), 1);
}

TEST_CASE("check_sanitizers looks at flags and presets") {
    CHECK_EQ(count(check_sanitizers("-fsanitize=address", {"sanitize"}), Severity::ok), 2);
    CHECK_EQ(count(check_sanitizers("", {}), Severity::warning), 2);
}

TEST_CASE("diagnose a healthy project in a sandbox") {
    cppm::TempDir dir("day25");
    dir.write("CMakeLists.txt", "add_compile_options(-Wall -Wextra) option(WERROR) -Werror -fsanitize=address");
    dir.write("CMakePresets.json", R"({"configurePresets":[{"name":"dev"},{"name":"sanitize"}]})");
    dir.write(".clang-format", "BasedOnStyle: Google");
    dir.write(".gitignore", "build/\n");
    dir.write("tests/test_x.cpp", "");
    const auto findings = diagnose(dir.path(), {"GCC", "13", 202002L});
    CHECK_EQ(count(findings, Severity::warning) + count(findings, Severity::error), 0);
}

TEST_CASE("diagnose an empty folder lists every missing piece") {
    cppm::TempDir dir("day25");
    const auto findings = diagnose(dir.path(), {"GCC", "9", 201703L});
    CHECK(count(findings, Severity::error) >= 3);  // C++17, no CMakeLists.txt, no strict warnings
    CHECK(count(findings, Severity::ok) == 0);
}

TEST_CASE("this repository passes its own doctor") {
    const std::filesystem::path root = std::filesystem::path(__FILE__).parent_path().parent_path();
    const auto findings = diagnose(root, detect_compiler());
    CHECK_EQ(count(findings, Severity::error) + count(findings, Severity::warning), 0);
}

TEST_CASE("run reports on a chosen folder") {
    cppm::TempDir dir("day25");
    std::istringstream in(dir.path().string() + "\n");
    std::ostringstream out;
    CHECK_EQ(run(in, out), 0);
    CHECK(out.str().find("FAIL no CMakeLists.txt") != std::string::npos);
    CHECK(out.str().find("thing(s) to improve") != std::string::npos);
}
