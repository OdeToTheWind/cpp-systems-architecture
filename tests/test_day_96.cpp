// Tests for Day 96 – Capstone: Packaging a Real Tool. The release files in the source tree are
// checked too, so a forgotten changelog entry or stale docs fail the build's tests.
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

#include "cppm/testing.hpp"
#include "day_96_release_tool/lesson.hpp"

using namespace cppm::day96;

namespace {
std::vector<Release> changelog_from(const std::string& text) {
    std::istringstream in(text);
    return parse_changelog(in);
}
const std::string good_log =
    "## [Unreleased]\n\n## [1.3.0] - 2026-06-16\n### Added\n- count\n\n## [1.2.1] - 2026-05-02\n- fix\n";
}  // namespace

TEST_CASE("the version comes from the build system") {
    CHECK_EQ(std::string(LOGSLICE_VERSION), "1.3.0");
    CHECK(parse_version(LOGSLICE_VERSION).has_value());
    CHECK(!parse_version("1.3").has_value());
    CHECK(!parse_version("1.3.0-beta").has_value());
}

TEST_CASE("help text and Markdown docs come from the same option table") {
    const auto help = usage_text();
    const auto docs = markdown_docs();
    for (const auto& o : OPTIONS) {
        CHECK(help.find(o.flag) != std::string::npos);
        CHECK(docs.find(std::string("| `") + o.flag + "`") != std::string::npos);
    }
    CHECK(help.find("  --from HH:MM        first time to include") != std::string::npos);
}

TEST_CASE("changelogs are parsed into releases with entries") {
    const auto releases = changelog_from(good_log);
    CHECK_EQ(releases.size(), 3u);
    CHECK_EQ(releases[1].version, "1.3.0");
    CHECK_EQ(releases[1].date, "2026-06-16");
    CHECK(releases[1].entries == std::vector<std::string>{"count"});
    CHECK(releases[0].entries.empty());
}

TEST_CASE("only single semantic-version steps are valid bumps") {
    CHECK(valid_bump({1, 2, 1}, {1, 3, 0}));
    CHECK(valid_bump({1, 2, 1}, {1, 2, 2}));
    CHECK(valid_bump({1, 2, 1}, {2, 0, 0}));
    CHECK(!valid_bump({1, 2, 1}, {1, 4, 0}));
    CHECK(!valid_bump({1, 2, 1}, {1, 3, 1}));
    CHECK(!valid_bump({1, 2, 1}, {1, 2, 1}));
}

TEST_CASE("pre-flight lists every blocker") {
    CHECK(preflight("1.3.0", changelog_from(good_log), markdown_docs()).empty());
    const auto bad = preflight(
        "1.4.0", changelog_from("## [Unreleased]\n- wip\n## [1.3.0] - 2026-06-16\n## [1.1.0] - 2026-07-01\n- x\n"),
        "old docs");
    CHECK(bad == std::vector<std::string>{"changelog has unreleased entries – move them under 1.4.0",
                                          "newest changelog entry is 1.3.0, binary is 1.4.0",
                                          "release 1.3.0 lists no changes",
                                          "1.1.0 -> 1.3.0 is not a single semantic-version step",
                                          "dates go backwards at 1.3.0", "USAGE.md is out of date – regenerate it"});
}

TEST_CASE("the real release files pass pre-flight") {
    std::ifstream changelog(std::string(LOGSLICE_SOURCE_DIR) + "/release/CHANGELOG.md");
    CHECK(changelog.good());
    const auto problems = preflight(LOGSLICE_VERSION, parse_changelog(changelog),
                                    read_file(std::string(LOGSLICE_SOURCE_DIR) + "/release/USAGE.md"));
    for (const auto& p : problems) CHECK_EQ(p, "");  // print each blocker if any
    CHECK(problems.empty());
}

TEST_CASE("the tool slices by time and level, with exit codes") {
    const std::string log =
        "09:58:01 INFO start\n10:00:05 ERROR disk full\n  at write()\n10:02:00 INFO ok\n10:07:30 ERROR timeout\n";
    std::istringstream in1(log);
    std::ostringstream out;
    std::ostringstream err;
    CHECK_EQ(logslice({"--from", "10:00", "--until", "10:02"}, in1, out, err), 0);
    CHECK_EQ(out.str(), "10:00:05 ERROR disk full\n  at write()\n10:02:00 INFO ok\n");
    std::istringstream in2(log);
    std::ostringstream count;
    logslice({"--level", "ERROR", "--count"}, in2, count, err);
    CHECK_EQ(count.str(), "2\n");
    std::istringstream in3(log);
    std::ostringstream bad;
    CHECK_EQ(logslice({"--from"}, in3, out, bad), 2);
    CHECK(bad.str().find("--from needs a value") != std::string::npos);
    std::istringstream demo("check\nslice --version\nstop\n");
    std::ostringstream shown;
    CHECK_EQ(run(demo, shown), 0);
    CHECK(shown.str().find("pre-flight passed: ready to tag v1.3.0") != std::string::npos);
    CHECK(shown.str().find("logslice 1.3.0\n") != std::string::npos);
}
