// Tests for Day 42 – Working with Directories. Everything happens inside a temporary sandbox.
#include <filesystem>
#include <sstream>
#include <stdexcept>
#include <string>

#include "cppm/temp_dir.hpp"
#include "cppm/testing.hpp"
#include "day_42_directories/lesson.hpp"

using namespace cppm::day42;

TEST_CASE("the sandbox resolves inside paths and refuses escapes") {
    cppm::TempDir dir("day42");
    const Sandbox sandbox(dir.path());
    CHECK(sandbox.resolve("inbox/a.jpg").string().find("inbox") != std::string::npos);
    CHECK_NOTHROW(sandbox.resolve("inbox/../library/x.jpg"));
    CHECK_THROWS_AS(sandbox.resolve("../outside.txt"), std::invalid_argument);
    CHECK_THROWS_AS(sandbox.resolve("inbox/../../etc"), std::invalid_argument);
    CHECK_THROWS_AS(sandbox.resolve(dir.path() / "abs.txt"), std::invalid_argument);
    CHECK_NOTHROW(sandbox.resolve("."));  // the root itself is inside
    CHECK_THROWS_AS(Sandbox(dir.path() / "missing"), std::invalid_argument);
}

TEST_CASE("categories come from case-insensitive extensions") {
    CHECK_EQ(category_for("IMG_1.CR2"), "raw");
    CHECK_EQ(category_for("a/b/photo.JpEg"), "jpeg");
    CHECK_EQ(category_for("clip.mov"), "video");
    CHECK_EQ(category_for("IMG_1.xmp"), "sidecar");
    CHECK_EQ(category_for("README"), "other");
}

TEST_CASE("find_files walks every level and can filter") {
    cppm::TempDir dir("day42");
    dir.write("card/DCIM/a.jpg", "x");
    dir.write("card/DCIM/sub/b.JPG", "x");
    dir.write("card/c.cr2", "x");
    CHECK_EQ(find_files(dir.path() / "card").size(), 3u);
    const auto jpegs = find_files(dir.path() / "card", "jpeg");
    REQUIRE_EQ(jpegs.size(), 2u);
    CHECK_EQ(jpegs[0].filename().string(), "a.jpg");
}

TEST_CASE("plan_ingest groups by category and avoids name collisions") {
    cppm::TempDir dir("day42");
    dir.write("inbox/100/IMG_1.jpg", "1");
    dir.write("inbox/101/IMG_1.jpg", "2");
    dir.write("inbox/IMG_1.CR2", "3");
    dir.write("library/jpeg/IMG_1_1.jpg", "already there");
    const Sandbox sandbox(dir.path());
    const auto plan = plan_ingest(sandbox);
    REQUIRE_EQ(plan.size(), 3u);
    std::vector<std::string> targets;
    for (const auto& move : plan) {
        targets.push_back(move.to.lexically_relative(sandbox.root()).generic_string());
    }
    // files are visited in sorted path order: inbox/100/…, inbox/101/…, inbox/IMG_1.CR2
    CHECK_EQ(targets[0], "library/jpeg/IMG_1.jpg");
    CHECK_EQ(targets[1], "library/jpeg/IMG_1_2.jpg");  // IMG_1_1.jpg already exists in the library
    CHECK_EQ(targets[2], "library/raw/IMG_1.CR2");
}

TEST_CASE("apply_plan creates folders and moves every file") {
    cppm::TempDir dir("day42");
    dir.write("inbox/x.mp4", "v");
    dir.write("inbox/y.txt", "t");
    const Sandbox sandbox(dir.path());
    CHECK_EQ(apply_plan(sandbox, plan_ingest(sandbox)), 2);
    CHECK(std::filesystem::exists(dir.path() / "library" / "video" / "x.mp4"));
    CHECK(std::filesystem::exists(dir.path() / "library" / "other" / "y.txt"));
    CHECK(find_files(dir.path() / "inbox").empty());
}

TEST_CASE("apply_plan refuses moves that leave the sandbox") {
    cppm::TempDir dir("day42");
    cppm::TempDir elsewhere("day42-other");
    dir.write("inbox/a.jpg", "a");
    const Sandbox sandbox(dir.path());
    const std::vector<Move> evil{{dir.path() / "inbox" / "a.jpg", elsewhere.path() / "stolen.jpg"}};
    CHECK_THROWS_AS(apply_plan(sandbox, evil), std::invalid_argument);
    CHECK(std::filesystem::exists(dir.path() / "inbox" / "a.jpg"));
}

TEST_CASE("tree lists folders before their contents, sorted") {
    cppm::TempDir dir("day42");
    dir.write("b/two.txt", "");
    dir.write("a/one.txt", "");
    dir.write("c.txt", "");
    CHECK_EQ(tree(dir.path()), "a/\n  one.txt\nb/\n  two.txt\nc.txt\n");
}

TEST_CASE("run shows a dry-run plan and applies it on request") {
    cppm::TempDir dir("day42");
    std::istringstream no("n\n");
    std::ostringstream out;
    CHECK_EQ(run(no, out, dir.path()), 0);
    CHECK(out.str().find("IMG_0001.CR2 -> library/raw/IMG_0001.CR2") != std::string::npos);
    CHECK(out.str().find("Dry run only") != std::string::npos);
    cppm::TempDir dir2("day42");
    std::istringstream yes("y\n");
    std::ostringstream out2;
    run(yes, out2, dir2.path());
    CHECK(out2.str().find("6 file(s) moved") != std::string::npos);
    CHECK(out2.str().find("sidecar/\n  IMG_0001.xmp") != std::string::npos);
}
