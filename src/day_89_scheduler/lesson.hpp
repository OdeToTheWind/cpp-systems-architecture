/**
 * @file
 * Day 89 – Capstone: Task Scheduler.
 *
 * Scenario: the *maintenance scheduler of a SaaS back end*: refresh caches every 15 minutes, send
 * usage reports hourly at :05, back up the database daily at 02:30. Schedules are written as short
 * specs, time comes from an injectable clock so a whole week can be simulated in milliseconds,
 * a job that runs too long is cut off by its timeout, and a job never overlaps with itself.
 *
 * Deliverables (syllabus):
 * - Schedule specifications
 * - An injectable clock
 * - Timeouts
 * - No overlapping runs
 */
#pragma once

#include <algorithm>
#include <cstdint>
#include <functional>
#include <istream>
#include <limits>
#include <ostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <variant>
#include <vector>

#include "cppm/lesson.hpp"

namespace cppm::day89 {

inline constexpr Deliverable DELIVERABLES[] = {
    {"parsing every / hourly / daily specs", "parse_schedule"},
    {"computing the next run strictly after a time", "next_after"},
    {"a clock the scheduler reads but never owns", "SimulatedClock"},
    {"cutting off runs that exceed their timeout", "Scheduler::run_due"},
    {"skipping a run while the previous one is still busy", "Scheduler::run_until"},
};

using Seconds = std::int64_t;  // seconds since Monday 00:00 of the simulated week
inline constexpr Seconds MINUTE = 60;
inline constexpr Seconds HOUR = 3600;
inline constexpr Seconds DAY = 86400;

struct Every {
    Seconds interval;
};
struct Hourly {
    int minute;
};
struct Daily {
    int hour;
    int minute;
};
using Schedule = std::variant<Every, Hourly, Daily>;

/// "every 15m" / "every 2h" / "hourly :05" / "daily 02:30".
inline Schedule parse_schedule(const std::string& spec) {
    std::istringstream words(spec);
    std::string kind;
    std::string arg;
    std::string extra;
    if (!(words >> kind >> arg) || words >> extra) throw std::invalid_argument("bad schedule '" + spec + "'");
    auto number = [&](const std::string& text, int low, int high) {
        if (text.empty() || text.size() > 4 || text.find_first_not_of("0123456789") != std::string::npos) throw std::invalid_argument("bad schedule '" + spec + "'");
        const int v = std::stoi(text);
        if (v < low || v > high) throw std::invalid_argument("bad schedule '" + spec + "'");
        return v;
    };
    if (kind == "every") {
        const char unit = arg.empty() ? '?' : arg.back();
        const Seconds scale = unit == 's' ? 1 : unit == 'm' ? MINUTE : unit == 'h' ? HOUR : 0;
        if (scale == 0) throw std::invalid_argument("bad schedule '" + spec + "'");
        return Every{number(arg.substr(0, arg.size() - 1), 1, 9999) * scale};
    }
    if (kind == "hourly" && arg.size() == 3 && arg[0] == ':') return Hourly{number(arg.substr(1), 0, 59)};
    if (kind == "daily" && arg.size() == 5 && arg[2] == ':') return Daily{number(arg.substr(0, 2), 0, 23), number(arg.substr(3), 0, 59)};
    throw std::invalid_argument("bad schedule '" + spec + "'");
}

/// The first scheduled time strictly after @p t. "Every" schedules are aligned to multiples of
/// the interval, so restarts do not shift them.
inline Seconds next_after(const Schedule& schedule, Seconds t) {
    struct Visitor {
        Seconds t;
        Seconds operator()(const Every& e) const { return (t / e.interval + 1) * e.interval; }
        Seconds operator()(const Hourly& h) const {
            const Seconds candidate = t - t % HOUR + h.minute * MINUTE;
            return candidate > t ? candidate : candidate + HOUR;
        }
        Seconds operator()(const Daily& d) const {
            const Seconds candidate = t - t % DAY + d.hour * HOUR + d.minute * MINUTE;
            return candidate > t ? candidate : candidate + DAY;
        }
    };
    return std::visit(Visitor{t}, schedule);
}

/// "Tue 02:30" for a simulated time.
inline std::string format_time(Seconds t) {
    static const char* const days[] = {"Mon", "Tue", "Wed", "Thu", "Fri", "Sat", "Sun"};
    const auto day = static_cast<std::size_t>((t / DAY) % 7);
    const auto h = (t % DAY) / HOUR;
    const auto m = (t % HOUR) / MINUTE;
    return std::string(days[day]) + " " + (h < 10 ? "0" : "") + std::to_string(h) + ":" + (m < 10 ? "0" : "") + std::to_string(m);
}

/// The scheduler only *reads* time through this interface; tests and the demo move it by hand.
class SimulatedClock {
public:
    Seconds now() const { return now_; }
    void advance_to(Seconds t) {
        if (t < now_) throw std::logic_error("time cannot go backwards");
        now_ = t;
    }

private:
    Seconds now_ = 0;
};

/// A job reports how long it took (in a real system: measured with the clock).
using Job = std::function<Seconds(Seconds started_at)>;

struct RunRecord {
    std::string job;
    Seconds started;
    std::string outcome;  // "ok", "timeout", "skipped (still running)", "failed: …"
};

class Scheduler {
public:
    explicit Scheduler(SimulatedClock& clock) : clock_(clock) {}

    void add(const std::string& name, const std::string& spec, Job job, Seconds timeout) {
        if (timeout <= 0) throw std::invalid_argument("timeout must be positive");
        const Schedule schedule = parse_schedule(spec);
        tasks_.push_back({name, schedule, std::move(job), timeout, next_after(schedule, clock_.now()), 0});
    }

    /// Run every task that is due at the current time. A task still busy from its previous run
    /// is skipped (never two copies at once); a run longer than its timeout is cut off at the timeout.
    void run_due() {
        const Seconds now = clock_.now();
        for (auto& task : tasks_) {
            if (task.next_run > now) continue;
            task.next_run = next_after(task.schedule, now);
            if (task.busy_until > now) {
                history_.push_back({task.name, now, "skipped (still running)"});
                continue;
            }
            std::string outcome = "ok";
            Seconds took = 0;
            try {
                took = task.job(now);
            } catch (const std::exception& e) {
                outcome = std::string("failed: ") + e.what();
            }
            if (took > task.timeout) {
                outcome = "timeout";
                took = task.timeout;  // the run is killed when the timeout expires
            }
            task.busy_until = now + took;
            history_.push_back({task.name, now, outcome});
        }
    }

    /// Jump the clock from event to event until @p end: nothing happens between scheduled times,
    /// so a simulated week costs a few thousand steps, not 604,800.
    void run_until(Seconds end) {
        while (true) {
            Seconds next = std::numeric_limits<Seconds>::max();
            for (const auto& task : tasks_) next = std::min(next, task.next_run);
            if (next > end) break;
            clock_.advance_to(next);
            run_due();
        }
        clock_.advance_to(std::max(end, clock_.now()));
    }

    const std::vector<RunRecord>& history() const { return history_; }
    int count(const std::string& job, const std::string& outcome) const {
        return static_cast<int>(std::count_if(history_.begin(), history_.end(), [&](const RunRecord& r) { return r.job == job && r.outcome == outcome; }));
    }

private:
    struct Task {
        std::string name;
        Schedule schedule;
        Job job;
        Seconds timeout;
        Seconds next_run;
        Seconds busy_until;
    };
    SimulatedClock& clock_;
    std::vector<Task> tasks_;
    std::vector<RunRecord> history_;
};

/// The interactive demo: "add <name> <took_s> <timeout_s> <spec…>" then "run <hours>" to simulate.
inline int run(std::istream& in, std::ostream& out) {
    out << "Day 89 – Capstone: Task Scheduler\n";
    SimulatedClock clock;
    Scheduler scheduler(clock);
    std::size_t shown = 0;
    while (auto line = prompt_line(in, out, "add <name> <took_s> <timeout_s> <spec> | run <hours>> ")) {
        std::istringstream words(*line);
        std::string command;
        if (!(words >> command)) break;
        try {
            if (command == "add") {
                std::string name;
                Seconds took = 0;
                Seconds timeout = 0;
                std::string spec;
                words >> name >> took >> timeout;
                std::getline(words >> std::ws, spec);
                scheduler.add(name, spec, [took](Seconds) { return took; }, timeout);
            } else if (command == "run") {
                int hours = 0;
                words >> hours;
                scheduler.run_until(clock.now() + hours * HOUR);
                for (; shown < scheduler.history().size(); ++shown) {
                    const auto& r = scheduler.history()[shown];
                    out << "  " << format_time(r.started) << ' ' << r.job << ": " << r.outcome << '\n';
                }
            }
        } catch (const std::exception& error) {
            out << "  " << error.what() << '\n';
        }
    }
    return 0;
}

}  // namespace cppm::day89
