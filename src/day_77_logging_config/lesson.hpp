/**
 * @file
 * Day 77 – Logging & Configuration.
 *
 * Scenario: a *payment gateway* that must be tuned per environment without recompiling. Settings
 * come from an INI file and can be overridden by environment variables in production; log
 * messages have levels, go through a formatter (human text or key=value for log shippers) and are
 * written to any number of sinks, each with its own minimum level.
 *
 * Deliverables (syllabus):
 * - Log levels
 * - Formatters and sinks
 * - INI-style configuration
 * - Environment overrides
 */
#pragma once

#include <algorithm>
#include <cctype>
#include <functional>
#include <istream>
#include <map>
#include <memory>
#include <optional>
#include <ostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "cppm/lesson.hpp"

namespace cppm::day77 {

inline constexpr Deliverable DELIVERABLES[] = {
    {"ordered log levels parsed from text", "parse_level"},
    {"text and key=value formatters", "Formatter"},
    {"sinks with their own minimum level", "Sink"},
    {"a logger that fans records out to sinks", "Logger::log"},
    {"parsing INI sections, keys and comments", "parse_ini"},
    {"environment variables overriding file settings", "Config::apply_env"},
};

enum class Level { debug, info, warning, error };

inline std::string_view level_name(Level level) {
    constexpr std::string_view names[] = {"DEBUG", "INFO", "WARNING", "ERROR"};
    return names[static_cast<int>(level)];
}

inline std::string upper(std::string text) {
    std::transform(text.begin(), text.end(), text.begin(), [](unsigned char c) { return static_cast<char>(std::toupper(c)); });
    return text;
}

inline Level parse_level(const std::string& text) {
    const std::string name = upper(text);
    for (const Level level : {Level::debug, Level::info, Level::warning, Level::error}) {
        if (level_name(level) == name) return level;
    }
    if (name == "WARN") return Level::warning;
    throw std::invalid_argument("unknown log level '" + text + "'");
}

struct Record {
    Level level;
    std::string time;  // supplied by an injected clock
    std::string logger;
    std::string message;
    std::vector<std::pair<std::string, std::string>> fields;
};

// ---- Formatters turn a record into one line ----
class Formatter {
public:
    virtual ~Formatter() = default;
    virtual std::string format(const Record& record) const = 0;
};

class TextFormatter : public Formatter {
public:
    std::string format(const Record& r) const override {
        std::string line = r.time + " " + std::string(level_name(r.level)) + " [" + r.logger + "] " + r.message;
        for (const auto& [key, value] : r.fields) line += " " + key + "=" + value;
        return line;
    }
};

/// logfmt style: easy to grep and to parse by log shippers. Values with spaces are quoted.
class KeyValueFormatter : public Formatter {
public:
    std::string format(const Record& r) const override {
        std::string line = "time=" + r.time + " level=" + upper(std::string(level_name(r.level))) + " logger=" + r.logger +
                           " msg=" + quote(r.message);
        for (const auto& [key, value] : r.fields) line += " " + key + "=" + quote(value);
        return line;
    }

private:
    static std::string quote(const std::string& value) {
        if (value.find_first_of(" \"=") == std::string::npos && !value.empty()) return value;
        std::string out = "\"";
        for (const char c : value) out += c == '"' ? std::string("\\\"") : std::string(1, c);
        return out + "\"";
    }
};

// ---- Sinks decide where lines go and filter by level ----
class Sink {
public:
    Sink(Level minimum, std::shared_ptr<const Formatter> formatter) : minimum_(minimum), formatter_(std::move(formatter)) {}
    virtual ~Sink() = default;
    void accept(const Record& record) {
        if (record.level >= minimum_) write(formatter_->format(record));
    }

protected:
    virtual void write(const std::string& line) = 0;

private:
    Level minimum_;
    std::shared_ptr<const Formatter> formatter_;
};

class StreamSink : public Sink {
public:
    StreamSink(std::ostream& out, Level minimum, std::shared_ptr<const Formatter> formatter)
        : Sink(minimum, std::move(formatter)), out_(out) {}

protected:
    void write(const std::string& line) override { out_ << line << '\n'; }

private:
    std::ostream& out_;
};

/// Keeps lines in memory – for tests, or for an in-app "recent events" panel.
class MemorySink : public Sink {
public:
    using Sink::Sink;
    const std::vector<std::string>& lines() const { return lines_; }

protected:
    void write(const std::string& line) override { lines_.push_back(line); }

private:
    std::vector<std::string> lines_;
};

class Logger {
public:
    using Clock = std::function<std::string()>;
    Logger(std::string name, Clock clock) : name_(std::move(name)), clock_(std::move(clock)) {}
    void add_sink(std::shared_ptr<Sink> sink) { sinks_.push_back(std::move(sink)); }
    void log(Level level, const std::string& message, std::vector<std::pair<std::string, std::string>> fields = {}) {
        const Record record{level, clock_(), name_, message, std::move(fields)};
        for (const auto& sink : sinks_) sink->accept(record);
    }
    void info(const std::string& message) { log(Level::info, message); }
    void error(const std::string& message) { log(Level::error, message); }

private:
    std::string name_;
    Clock clock_;
    std::vector<std::shared_ptr<Sink>> sinks_;
};

// ---- Configuration ----
class Config {
public:
    /// Values are stored as text under "section.key".
    void set(const std::string& key, std::string value) { values_[key] = std::move(value); }
    std::optional<std::string> get(const std::string& key) const {
        const auto it = values_.find(key);
        return it == values_.end() ? std::nullopt : std::optional<std::string>(it->second);
    }
    std::string get_or(const std::string& key, const std::string& fallback) const { return get(key).value_or(fallback); }
    int get_int(const std::string& key, int fallback) const {
        const auto text = get(key);
        if (!text) return fallback;
        std::size_t used = 0;
        int value = 0;
        try {
            value = std::stoi(*text, &used);
        } catch (const std::exception&) {
            used = 0;
        }
        if (used == 0 || used != text->size()) throw std::invalid_argument(key + " must be an integer, got '" + *text + "'");
        return value;
    }
    bool get_bool(const std::string& key, bool fallback) const {
        const auto text = get(key);
        if (!text) return fallback;
        const std::string v = upper(*text);
        if (v == "TRUE" || v == "YES" || v == "ON" || v == "1") return true;
        if (v == "FALSE" || v == "NO" || v == "OFF" || v == "0") return false;
        throw std::invalid_argument(key + " must be a boolean, got '" + *text + "'");
    }
    /// PAYGATE_DATABASE_POOL_SIZE overrides database.pool_size: environment beats file, so the
    /// same file works everywhere and secrets need not be in it. Returns the keys overridden.
    std::vector<std::string> apply_env(const std::map<std::string, std::string>& env, const std::string& prefix) {
        std::vector<std::string> overridden;
        for (auto& [key, value] : values_) {
            std::string name = prefix + "_" + upper(key);
            std::replace(name.begin(), name.end(), '.', '_');
            if (const auto it = env.find(name); it != env.end()) {
                value = it->second;
                overridden.push_back(key);
            }
        }
        return overridden;
    }

private:
    std::map<std::string, std::string> values_;
};

inline std::string trim(const std::string& s) {
    const auto first = s.find_first_not_of(" \t\r");
    return first == std::string::npos ? "" : s.substr(first, s.find_last_not_of(" \t\r") - first + 1);
}

/// [section] headers, key = value lines, ; or # comments (whole-line or after whitespace). Errors name the line number.
inline Config parse_ini(std::istream& in) {
    Config config;
    std::string section;
    int number = 0;
    for (std::string raw; std::getline(in, raw);) {
        ++number;
        const std::string line = trim(raw);
        if (line.empty() || line[0] == ';' || line[0] == '#') continue;
        if (line.front() == '[') {
            if (line.back() != ']') throw std::runtime_error("line " + std::to_string(number) + ": unclosed section header");
            section = trim(line.substr(1, line.size() - 2));
            continue;
        }
        const auto eq = line.find('=');
        if (eq == std::string::npos) throw std::runtime_error("line " + std::to_string(number) + ": expected key = value");
        const std::string key = trim(line.substr(0, eq));
        if (key.empty()) throw std::runtime_error("line " + std::to_string(number) + ": empty key");
        std::string value = line.substr(eq + 1);
        for (const char* marker : {" ;", " #", "\t;", "\t#"}) {  // inline comments need whitespace before them
            if (const auto at = value.find(marker); at != std::string::npos) value.erase(at);
        }
        config.set(section.empty() ? key : section + "." + key, trim(value));
    }
    return config;
}

/// The interactive demo: the gateway starts from a built-in INI and an env override, then
/// logs each "amount currency" payment line.
inline int run(std::istream& in, std::ostream& out) {
    out << "Day 77 – Logging & Configuration\n";
    std::istringstream file("[log]\nlevel = info\nformat = text\n\n[gateway]\nmax_amount = 5000 ; per payment\n"
                            "currency = EUR\n");
    Config config = parse_ini(file);
    for (const auto& key : config.apply_env({{"PAYGATE_GATEWAY_MAX_AMOUNT", "1000"}}, "PAYGATE")) out << "override: " << key << '\n';
    int clock = 0;
    Logger log("gateway", [&clock] { return "t+" + std::to_string(clock++) + "s"; });
    std::shared_ptr<const Formatter> formatter;
    if (config.get_or("log.format", "text") == "kv") formatter = std::make_shared<KeyValueFormatter>();
    else formatter = std::make_shared<TextFormatter>();
    log.add_sink(std::make_shared<StreamSink>(out, parse_level(config.get_or("log.level", "info")), formatter));
    const int limit = config.get_int("gateway.max_amount", 100);
    while (auto line = prompt_line(in, out, "amount currency> ")) {
        std::istringstream words(*line);
        int amount = 0;
        std::string currency;
        if (!(words >> amount >> currency)) break;
        log.log(Level::debug, "received payment");
        if (currency != config.get_or("gateway.currency", "EUR")) {
            log.log(Level::warning, "currency not supported", {{"currency", currency}});
        } else if (amount > limit) {
            log.log(Level::error, "amount over limit", {{"amount", std::to_string(amount)}, {"limit", std::to_string(limit)}});
        } else {
            log.log(Level::info, "payment accepted", {{"amount", std::to_string(amount)}});
        }
    }
    return 0;
}

}  // namespace cppm::day77
