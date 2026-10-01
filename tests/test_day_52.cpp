// Tests for Day 52 – Local Persistence. Each test uses its own temporary folder.
#include <filesystem>
#include <sstream>
#include <stdexcept>

#include "cppm/temp_dir.hpp"
#include "cppm/testing.hpp"
#include "day_52_persistence/lesson.hpp"

using namespace cppm::day52;

namespace {
GardenState sample() {
    GardenState state;
    state.plants["fern"] = {3, 10};
    state.plants["cactus"] = {21, 1};
    return state;
}
}  // namespace

TEST_CASE("serialise writes a version header and one line per plant") {
    CHECK_EQ(serialise(sample()), "version 2\nplant cactus 21 1\nplant fern 3 10\n");
}

TEST_CASE("state round-trips through serialise and deserialise") {
    CHECK(deserialise(serialise(sample())) == sample());
    CHECK(deserialise("version 2\n").plants.empty());
}

TEST_CASE("version 1 files still load, with the default interval") {
    const auto state = deserialise("version 1\nplant basil 5\n");
    CHECK_EQ(state.plants.at("basil").interval_days, 7);
    CHECK_EQ(state.plants.at("basil").last_watered_day, 5);
}

TEST_CASE("unknown versions and corrupt lines are rejected") {
    CHECK_THROWS_AS(deserialise("version 9\n"), std::runtime_error);
    CHECK_THROWS_AS(deserialise("plants!\n"), std::runtime_error);
    CHECK_THROWS_AS(deserialise("version 2\nplant fern three 10\n"), std::runtime_error);
    CHECK_THROWS_AS(deserialise("version 2\nplant fern 3 10 extra\n"), std::runtime_error);
    CHECK_THROWS_AS(deserialise("version 2\nplant fern 0 10\n"), std::runtime_error);
}

TEST_CASE("atomic saves replace the file and leave no temporary behind") {
    cppm::TempDir dir("day52");
    const auto file = dir.path() / "garden.txt";
    save_atomically(file, sample());
    GardenState changed = sample();
    changed.plants.erase("fern");
    save_atomically(file, changed);
    CHECK(load_or_recover(file).state == changed);
    CHECK(!std::filesystem::exists(file.string() + ".tmp"));
}

TEST_CASE("a missing file loads as an empty garden") {
    cppm::TempDir dir("day52");
    const auto result = load_or_recover(dir.path() / "none.txt");
    CHECK(result.state.plants.empty());
    CHECK(!result.recovered);
}

TEST_CASE("a corrupt file is moved aside, never deleted") {
    cppm::TempDir dir("day52");
    const auto file = dir.write("garden.txt", "version 2\nplant fern 3\n");
    const auto result = load_or_recover(file);
    CHECK(result.recovered);
    CHECK(result.state.plants.empty());
    CHECK(!std::filesystem::exists(file));
    CHECK(std::filesystem::exists(result.quarantined));
}

TEST_CASE("due lists plants whose interval has passed") {
    const auto state = sample();
    CHECK(state.due(12).empty());
    CHECK(state.due(13) == std::vector<std::string>{"fern"});
    CHECK(state.due(22) == std::vector<std::string>{"cactus", "fern"});
}

TEST_CASE("run keeps the garden between two sessions") {
    cppm::TempDir dir("day52");
    const auto file = dir.path() / "garden.txt";
    std::istringstream first("add fern 3\nadd cactus 21\nwater fern 10\nbad\n\n");
    std::ostringstream out1;
    CHECK_EQ(run(first, out1, file), 0);
    std::istringstream second("due 13\n\n");
    std::ostringstream out2;
    run(second, out2, file);
    CHECK(out2.str().find("2 plant(s) loaded") != std::string::npos);
    CHECK(out2.str().find("water fern") != std::string::npos);
    CHECK(out2.str().find("water cactus") == std::string::npos);
}
