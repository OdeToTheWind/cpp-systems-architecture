// Tests for Day 61 – Notification Automation. A recording sink replaces the webhook; time is passed in.
#include <chrono>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

#include "cppm/testing.hpp"
#include "day_61_notifications/lesson.hpp"

using namespace cppm::day61;
using namespace std::chrono_literals;

namespace {
struct Recorder {
    std::vector<std::string> payloads;
    Sink sink() {
        return [this](const std::string& p) { payloads.push_back(p); };
    }
};
}  // namespace

TEST_CASE("readings are classified against inclusive thresholds") {
    const Threshold disk{80, 95};
    CHECK(evaluate(79.9, disk) == Level::ok);
    CHECK(evaluate(80, disk) == Level::warning);
    CHECK(evaluate(95, disk) == Level::critical);
    CHECK_THROWS_AS(evaluate(1, {10, 5}), std::invalid_argument);
}

TEST_CASE("webhook payloads are valid JSON with escaped text") {
    const auto payload = webhook_payload({"disk \"/\"", Level::critical, 97, "line1\nline2"});
    CHECK(payload.find(R"("level":"CRITICAL")") != std::string::npos);
    CHECK(payload.find(R"("check":"disk \"/\"")") != std::string::npos);
    CHECK(payload.find(R"(line1\nline2)") != std::string::npos);
    CHECK_EQ(json_escape(std::string("\x01", 1)), "\\u0001");
}

TEST_CASE("the cooldown allows one notification per key per period") {
    Cooldown cooldown(600s);
    CHECK(cooldown.allow("a", 0s));
    CHECK(!cooldown.allow("a", 599s));
    CHECK(cooldown.allow("b", 1s));
    CHECK(cooldown.allow("a", 600s));
}

TEST_CASE("unchanged bad states are rate-limited; changes always notify") {
    Recorder recorder;
    Notifier notifier({{"disk", {80, 95}}}, recorder.sink(), 900s);
    CHECK(notifier.observe("disk", 85, 0s));    // ok -> warning
    CHECK(!notifier.observe("disk", 86, 60s));  // still warning, in cooldown
    CHECK(notifier.observe("disk", 96, 120s));  // escalation
    CHECK(!notifier.observe("disk", 97, 300s));
    CHECK(notifier.observe("disk", 97, 1020s));   // cooldown elapsed: reminder
    CHECK(notifier.observe("disk", 50, 1100s));   // recovery
    CHECK(!notifier.observe("disk", 40, 1200s));  // ok stays quiet
    CHECK_EQ(recorder.payloads.size(), 4u);
    CHECK(recorder.payloads.back().find("recovered") != std::string::npos);
    CHECK_EQ(notifier.suppressed(), 3);
}

TEST_CASE("checks are tracked independently and unknown checks are rejected") {
    Recorder recorder;
    Notifier notifier({{"a", {1, 2}}, {"b", {1, 2}}}, recorder.sink(), 900s);
    CHECK(notifier.observe("a", 1, 0s));
    CHECK(notifier.observe("b", 1, 0s));
    CHECK_THROWS_AS(notifier.observe("c", 1, 0s), std::invalid_argument);
}

TEST_CASE("the dry-run sink prints instead of posting") {
    std::ostringstream out;
    Notifier notifier({{"latency", {300, 1000}}}, dry_run_sink(out), 900s);
    notifier.observe("latency", 1500, 0s);
    CHECK(out.str().find("[dry-run] POST {\"text\":") != std::string::npos);
    CHECK(out.str().find("latency is CRITICAL: 1500") != std::string::npos);
}

TEST_CASE("run reports what was sent and suppressed") {
    std::istringstream in("0 disk_pct 85\n5 disk_pct 86\n10 nope 1\n20 disk_pct 20\nstop\n");
    std::ostringstream out;
    CHECK_EQ(run(in, out), 0);
    CHECK(out.str().find("no thresholds for nope") != std::string::npos);
    CHECK(out.str().find("2 sent, 1 suppressed") != std::string::npos);
}
