// Tests for Day 29 – Using External Libraries.
#include <sstream>
#include <stdexcept>
#include <string>

#include "cppm/testing.hpp"
#include "day_29_external_libraries/lesson.hpp"

using namespace cppm::day29;

TEST_CASE("the header-only part works without linking anything") {
    CHECK_EQ(textkit::title_case("weekly c++ digest"), "Weekly C++ Digest");
    CHECK_EQ(textkit::slug("Hello, World!"), "hello-world");
    CHECK_EQ(headline("release notes"), "Release Notes  [release-notes]");
}

TEST_CASE("the compiled part comes from the linked textkit library") {
    const auto stats = textkit::analyse("C++ is fast. C++ is fun!");
    CHECK_EQ(stats.words, 6u);
    CHECK_EQ(stats.sentences, 2u);
    CHECK_EQ(summarise("the cat saw the other cat. the end."), "8 words, 2 sentence(s), top: the(3) cat(2) end(1)");
}

TEST_CASE("headers and the linked binary report the same version") {
    const std::string headers = std::to_string(TEXTKIT_VERSION_MAJOR) + "." + std::to_string(TEXTKIT_VERSION_MINOR) +
                                "." + std::to_string(TEXTKIT_VERSION_PATCH);
    CHECK_EQ(std::string(textkit::version()), headers);
}

TEST_CASE("versions parse and compare component by component") {
    CHECK(parse_version("1.10.0") > parse_version("1.9.9"));
    CHECK(parse_version("2") == Version{2, 0, 0});
    CHECK_EQ(to_string(parse_version("3.11")), "3.11.0");
    CHECK_THROWS_AS(parse_version("1.x"), std::invalid_argument);
    CHECK_THROWS_AS(parse_version("v1"), std::invalid_argument);
}

TEST_CASE("requirements follow >=, ^ and == semantics") {
    const Version v{10, 2, 1};
    CHECK(satisfies(v, ">=10.0"));
    CHECK(satisfies(v, "^10.1.0"));
    CHECK(!satisfies(v, "^9.0.0"));
    CHECK(!satisfies(v, "^10.3.0"));
    CHECK(satisfies(v, "==10.2.1"));
    CHECK(!satisfies(v, "==10.2.0"));
    CHECK(satisfies(v, "9.5"));
}

TEST_CASE("check_manifest reports ok, missing and mismatched packages") {
    const auto report = check_manifest({{"fmt", "^10.0.0"}, {"json", ">=3.11"}, {"boost", "==1.84.0"}},
                                       {{"fmt", "10.2.1"}, {"boost", "1.83.0"}});
    CHECK_EQ(report.at("fmt"), "ok (10.2.1)");
    CHECK_EQ(report.at("json"), "missing");
    CHECK_EQ(report.at("boost"), "installed 1.83.0 does not satisfy ==1.84.0");
}

TEST_CASE("cmake_snippet writes find_package or FetchContent code") {
    const auto system = cmake_snippet("fmt", "10", Acquire::find_package);
    CHECK(system.find("find_package(fmt 10 CONFIG REQUIRED)") != std::string::npos);
    CHECK(system.find("fmt::fmt") != std::string::npos);
    const auto fetched = cmake_snippet("json", "3.11.3", Acquire::fetch_content, "https://example.org/json.git");
    CHECK(fetched.find("GIT_TAG v3.11.3") != std::string::npos);
    CHECK(fetched.find("FetchContent_MakeAvailable(json)") != std::string::npos);
    CHECK_THROWS_AS(cmake_snippet("json", "1", Acquire::fetch_content), std::invalid_argument);
}

TEST_CASE("run summarises paragraphs and checks the manifest") {
    std::istringstream in("Short news. More news!\n\n");
    std::ostringstream out;
    CHECK_EQ(run(in, out), 0);
    const auto text = out.str();
    CHECK(text.find("linked library 1.4.2") != std::string::npos);
    CHECK(text.find("4 words, 2 sentence(s)") != std::string::npos);
    CHECK(text.find("nlohmann_json: missing") != std::string::npos);
    CHECK(text.find("textkit: ok (1.4.2)") != std::string::npos);
    CHECK(text.find("FetchContent_Declare(nlohmann_json") != std::string::npos);
}
