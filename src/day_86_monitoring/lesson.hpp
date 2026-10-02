/**
 * @file
 * Day 86 – Capstone: Logging & Monitoring Toolkit.
 *
 * Scenario: the *observability layer of a URL shortener*. Every request is logged as one JSON
 * line (easy for log shippers), log files rotate by size so the disk never fills, request counts
 * and latencies are exposed in the Prometheus text format, and alert rules fire only after a
 * problem persists for several evaluations – and resolve on their own when it goes away.
 *
 * Deliverables (syllabus):
 * - Structured JSON logs
 * - Size-based log rotation
 * - Prometheus text exposition
 * - Alert rules
 */
#pragma once

#include <algorithm>
#include <cstdint>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <functional>
#include <istream>
#include <map>
#include <ostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include "cppm/lesson.hpp"

namespace cppm::day86 {

namespace fs = std::filesystem;

inline constexpr Deliverable DELIVERABLES[] = {
    {"one JSON object per log line", "json_log_line"},
    {"rotating log files by size", "RotatingFile::write"},
    {"counters, gauges and histograms in one registry", "Registry"},
    {"the Prometheus text exposition format", "Registry::expose"},
    {"alerts that fire after a duration and resolve", "Alerter::evaluate"},
};

inline std::string json_escape(const std::string& s) {
    std::string out;
    for (const char c : s) {
        switch (c) {
            case '"': out += "\\\""; break;
            case '\\': out += "\\\\"; break;
            case '\n': out += "\\n"; break;
            case '\t': out += "\\t"; break;
            default:
                if (static_cast<unsigned char>(c) < 0x20) {
                    char u[8];
                    std::snprintf(u, sizeof u, "\\u%04x", static_cast<unsigned>(c));
                    out += u;
                } else {
                    out += c;
                }
        }
    }
    return out;
}

/// {"ts":"…","level":"…","msg":"…",<fields>} – fields keep their order; numbers stay unquoted.
inline std::string json_log_line(const std::string& ts, const std::string& level, const std::string& message,
                                 const std::vector<std::pair<std::string, std::string>>& fields = {}) {
    std::string line = "{\"ts\":\"" + json_escape(ts) + "\",\"level\":\"" + json_escape(level) + "\",\"msg\":\"" + json_escape(message) + "\"";
    for (const auto& [key, value] : fields) {
        const bool number = !value.empty() && value.find_first_not_of("0123456789.-") == std::string::npos && value != "-" && value != ".";
        line += ",\"" + json_escape(key) + "\":" + (number ? value : "\"" + json_escape(value) + "\"");
    }
    return line + "}";
}

/// app.log grows to max_bytes, then app.log -> app.log.1 -> app.log.2 …; only @p keep old files remain.
class RotatingFile {
public:
    RotatingFile(fs::path path, std::uintmax_t max_bytes, int keep) : path_(std::move(path)), max_bytes_(max_bytes), keep_(keep) {
        if (max_bytes == 0 || keep < 0) throw std::invalid_argument("bad rotation settings");
    }
    void write(const std::string& line) {
        const std::uintmax_t current = fs::exists(path_) ? fs::file_size(path_) : 0;
        if (current > 0 && current + line.size() + 1 > max_bytes_) rotate();  // never split a line across files
        std::ofstream(path_, std::ios::binary | std::ios::app) << line << '\n';
    }
    int rotations() const { return rotations_; }

private:
    fs::path numbered(int n) const { return path_.string() + "." + std::to_string(n); }
    void rotate() {
        std::error_code ignored;
        fs::remove(numbered(keep_), ignored);  // the oldest falls off the end
        for (int n = keep_ - 1; n >= 1; --n) {
            if (fs::exists(numbered(n))) fs::rename(numbered(n), numbered(n + 1));
        }
        if (keep_ > 0) fs::rename(path_, numbered(1));
        else fs::remove(path_);
        ++rotations_;
    }
    fs::path path_;
    std::uintmax_t max_bytes_;
    int keep_;
    int rotations_ = 0;
};

/// Metric values keyed by name and a rendered label set such as {route="/r",status="200"}.
class Registry {
public:
    using Labels = std::vector<std::pair<std::string, std::string>>;

    void counter_add(const std::string& name, const std::string& help, const Labels& labels = {}, double by = 1) {
        if (by < 0) throw std::invalid_argument("counters only go up");
        declare(name, help, "counter")[render(labels)] += by;
    }
    void gauge_set(const std::string& name, const std::string& help, double value, const Labels& labels = {}) {
        declare(name, help, "gauge")[render(labels)] = value;
    }
    /// Histogram with cumulative buckets: each `le` bucket counts observations <= its bound.
    void observe(const std::string& name, const std::string& help, double value, const std::vector<double>& bounds) {
        auto& series = declare(name, help, "histogram");
        for (const double b : bounds) {
            if (value <= b) series["_bucket{le=\"" + number(b) + "\"}"] += 1;
            else series["_bucket{le=\"" + number(b) + "\"}"] += 0;
        }
        series["_bucket{le=\"+Inf\"}"] += 1;
        series["_sum"] += value;
        series["_count"] += 1;
    }
    double value(const std::string& name, const Labels& labels = {}) const {
        const auto m = metrics_.find(name);
        if (m == metrics_.end()) return 0;
        const auto s = m->second.series.find(render(labels));
        return s == m->second.series.end() ? 0 : s->second;
    }
    double histogram_value(const std::string& name, const std::string& suffix) const {
        const auto m = metrics_.find(name);
        if (m == metrics_.end()) return 0;
        const auto s = m->second.series.find(suffix);
        return s == m->second.series.end() ? 0 : s->second;
    }

    /// What /metrics returns: # HELP and # TYPE lines, then one sample per series.
    std::string expose() const {
        std::string out;
        for (const auto& [name, metric] : metrics_) {
            out += "# HELP " + name + " " + metric.help + "\n# TYPE " + name + " " + metric.type + "\n";
            if (metric.type == "histogram") {
                // Keys sort as text ("100" < "50"), so order the finite buckets by their numeric bound.
                std::vector<std::pair<double, std::string>> buckets;
                for (const auto& [key, v] : metric.series) {
                    if (key.starts_with("_bucket") && key.find("+Inf") == std::string::npos) {
                        buckets.emplace_back(std::stod(key.substr(key.find('"') + 1)), name + key + " " + number(v) + "\n");
                    }
                }
                std::sort(buckets.begin(), buckets.end());
                for (const auto& [bound, sample] : buckets) out += sample;
                out += name + "_bucket{le=\"+Inf\"} " + number(metric.series.at("_bucket{le=\"+Inf\"}")) + "\n";
                out += name + "_sum " + number(metric.series.at("_sum")) + "\n" + name + "_count " + number(metric.series.at("_count")) + "\n";
            } else {
                for (const auto& [labels, v] : metric.series) out += name + labels + " " + number(v) + "\n";
            }
        }
        return out;
    }

private:
    struct Metric {
        std::string help;
        std::string type;
        std::map<std::string, double> series;
    };
    std::map<std::string, double>& declare(const std::string& name, const std::string& help, const std::string& type) {
        auto [it, inserted] = metrics_.try_emplace(name, Metric{help, type, {}});
        if (!inserted && it->second.type != type) throw std::logic_error(name + " is already a " + it->second.type);
        return it->second.series;
    }
    static std::string render(const Labels& labels) {
        if (labels.empty()) return "";
        std::string out = "{";
        for (std::size_t i = 0; i < labels.size(); ++i) out += (i ? "," : "") + labels[i].first + "=\"" + labels[i].second + "\"";
        return out + "}";
    }
    static std::string number(double v) {
        char text[32];
        std::snprintf(text, sizeof text, "%g", v);
        return text;
    }
    std::map<std::string, Metric> metrics_;  // std::map: stable, sorted output
};

struct AlertRule {
    std::string name;
    std::function<bool(const Registry&)> condition;
    int for_evaluations;  // the condition must hold this many evaluations in a row
};

/// Evaluates rules periodically. States: inactive -> pending -> firing -> (resolved) inactive.
/// Only the transitions to firing and back are notified, so a flapping metric does not page anyone.
class Alerter {
public:
    explicit Alerter(std::vector<AlertRule> rules) : rules_(std::move(rules)) {}
    std::vector<std::string> evaluate(const Registry& registry) {
        std::vector<std::string> notifications;
        for (const auto& rule : rules_) {
            State& state = states_[rule.name];
            if (rule.condition(registry)) {
                ++state.streak;
                if (!state.firing && state.streak >= rule.for_evaluations) {
                    state.firing = true;
                    notifications.push_back("FIRING " + rule.name);
                }
            } else {
                state.streak = 0;
                if (state.firing) {
                    state.firing = false;
                    notifications.push_back("RESOLVED " + rule.name);
                }
            }
        }
        return notifications;
    }
    bool firing(const std::string& name) const {
        const auto it = states_.find(name);
        return it != states_.end() && it->second.firing;
    }

private:
    struct State {
        int streak = 0;
        bool firing = false;
    };
    std::vector<AlertRule> rules_;
    std::map<std::string, State> states_;
};

/// Error ratio over all requests recorded so far.
inline double error_ratio(const Registry& r) {
    const double ok = r.value("http_requests_total", {{"status", "2xx"}});
    const double errors = r.value("http_requests_total", {{"status", "5xx"}});
    return ok + errors == 0 ? 0 : errors / (ok + errors);
}

/// The interactive demo: each line is one evaluation interval, "<ok> <errors> <latency_ms>".
inline int run(std::istream& in, std::ostream& out, const fs::path& log = fs::temp_directory_path() / "cppm-shortener.log") {
    out << "Day 86 – Capstone: Logging & Monitoring Toolkit\n";
    Registry registry;
    RotatingFile file(log, 4096, 3);
    Alerter alerter({{"HighErrorRate", [](const Registry& r) { return error_ratio(r) > 0.05; }, 2}});
    int minute = 0;
    while (auto line = prompt_line(in, out, "ok errors latency_ms> ")) {
        std::istringstream words(*line);
        int ok = 0;
        int errors = 0;
        double latency = 0;
        if (!(words >> ok >> errors >> latency)) break;
        Registry window;  // the rule looks at the latest interval, like rate(...[1m])
        for (Registry* r : {&registry, &window}) {
            r->counter_add("http_requests_total", "Requests by status class.", {{"status", "2xx"}}, ok);
            r->counter_add("http_requests_total", "Requests by status class.", {{"status", "5xx"}}, errors);
        }
        registry.observe("http_request_duration_ms", "Request latency.", latency, {50, 100, 250, 1000});
        const std::string ts = "2026-06-05T10:" + std::string(minute < 10 ? "0" : "") + std::to_string(minute) + ":00Z";
        file.write(json_log_line(ts, errors ? "warn" : "info", "interval", {{"ok", std::to_string(ok)}, {"errors", std::to_string(errors)}}));
        ++minute;
        for (const auto& n : alerter.evaluate(window)) out << "  ALERT " << n << '\n';
    }
    out << registry.expose();
    return 0;
}

}  // namespace cppm::day86
