/**
 * @file
 * Day 61 – Notification Automation.
 *
 * Scenario: an *on-call alert bot* for a small web shop. Health readings (disk usage, response
 * latency, queue depth) are compared with warning and critical thresholds; a state change produces
 * a chat-webhook JSON payload, repeats are rate-limited per check, and recoveries are announced.
 * Delivery goes through an injected sink, so a dry run prints what would be posted.
 *
 * Deliverables (syllabus):
 * - Health checks with alert thresholds
 * - Webhook payloads
 * - Rate limiting
 * - Dry-run delivery
 */
#pragma once

#include <chrono>
#include <cstdio>
#include <functional>
#include <istream>
#include <map>
#include <ostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

#include "cppm/lesson.hpp"

namespace cppm::day61 {

inline constexpr Deliverable DELIVERABLES[] = {
    {"comparing a reading with warning and critical thresholds", "evaluate"},
    {"a chat-webhook JSON payload with escaped text", "webhook_payload"},
    {"a per-check cooldown that limits repeated alerts", "Cooldown::allow"},
    {"alerting on state changes and announcing recoveries", "Notifier::observe"},
    {"a dry-run sink that prints instead of posting", "dry_run_sink"},
};

enum class Level { ok, warning, critical };

inline std::string_view to_string(Level level) {
    switch (level) {
        case Level::ok: return "OK";
        case Level::warning: return "WARNING";
        case Level::critical: return "CRITICAL";
    }
    return "?";
}

struct Threshold {
    double warning;
    double critical;
};

/// Higher readings are worse. Thresholds must be ordered: warning <= critical.
inline Level evaluate(double value, const Threshold& threshold) {
    if (threshold.warning > threshold.critical) throw std::invalid_argument("warning threshold above critical");
    if (value >= threshold.critical) return Level::critical;
    if (value >= threshold.warning) return Level::warning;
    return Level::ok;
}

inline std::string json_escape(std::string_view text) {
    std::string out;
    for (const char c : text) {
        switch (c) {
            case '"': out += "\\\""; break;
            case '\\': out += "\\\\"; break;
            case '\n': out += "\\n"; break;
            default:
                if (static_cast<unsigned char>(c) < 0x20) {
                    char buffer[8];
                    std::snprintf(buffer, sizeof buffer, "\\u%04x", static_cast<unsigned>(c));
                    out += buffer;
                } else {
                    out += c;
                }
        }
    }
    return out;
}

struct Alert {
    std::string check;
    Level level;
    double value;
    std::string message;
};

/// {"text": "...", "level": "...", "check": "..."} – the shape most chat webhooks accept.
inline std::string webhook_payload(const Alert& alert) {
    const std::string emoji = alert.level == Level::ok ? ":white_check_mark:" : alert.level == Level::warning ? ":warning:" : ":rotating_light:";
    return R"({"text":")" + json_escape(emoji + " " + alert.message) + R"(","level":")" + std::string(to_string(alert.level)) +
           R"(","check":")" + json_escape(alert.check) + "\"}";
}

/// Allows at most one notification per key per cooldown period.
class Cooldown {
public:
    explicit Cooldown(std::chrono::seconds period) : period_(period) {}
    bool allow(const std::string& key, std::chrono::seconds now) {
        const auto it = last_.find(key);
        if (it != last_.end() && now - it->second < period_) return false;
        last_[key] = now;
        return true;
    }
    void reset(const std::string& key) { last_.erase(key); }

private:
    std::chrono::seconds period_;
    std::map<std::string, std::chrono::seconds> last_;
};

using Sink = std::function<void(const std::string& payload)>;

/// Prints payloads instead of posting them – used with --dry-run and in tests.
inline Sink dry_run_sink(std::ostream& out) {
    return [&out](const std::string& payload) { out << "  [dry-run] POST " << payload << '\n'; };
}

class Notifier {
public:
    Notifier(std::map<std::string, Threshold> thresholds, Sink sink, std::chrono::seconds cooldown)
        : thresholds_(std::move(thresholds)), sink_(std::move(sink)), cooldown_(cooldown) {}

    /// Feed one reading. A changed level always notifies (escalations and recoveries matter);
    /// an unchanged bad level re-notifies only when the cooldown has passed. Returns whether a
    /// payload was sent.
    bool observe(const std::string& check, double value, std::chrono::seconds now) {
        const auto threshold = thresholds_.find(check);
        if (threshold == thresholds_.end()) throw std::invalid_argument("no thresholds for " + check);
        const Level level = evaluate(value, threshold->second);
        const auto previous = state_.contains(check) ? state_[check] : Level::ok;
        state_[check] = level;
        bool send = false;
        if (level != previous) {
            send = true;
            cooldown_.reset(check);
            cooldown_.allow(check, now);
        } else if (level != Level::ok) {
            send = cooldown_.allow(check, now);
        }
        if (!send) {
            ++suppressed_;
            return false;
        }
        std::ostringstream message;
        if (level == Level::ok) {
            message << check << " recovered (" << value << ")";
        } else {
            message << check << " is " << to_string(level) << ": " << value << " (warn " << threshold->second.warning
                    << ", crit " << threshold->second.critical << ")";
        }
        sink_(webhook_payload({check, level, value, message.str()}));
        ++sent_;
        return true;
    }
    int sent() const { return sent_; }
    int suppressed() const { return suppressed_; }

private:
    std::map<std::string, Threshold> thresholds_;
    Sink sink_;
    Cooldown cooldown_;
    std::map<std::string, Level> state_;
    int sent_{0};
    int suppressed_{0};
};

/// The interactive demo: "<minute> <check> <value>" lines; delivery is always a dry run here.
inline int run(std::istream& in, std::ostream& out) {
    out << "Day 61 – Notification Automation\n";
    Notifier notifier({{"disk_pct", {80, 95}}, {"latency_ms", {300, 1000}}, {"queue_depth", {100, 500}}}, dry_run_sink(out),
                      std::chrono::minutes{15});
    while (auto line = prompt_line(in, out, "minute check value> ")) {
        std::istringstream words(*line);
        long long minute = 0;
        std::string check;
        double value = 0;
        if (!(words >> minute >> check >> value)) break;
        try {
            if (!notifier.observe(check, value, std::chrono::minutes{minute})) out << "  (no notification)\n";
        } catch (const std::invalid_argument& error) {
            out << "  " << error.what() << '\n';
        }
    }
    out << notifier.sent() << " sent, " << notifier.suppressed() << " suppressed\n";
    return 0;
}

}  // namespace cppm::day61
