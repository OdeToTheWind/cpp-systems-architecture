// Tests for Day 86 – Capstone: Logging & Monitoring Toolkit.
#include <filesystem>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

#include "cppm/temp_dir.hpp"
#include "cppm/testing.hpp"
#include "day_86_monitoring/lesson.hpp"

using namespace cppm::day86;

TEST_CASE("log lines are single-line JSON with typed fields") {
    const auto line = json_log_line("2026-06-05T10:00:00Z", "info", "redirect \"x\"\nnext",
                                    {{"code", "abc"}, {"ms", "12.5"}, {"user", "-"}});
    CHECK_EQ(
        line,
        R"({"ts":"2026-06-05T10:00:00Z","level":"info","msg":"redirect \"x\"\nnext","code":"abc","ms":12.5,"user":"-"})");
    CHECK(line.find('\n') == std::string::npos);
}

TEST_CASE("files rotate by size and keep a bounded history") {
    cppm::TempDir dir("day86");
    const auto log = dir.path() / "app.log";
    RotatingFile file(log, 100, 2);
    for (int i = 0; i < 20; ++i) file.write("line " + std::to_string(i) + " " + std::string(20, '.'));
    CHECK(file.rotations() > 2);
    CHECK(std::filesystem::exists(log.string() + ".1"));
    CHECK(std::filesystem::exists(log.string() + ".2"));
    CHECK(!std::filesystem::exists(log.string() + ".3"));
    CHECK(std::filesystem::file_size(log) <= 100u);
    std::ifstream current(log);
    std::string last;
    for (std::string l; std::getline(current, l);) last = l;
    CHECK(last.rfind("line 19 ", 0) == 0);  // newest line is in the live file, whole
    CHECK_THROWS_AS(RotatingFile(log, 0, 1), std::invalid_argument);
}

TEST_CASE("counters and gauges are exposed in Prometheus format") {
    Registry r;
    r.counter_add("links_created_total", "Links created.");
    r.counter_add("links_created_total", "Links created.", {}, 2);
    r.counter_add("redirects_total", "Redirects.", {{"code", "a1"}});
    r.gauge_set("db_connections", "Open connections.", 7);
    CHECK_EQ(r.value("links_created_total"), 3.0);
    CHECK_EQ(r.expose(),
             "# HELP db_connections Open connections.\n# TYPE db_connections gauge\ndb_connections 7\n"
             "# HELP links_created_total Links created.\n# TYPE links_created_total counter\nlinks_created_total 3\n"
             "# HELP redirects_total Redirects.\n# TYPE redirects_total counter\nredirects_total{code=\"a1\"} 1\n");
    CHECK_THROWS_AS(r.counter_add("links_created_total", "x", {}, -1), std::invalid_argument);
    CHECK_THROWS_AS(r.gauge_set("links_created_total", "x", 1), std::logic_error);
}

TEST_CASE("histogram buckets are cumulative") {
    Registry r;
    for (const double ms : {20.0, 80.0, 80.0, 400.0}) r.observe("latency_ms", "Latency.", ms, {50, 100});
    CHECK_EQ(r.histogram_value("latency_ms", "_bucket{le=\"50\"}"), 1.0);
    CHECK_EQ(r.histogram_value("latency_ms", "_bucket{le=\"100\"}"), 3.0);
    CHECK_EQ(r.histogram_value("latency_ms", "_count"), 4.0);
    const auto text = r.expose();
    CHECK(
        text.find("latency_ms_bucket{le=\"50\"} 1\nlatency_ms_bucket{le=\"100\"} 3\nlatency_ms_bucket{le=\"+Inf\"} 4\n"
                  "latency_ms_sum 580\nlatency_ms_count 4\n") != std::string::npos);
}

TEST_CASE("alerts fire after their duration and resolve once") {
    Registry r;
    bool bad = false;
    Alerter alerter({{"Bad", [&bad](const Registry&) { return bad; }, 3}});
    bad = true;
    CHECK(alerter.evaluate(r).empty());  // pending 1
    CHECK(alerter.evaluate(r).empty());  // pending 2
    CHECK(alerter.evaluate(r) == std::vector<std::string>{"FIRING Bad"});
    CHECK(alerter.evaluate(r).empty());  // still firing: no repeat
    CHECK(alerter.firing("Bad"));
    bad = false;
    CHECK(alerter.evaluate(r) == std::vector<std::string>{"RESOLVED Bad"});
    bad = true;
    CHECK(alerter.evaluate(r).empty());  // a single blip does not fire
}

TEST_CASE("the error ratio counts only 5xx responses") {
    Registry r;
    CHECK_EQ(error_ratio(r), 0.0);
    r.counter_add("http_requests_total", "h", {{"status", "2xx"}}, 90);
    r.counter_add("http_requests_total", "h", {{"status", "5xx"}}, 10);
    CHECK_NEAR(error_ratio(r), 0.1, 1e-12);
}

TEST_CASE("run alerts on a sustained error burst") {
    cppm::TempDir dir("day86");
    std::istringstream in("100 0 40\n90 10 300\n80 20 900\n100 1 60\nstop\n");
    std::ostringstream out;
    CHECK_EQ(run(in, out, dir.path() / "s.log"), 0);
    const auto text = out.str();
    CHECK(text.find("ALERT FIRING HighErrorRate") < text.find("ALERT RESOLVED HighErrorRate"));
    CHECK(text.find("http_requests_total{status=\"5xx\"} 31") != std::string::npos);
    std::ifstream log(dir.path() / "s.log");
    std::string first;
    std::getline(log, first);
    CHECK_EQ(first, R"({"ts":"2026-06-05T10:00:00Z","level":"info","msg":"interval","ok":100,"errors":0})");
}
