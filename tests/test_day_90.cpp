// Tests for Day 90 – Capstone: Large File Processor. Every chunk size must give the same answer.
#include <algorithm>
#include <filesystem>
#include <fstream>
#include <functional>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

#include "cppm/temp_dir.hpp"
#include "cppm/testing.hpp"
#include "day_90_large_files/lesson.hpp"

using namespace cppm::day90;

namespace {
std::vector<std::string> lines_of(const std::string& text, std::size_t chunk) {
    std::istringstream in(text);
    std::vector<std::string> lines;
    for_each_line(in, chunk, [&lines](std::string_view l) { lines.emplace_back(l); });
    return lines;
}
}  // namespace

TEST_CASE("lines are reassembled across any chunk boundary") {
    const std::string text = "alpha\nbeta gamma\r\n\ndelta";
    const std::vector<std::string> expected{"alpha", "beta gamma", "", "delta"};
    for (std::size_t chunk = 1; chunk <= text.size() + 2; ++chunk) CHECK(lines_of(text, chunk) == expected);
    CHECK(lines_of("", 4).empty());
    CHECK(lines_of("x\n", 1) == std::vector<std::string>{"x"});
    CHECK_THROWS_AS(lines_of("x", 0), std::invalid_argument);
}

TEST_CASE("the assembler only buffers the unfinished line") {
    LineAssembler assembler;
    std::vector<std::string> got;
    auto collect = [&got](std::string_view l) { got.emplace_back(l); };
    assembler.feed("one\ntw", collect);
    assembler.feed("o\nthr", collect);
    assembler.feed("ee", collect);
    assembler.finish(collect);
    CHECK(got == std::vector<std::string>{"one", "two", "three"});
    CHECK_EQ(assembler.max_partial(), 5u);
}

TEST_CASE("streaming statistics do not depend on the chunk size") {
    const std::string log = synthetic_log(2000) + "garbage line\n";
    std::istringstream small(log);
    std::istringstream big(log);
    const auto a = summarise_log(small, 7);
    const auto b = summarise_log(big, 1 << 16);
    CHECK_EQ(a.lines, 2001u);
    CHECK_EQ(a.malformed, 1u);
    CHECK(a.by_status == b.by_status);
    CHECK_EQ(a.total_ms, b.total_ms);
    CHECK_EQ(a.slowest_path, b.slowest_path);
}

TEST_CASE("runs are sorted and bounded in size") {
    cppm::TempDir dir("day90");
    std::istringstream in("5\n3\n9\n1\n7\n");
    const auto runs = write_sorted_runs(in, 2, dir.path(), std::less<std::string>{});
    CHECK_EQ(runs.size(), 3u);
    std::ifstream first(runs[0]);
    std::string a;
    std::string b;
    first >> a >> b;
    CHECK_EQ(a + b, "35");
    std::istringstream empty("");
    CHECK(write_sorted_runs(empty, 2, dir.path(), std::less<std::string>{}).empty());
}

TEST_CASE("external sort equals an in-memory stable sort") {
    cppm::TempDir dir("day90");
    const std::string log = synthetic_log(3000);
    std::istringstream in(log);
    std::ostringstream out;
    const auto report = external_sort(in, out, 128, dir.path() / "tmp", slower_first);
    CHECK_EQ(report.lines, 3000u);
    CHECK_EQ(report.runs, 24u);  // ceil(3000 / 128)
    std::vector<std::string> expected;
    std::istringstream again(log);
    for (std::string l; std::getline(again, l);) expected.push_back(l);
    std::stable_sort(expected.begin(), expected.end(), slower_first);
    std::string joined;
    for (const auto& l : expected) joined += l + "\n";
    CHECK(out.str() == joined);
    CHECK(std::filesystem::is_empty(dir.path() / "tmp"));  // run files cleaned up
}

TEST_CASE("comparators handle odd lines") {
    CHECK(slower_first("a 900", "b 10"));
    CHECK(!slower_first("a 10", "b 900"));
    CHECK(slower_first("a 1", "no number"));
}

TEST_CASE("run analyses and sorts a synthetic log") {
    cppm::TempDir dir("day90");
    std::istringstream in("1000 100 64\nstop\n");
    std::ostringstream out;
    CHECK_EQ(run(in, out, dir.path()), 0);
    CHECK(out.str().find("1000 line(s): 200 x") != std::string::npos);
    CHECK(out.str().find("sorted with 16 run(s); top: ") != std::string::npos);
}
