// Tests for Day 79 – Build Systems & Packaging. The unitconv library is linked like any dependency.
#include <algorithm>
#include <map>
#include <set>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

#include "cppm/testing.hpp"
#include "day_79_build_packaging/lesson.hpp"

using namespace cppm::day79;

TEST_CASE("the generated header and the compiled library agree on the version") {
    CHECK_EQ(std::string(UNITCONV_VERSION), "2.1.0");
    CHECK_EQ(std::string(unitconv::library_version()), UNITCONV_VERSION);
    CHECK(header_version() == Version{2, 1, 0});
}

TEST_CASE("the linked library converts through its public API") {
    CHECK_NEAR(unitconv::convert(1, "mi", "km"), 1.609344, 1e-9);
    CHECK_NEAR(unitconv::convert(100, "C", "F"), 212.0, 1e-9);
    CHECK_NEAR(unitconv::convert(0, "K", "C"), -273.15, 1e-9);
    CHECK_THROWS_AS(unitconv::convert(1, "kg", "m"), std::invalid_argument);
    CHECK_THROWS_AS(unitconv::convert(1, "parsec", "m"), std::invalid_argument);
}

TEST_CASE("versions parse strictly and compare numerically") {
    CHECK(parse_version("2.10.3") > parse_version("2.9.9"));
    CHECK(parse_version("3.1") == Version{3, 1, 0});
    for (const char* bad : {"2", "2.", "2..1", "02.1.0", "2.1.0.4", "v2.1", "2.x"}) {
        CHECK_THROWS_AS(parse_version(bad), std::invalid_argument);
    }
}

TEST_CASE("SameMajorVersion accepts newer minors but never another major") {
    const Version installed{2, 1, 0};
    CHECK(compatible({2, 0, 0}, installed));
    CHECK(compatible({2, 1, 0}, installed));
    CHECK(!compatible({2, 2, 0}, installed));  // too old
    CHECK(!compatible({1, 0, 0}, installed));  // breaking change in between
    CHECK(!compatible({3, 0, 0}, installed));
}

TEST_CASE("targets are built after their dependencies, and cycles are rejected") {
    const auto order = build_order({{"app", {"unitconv", "logging"}}, {"unitconv", {}}, {"logging", {"fmtlite"}}, {"fmtlite", {}}});
    auto pos = [&](const std::string& t) { return std::find(order.begin(), order.end(), t) - order.begin(); };
    CHECK_EQ(order.size(), 4u);
    CHECK(pos("fmtlite") < pos("logging"));
    CHECK(pos("logging") < pos("app"));
    CHECK(pos("unitconv") < pos("app"));
    try {
        build_order({{"a", {"b"}}, {"b", {"c"}}, {"c", {"a"}}});
        CHECK(false);
    } catch (const std::runtime_error& e) {
        CHECK_EQ(std::string(e.what()), "dependency cycle: a -> b -> c -> a");
    }
}

TEST_CASE("usage requirements propagate by scope") {
    const std::map<std::string, Target> targets{
        {"unitconv", {{{"unitconv/include", Scope::PUBLIC}, {"unitconv/src", Scope::PRIVATE}}, {}}},
        {"headers_only", {{{"ho/include", Scope::INTERFACE}}, {}}},
        {"service", {{{"service/src", Scope::PRIVATE}}, {{"unitconv", Scope::PUBLIC}, {"headers_only", Scope::PRIVATE}}}},
        {"app", {{}, {{"service", Scope::PRIVATE}}}}};
    CHECK(effective_includes(targets, "unitconv") == std::set<std::string>{"unitconv/include", "unitconv/src"});
    CHECK(effective_includes(targets, "headers_only").empty());  // INTERFACE: for consumers only
    CHECK(effective_includes(targets, "service") == std::set<std::string>{"service/src", "unitconv/include", "ho/include"});
    CHECK(effective_includes(targets, "app") == std::set<std::string>{"unitconv/include"});  // only what service made PUBLIC
}

TEST_CASE("CPack file names and the demo") {
    CHECK_EQ(package_file_name("unitconv", {2, 1, 0}, "Linux", "TGZ"), "unitconv-2.1.0-Linux.tar.gz");
    CHECK_EQ(package_file_name("unitconv", {2, 1, 0}, "win64", "NSIS"), "unitconv-2.1.0-win64.exe");
    CHECK_THROWS_AS(package_file_name("x", {1, 0, 0}, "Linux", "MSI"), std::invalid_argument);
    std::istringstream in("5 km mi\n20 C F\n1 kg ft\nneed 2.0\nneed 3.0\nstop\n");
    std::ostringstream out;
    CHECK_EQ(run(in, out), 0);
    const auto text = out.str();
    CHECK(text.find("unitconv 2.1.0 (header 2.1.0), package unitconv-2.1.0-Linux.tar.gz") != std::string::npos);
    CHECK(text.find("5 km = 3.107 mi") != std::string::npos);
    CHECK(text.find("20 C = 68 F") != std::string::npos);
    CHECK(text.find("cannot convert kg to ft") != std::string::npos);
    CHECK(text.find("find_package(unitconv 2.0): found") != std::string::npos);
    CHECK(text.find("find_package(unitconv 3.0): not compatible") != std::string::npos);
}
