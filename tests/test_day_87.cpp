// Tests for Day 87 – Capstone: Plugin Architecture.
#include <memory>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

#include "cppm/testing.hpp"
#include "day_87_plugins/lesson.hpp"

using namespace cppm::day87;

namespace {
Image row(std::vector<std::uint8_t> pixels) {
    const int w = static_cast<int>(pixels.size());
    return {w, 1, std::move(pixels)};
}
std::vector<std::string> names() {
    std::vector<std::string> out;
    for (const auto* p : PluginRegistry::instance().list()) out.push_back(p->name);
    return out;
}
}  // namespace

TEST_CASE("built-in plugins registered themselves before main") {
    CHECK(names() == std::vector<std::string>{"blur", "brighten", "invert", "threshold"});
    CHECK(plugins::invert_registered);
    CHECK(!plugins::legacy_registered);
}

TEST_CASE("incompatible API versions are refused with a reason") {
    const auto& rejected = PluginRegistry::instance().rejected();
    CHECK_EQ(rejected.at("sepia"), "needs API 1.x");
    CHECK_EQ(rejected.at("hdr"), "needs API 2.9 or newer");
    try {
        PluginRegistry::instance().create("hdr", {});
        CHECK(false);
    } catch (const std::invalid_argument& e) {
        CHECK_EQ(std::string(e.what()), "no plugin 'hdr' (rejected: needs API 2.9 or newer)");
    }
}

TEST_CASE("duplicate names are refused, other plugins can be added at run time") {
    CHECK(!register_plugin({"invert", "impostor", {2, 0}, [](const Options&) { return std::make_unique<plugins::Invert>(); }}));
    CHECK_EQ(PluginRegistry::instance().rejected().at("invert"), "duplicate name");
    CHECK(register_plugin({"zero", "all black", {2, 3}, [](const Options&) { return std::make_unique<plugins::Threshold>(256); }}));
    Image img = row({1, 200});
    PluginRegistry::instance().create("zero", {})->apply(img);
    CHECK(img.pixels == std::vector<std::uint8_t>{0, 0});
}

TEST_CASE("factories configure instances from options") {
    Image img = row({10, 100, 200});
    PluginRegistry::instance().create("threshold", {{"level", "100"}})->apply(img);
    CHECK(img.pixels == std::vector<std::uint8_t>{0, 255, 255});
    Image bright = row({10, 250});
    PluginRegistry::instance().create("brighten", {})->apply(bright);
    CHECK(bright.pixels == std::vector<std::uint8_t>{50, 255});  // clamped
    CHECK_THROWS_AS(PluginRegistry::instance().create("threshold", {{"level", "300"}}), std::invalid_argument);
    CHECK_THROWS_AS(PluginRegistry::instance().create("brighten", {{"amount", "lots"}}), std::invalid_argument);
}

TEST_CASE("the blur averages neighbours, including at the edges") {
    Image img{3, 1, {0, 90, 0}};
    plugins::BoxBlur{}.apply(img);
    CHECK(img.pixels == std::vector<std::uint8_t>{45, 30, 45});
}

TEST_CASE("pipelines are built from names and options") {
    const auto filters = build_pipeline("invert | threshold level=128 ||");
    CHECK_EQ(filters.size(), 2u);
    Image img = row({0, 200});
    for (const auto& f : filters) f->apply(img);
    CHECK(img.pixels == std::vector<std::uint8_t>{255, 0});
    CHECK_THROWS_AS(build_pipeline("threshold 100"), std::invalid_argument);
    CHECK_THROWS_AS(build_pipeline("emboss"), std::invalid_argument);
}

TEST_CASE("run lists plugins and renders pipelines") {
    std::istringstream in("threshold level=128\nsepia\n\n");
    std::ostringstream out;
    CHECK_EQ(run(in, out), 0);
    const auto text = out.str();
    CHECK(text.find("host API 2.3") != std::string::npos);
    CHECK(text.find("rejected sepia: needs API 1.x") != std::string::npos);
    CHECK(text.find("          @@@@@@@@@@\n") != std::string::npos);
    CHECK(text.find("no plugin 'sepia' (rejected: needs API 1.x)") != std::string::npos);
}
