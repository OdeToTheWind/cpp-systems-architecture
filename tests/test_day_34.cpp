// Tests for Day 34 – Optional, Required & Default Parameters.
#include <sstream>
#include <stdexcept>

#include "cppm/testing.hpp"
#include "day_34_optional_parameters/lesson.hpp"

using namespace cppm::day34;

TEST_CASE("only the required parameters are needed") {
    const Job job = schedule_job("lint", "clang-format --dry-run");
    CHECK_EQ(job.timeout_minutes, 30);
    CHECK_EQ(job.branch, "*");
    CHECK_EQ(job.retries, project_default_retries);
}

TEST_CASE("nullopt and zero retries mean different things") {
    CHECK_EQ(effective_retries(std::nullopt), 2);
    CHECK_EQ(effective_retries(0), 0);
    CHECK_EQ(schedule_job("deploy", "make deploy", 10, "main", 0).retries, 0);
    CHECK_THROWS_AS(effective_retries(-1), std::invalid_argument);
}

TEST_CASE("defaults are filled from the left: to set retries, pass the earlier ones too") {
    const Job job = schedule_job("docs", "doxygen", 30, std::nullopt, 4);
    CHECK_EQ(job.branch, "*");
    CHECK_EQ(job.retries, 4);
}

TEST_CASE("validation applies to every way of creating a job") {
    CHECK_THROWS_AS(schedule_job("", "x"), std::invalid_argument);
    CHECK_THROWS_AS(schedule_job("slow", "x", 0), std::invalid_argument);
    CHECK_THROWS_AS(schedule_job("flaky", "x", 30, std::nullopt, 9), std::invalid_argument);
    CHECK_THROWS_AS(JobBuilder("x", "y").timeout(999).build(), std::invalid_argument);
}

TEST_CASE("the matrix overload creates one job per environment") {
    const auto jobs = schedule_matrix("tests", "ctest", {"gcc", "clang", "msvc"}, 45);
    REQUIRE_EQ(jobs.size(), 3u);
    CHECK_EQ(jobs[1].name, "tests (clang)");
    CHECK(jobs[2].environment == std::vector<std::string>{"msvc"});
    CHECK_EQ(jobs[0].timeout_minutes, 45);
    CHECK_THROWS_AS(schedule_matrix("tests", "ctest", {}), std::invalid_argument);
}

TEST_CASE("the builder names every optional setting") {
    const Job job = JobBuilder("release", "make dist").only_on("main").retries(1).timeout(90).env("CI=1").build();
    CHECK_EQ(describe(job), "release: `make dist` on main, 90 min, 1 retry");
    CHECK(job.environment == std::vector<std::string>{"CI=1"});
}

TEST_CASE("run schedules typed jobs and rejects bad ones") {
    std::istringstream in("build make\ndeploy ./deploy.sh 15 main 0\nhuge x 500\nodd x - - -3\n\n");
    std::ostringstream out;
    CHECK_EQ(run(in, out), 0);
    const auto text = out.str();
    CHECK(text.find("unit-tests (gcc)") != std::string::npos);
    CHECK(text.find("build: `make` on *, 30 min, 2 retries") != std::string::npos);
    CHECK(text.find("deploy: `./deploy.sh` on main, 15 min, 0 retries") != std::string::npos);
    CHECK(text.find("rejected: timeout must be 1-360 minutes") != std::string::npos);
    CHECK(text.find("rejected: retries cannot be negative") != std::string::npos);
}
