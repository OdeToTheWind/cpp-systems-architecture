// Tests for Day 89 – Capstone: Task Scheduler. A simulated clock makes a week of scheduling instant.
#include <sstream>
#include <stdexcept>
#include <string>
#include <variant>

#include "cppm/testing.hpp"
#include "day_89_scheduler/lesson.hpp"

using namespace cppm::day89;

TEST_CASE("schedule specs are parsed strictly") {
    CHECK_EQ(std::get<Every>(parse_schedule("every 15m")).interval, 900);
    CHECK_EQ(std::get<Every>(parse_schedule("every 2h")).interval, 7200);
    CHECK_EQ(std::get<Hourly>(parse_schedule("hourly :05")).minute, 5);
    CHECK_EQ(std::get<Daily>(parse_schedule("daily 02:30")).hour, 2);
    for (const char* bad :
         {"every 0m", "every 5d", "hourly 5", "daily 24:00", "daily 2:30", "weekly mon", "every 5m now", ""}) {
        CHECK_THROWS_AS(parse_schedule(bad), std::invalid_argument);
    }
}

TEST_CASE("next runs are strictly in the future and aligned") {
    CHECK_EQ(next_after(Every{900}, 0), 900);
    CHECK_EQ(next_after(Every{900}, 900), 1800);   // strictly after
    CHECK_EQ(next_after(Every{900}, 1000), 1800);  // aligned to the interval, not to "now"
    CHECK_EQ(format_time(next_after(Hourly{5}, 3 * HOUR + 10 * MINUTE)), "Mon 04:05");
    CHECK_EQ(format_time(next_after(Daily{2, 30}, 2 * HOUR)), "Mon 02:30");
    CHECK_EQ(format_time(next_after(Daily{2, 30}, 3 * HOUR)), "Tue 02:30");
    CHECK_EQ(format_time(6 * DAY + 23 * HOUR + 59 * MINUTE), "Sun 23:59");
}

TEST_CASE("the clock only moves forward") {
    SimulatedClock clock;
    clock.advance_to(10);
    CHECK_EQ(clock.now(), 10);
    CHECK_THROWS_AS(clock.advance_to(5), std::logic_error);
}

TEST_CASE("a simulated week runs each job the right number of times") {
    SimulatedClock clock;
    Scheduler scheduler(clock);
    scheduler.add("cache", "every 15m", [](Seconds) { return Seconds{20}; }, 60);
    scheduler.add("report", "hourly :05", [](Seconds) { return Seconds{30}; }, 300);
    scheduler.add("backup", "daily 02:30", [](Seconds) { return Seconds{600}; }, 3600);
    scheduler.run_until(7 * DAY);
    CHECK_EQ(scheduler.count("cache", "ok"), 7 * 96);
    CHECK_EQ(scheduler.count("report", "ok"), 7 * 24);
    CHECK_EQ(scheduler.count("backup", "ok"), 7);
    CHECK_EQ(clock.now(), 7 * DAY);
}

TEST_CASE("long runs time out and later runs never overlap") {
    SimulatedClock clock;
    Scheduler scheduler(clock);
    int calls = 0;
    scheduler.add(
        "reindex", "every 10m", [&calls](Seconds) { return ++calls == 1 ? Seconds{25 * MINUTE} : Seconds{60}; },
        25 * MINUTE);
    scheduler.run_until(HOUR);
    const auto& h = scheduler.history();
    CHECK_EQ(h[0].outcome, "ok");  // exactly at the timeout is allowed
    CHECK_EQ(h[1].outcome, "skipped (still running)");
    CHECK_EQ(h[2].outcome, "skipped (still running)");
    CHECK_EQ(h[3].outcome, "ok");  // 00:40, the first run ended at 00:35
    CHECK_EQ(calls, 4);            // 00:10, 00:40, 00:50, 01:00
    SimulatedClock c2;
    Scheduler s2(c2);
    s2.add("slow", "every 1h", [](Seconds) { return Seconds{3 * HOUR}; }, 30 * MINUTE);
    s2.run_until(3 * HOUR);
    CHECK_EQ(s2.count("slow", "timeout"), 3);  // killed after 30 min, so the next hour can start
}

TEST_CASE("failing jobs are recorded and do not stop the scheduler") {
    SimulatedClock clock;
    Scheduler scheduler(clock);
    scheduler.add(
        "flaky", "every 1h",
        [](Seconds t) -> Seconds {
            if (t == 2 * HOUR) throw std::runtime_error("disk full");
            return 1;
        },
        60);
    scheduler.run_until(3 * HOUR);
    CHECK_EQ(scheduler.history()[1].outcome, "failed: disk full");
    CHECK_EQ(scheduler.count("flaky", "ok"), 2);
    CHECK_THROWS_AS(scheduler.add("x", "every 1h", [](Seconds) { return Seconds{0}; }, 0), std::invalid_argument);
}

TEST_CASE("run simulates and prints the history") {
    std::istringstream in(
        "add backup 600 3600 daily 02:30\nadd sync 4000 3600 hourly :00\nadd x 1 1 weekly\nrun 4\nstop\n");
    std::ostringstream out;
    CHECK_EQ(run(in, out), 0);
    const auto text = out.str();
    CHECK(text.find("bad schedule 'weekly'") != std::string::npos);
    CHECK(text.find("Mon 01:00 sync: timeout") != std::string::npos);
    CHECK(text.find("Mon 02:30 backup: ok") != std::string::npos);
    CHECK(text.find("Mon 04:00 sync: timeout") != std::string::npos);
}
