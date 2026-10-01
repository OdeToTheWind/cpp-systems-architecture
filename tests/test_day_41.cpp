// Tests for Day 41 – File I/O with fstream. Every file lives in a self-deleting temporary folder.
#include <fstream>
#include <iterator>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

#include "cppm/temp_dir.hpp"
#include "cppm/testing.hpp"
#include "day_41_file_io/lesson.hpp"

using namespace cppm::day41;

TEST_CASE("append mode keeps earlier lines") {
    cppm::TempDir dir("day41");
    const auto log = dir.path() / "events.log";
    append_event(log, "takeoff");
    append_event(log, "hover");
    append_event(log, "land");
    CHECK(read_events(log) == std::vector<std::string>{"takeoff", "hover", "land"});
}

TEST_CASE("reading a missing file is reported, not ignored") {
    cppm::TempDir dir("day41");
    CHECK_THROWS_AS(read_events(dir.path() / "nope.log"), std::runtime_error);
    CHECK_THROWS_AS(read_samples(dir.path() / "nope.bin"), std::runtime_error);
    CHECK_THROWS_AS(measured_size(dir.path() / "nope.bin"), std::runtime_error);
}

TEST_CASE("binary samples round-trip exactly, including negative altitudes") {
    cppm::TempDir dir("day41");
    const auto file = dir.path() / "t.bin";
    const std::vector<Sample> samples{{0, 0, 12'600}, {100, 250, 12'480}, {200, -35, 12'470}, {4'000'000'000u, 2'000'000, 65'535}};
    write_samples(file, samples);
    CHECK(read_samples(file) == samples);
    CHECK_EQ(measured_size(file), samples.size() * record_size);
}

TEST_CASE("the byte layout is little-endian and independent of struct padding") {
    cppm::TempDir dir("day41");
    const auto file = dir.path() / "one.bin";
    write_samples(file, {{0x01020304u, -1, 0x0A0B}});
    std::ifstream in(file, std::ios::binary);
    std::string bytes((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
    REQUIRE_EQ(bytes.size(), 10u);
    CHECK_EQ(static_cast<int>(static_cast<unsigned char>(bytes[0])), 0x04);
    CHECK_EQ(static_cast<int>(static_cast<unsigned char>(bytes[3])), 0x01);
    CHECK_EQ(static_cast<int>(static_cast<unsigned char>(bytes[4])), 0xFF);
    CHECK_EQ(static_cast<int>(static_cast<unsigned char>(bytes[8])), 0x0B);
}

TEST_CASE("a truncated binary file is detected") {
    cppm::TempDir dir("day41");
    const auto file = dir.path() / "cut.bin";
    write_samples(file, {{1, 2, 3}, {4, 5, 6}});
    std::filesystem::resize_file(file, 15);
    CHECK_THROWS_AS(read_samples(file), std::runtime_error);
}

TEST_CASE("writing an empty sample list creates an empty file") {
    cppm::TempDir dir("day41");
    const auto file = dir.path() / "empty.bin";
    write_samples(file, {});
    CHECK(read_samples(file).empty());
    CHECK_EQ(measured_size(file), 0u);
}

TEST_CASE("run records events and samples into the given folder") {
    cppm::TempDir dir("day41");
    std::istringstream in("event motors armed\nsample 0 0 12600\nsample 100 120 12500\nsample oops\nevent landed\n\n");
    std::ostringstream out;
    CHECK_EQ(run(in, out, dir.path()), 0);
    const auto text = out.str();
    CHECK(text.find("2 sample(s), 20 bytes") != std::string::npos);
    CHECK(text.find("2 event line(s) in the log") != std::string::npos);
    CHECK(text.find("sample needs three numbers") != std::string::npos);
    CHECK(read_events(dir.path() / "flight-events.log").front() == "motors armed");
}
