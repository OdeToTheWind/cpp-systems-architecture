// Tests for Day 85 – Capstone: Concurrent File Processor. Archives are built in temporary folders.
#include <filesystem>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

#include "cppm/temp_dir.hpp"
#include "cppm/testing.hpp"
#include "day_85_concurrent_files/lesson.hpp"

using namespace cppm::day85;

namespace {
void make_archive(const cppm::TempDir& dir, int files) {
    for (int i = 0; i < files; ++i) {
        dir.write("photos/" + std::to_string(i % 4) + "/img" + std::to_string(i) + ".raw",
                  std::string(static_cast<std::size_t>(i * 37 + 1), static_cast<char>('A' + i % 26)));
    }
}
}  // namespace

TEST_CASE("CRC-32 matches the standard check value and streams large input") {
    CHECK_EQ(crc32_of("123456789"), 0xCBF43926u);
    CHECK_EQ(crc32_of(""), 0u);
    CHECK_EQ(hex8(crc32_of(std::string(200000, 'x'))), hex8(crc32_of(std::string(200000, 'x'))));
    CHECK(crc32_of(std::string(200000, 'x')) != crc32_of(std::string(199999, 'x') + "y"));
}

TEST_CASE("listing is recursive, sorted and portable") {
    cppm::TempDir dir("day85");
    dir.write("b.txt", "b");
    dir.write("a/z.txt", "z");
    dir.write(MANIFEST_NAME, "ignored");
    std::filesystem::create_directories(dir.path() / "empty");
    CHECK(list_files(dir.path()) == std::vector<std::string>{"a/z.txt", "b.txt"});
}

TEST_CASE("parallel results equal sequential results, in input order") {
    cppm::TempDir dir("day85");
    make_archive(dir, 60);
    const auto files = list_files(dir.path());
    const auto one = checksum_all(dir.path(), files, 1);
    const auto many = checksum_all(dir.path(), files, 8);
    CHECK_EQ(many.size(), 60u);
    bool same = true;
    for (std::size_t i = 0; i < one.size(); ++i)
        same = same && one[i].path == many[i].path && one[i].crc == many[i].crc && one[i].size == many[i].size;
    CHECK(same);
    CHECK_THROWS_AS(checksum_all(dir.path(), files, 0), std::invalid_argument);
}

TEST_CASE("unreadable files are reported, not fatal") {
    cppm::TempDir dir("day85");
    dir.write("ok.txt", "fine");
    const auto results = checksum_all(dir.path(), {"ok.txt", "gone.txt"}, 2);
    CHECK(results[0].error.empty());
    CHECK_EQ(results[1].error, "cannot open");
}

TEST_CASE("manifests round-trip and malformed lines are rejected") {
    std::ostringstream out;
    write_manifest(out, {{"a b/c.jpg", 12, 0xDEADBEEF, ""}, {"skip", 0, 0, "cannot open"}});
    CHECK_EQ(out.str(), "deadbeef 12 a b/c.jpg\n");
    std::istringstream in(out.str());
    const auto entries = read_manifest(in);
    CHECK_EQ(entries.at("a b/c.jpg").crc, 0xDEADBEEFu);
    std::istringstream bad("deadbeef twelve x\n");
    CHECK_THROWS_AS(read_manifest(bad), std::runtime_error);
}

TEST_CASE("verification finds modified, missing and added files") {
    cppm::TempDir dir("day85");
    make_archive(dir, 20);
    CHECK_EQ(create_manifest(dir.path(), 4), 20u);
    std::ifstream first(dir.path() / MANIFEST_NAME);
    const auto manifest = read_manifest(first);
    CHECK(verify(dir.path(), manifest, 4).clean());
    dir.write("photos/1/img5.raw", std::string(186, 'X'));  // same size, different bytes
    std::filesystem::remove(dir.path() / "photos/2/img6.raw");
    dir.write("photos/new.raw", "new");
    const auto report = verify(dir.path(), manifest, 4);
    CHECK(report.modified == std::vector<std::string>{"photos/1/img5.raw"});
    CHECK(report.missing == std::vector<std::string>{"photos/2/img6.raw"});
    CHECK(report.added == std::vector<std::string>{"photos/new.raw"});
    CHECK_EQ(report.ok.size(), 18u);
}

TEST_CASE("run builds, verifies and detects changes") {
    cppm::TempDir dir("day85");
    std::istringstream in("manifest\nverify\ntouch 2026/06/IMG_2.jpg x\nrm 2026/06/IMG_3.jpg\nverify\nstop\n");
    std::ostringstream out;
    CHECK_EQ(run(in, out, dir.path()), 0);
    const auto text = out.str();
    CHECK(text.find("manifest lists 6 file(s)") != std::string::npos);
    CHECK(text.find("ok 6, modified 0, missing 0, added 0 – archive intact") != std::string::npos);
    CHECK(text.find("MODIFIED 2026/06/IMG_2.jpg") != std::string::npos);
    CHECK(text.find("MISSING  2026/06/IMG_3.jpg") != std::string::npos);
}
