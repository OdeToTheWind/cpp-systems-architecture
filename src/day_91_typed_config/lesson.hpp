/**
 * @file
 * Day 91 – Capstone: Type-safe Configuration.
 *
 * Scenario: the settings of a *food-delivery dispatch service*, read from environment variables in
 * every deployment. Instead of scattered getenv calls and string-to-int conversions, one schema
 * binds each variable to a typed struct member with a parser, a default and documentation. Loading
 * reports *all* problems at once, secrets never appear in logs, and the schema can print its own
 * .env.example.
 *
 * Deliverables (syllabus):
 * - Typed settings parsed from the environment
 * - Collecting every error at once
 * - Secret redaction
 * - Self-documenting configuration
 */
#pragma once

#include <algorithm>
#include <cctype>
#include <chrono>
#include <functional>
#include <istream>
#include <map>
#include <optional>
#include <ostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include "cppm/lesson.hpp"

namespace cppm::day91 {

inline constexpr Deliverable DELIVERABLES[] = {
    {"small parsers that throw readable messages", "parse_duration"},
    {"a schema binding variables to struct members", "Schema::field"},
    {"loading that collects every error", "Schema::load"},
    {"a secret type and redacted descriptions", "Schema::describe"},
    {"generating .env.example from the schema", "Schema::env_example"},
};

using Env = std::map<std::string, std::string>;

/// A value that never prints itself.
class Secret {
public:
    Secret() = default;
    explicit Secret(std::string value) : value_(std::move(value)) {}
    const std::string& reveal() const { return value_; }
    bool empty() const { return value_.empty(); }

private:
    std::string value_;
};

// ---- Parsers: text -> value, or an exception whose message ends up in the error list ----
inline int parse_int(const std::string& text, int low, int high) {
    std::size_t used = 0;
    long long v = 0;
    try {
        v = std::stoll(text, &used);
    } catch (const std::exception&) {
        used = 0;
    }
    if (used == 0 || used != text.size()) throw std::invalid_argument("expected an integer, got '" + text + "'");
    if (v < low || v > high) throw std::invalid_argument("must be between " + std::to_string(low) + " and " + std::to_string(high));
    return static_cast<int>(v);
}

inline bool parse_bool(const std::string& text) {
    std::string t = text;
    std::transform(t.begin(), t.end(), t.begin(), [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    if (t == "1" || t == "true" || t == "yes" || t == "on") return true;
    if (t == "0" || t == "false" || t == "no" || t == "off") return false;
    throw std::invalid_argument("expected true/false, got '" + text + "'");
}

/// "500ms", "30s", "5m", "2h" -> milliseconds.
inline std::chrono::milliseconds parse_duration(const std::string& text) {
    std::size_t digits = 0;
    while (digits < text.size() && std::isdigit(static_cast<unsigned char>(text[digits]))) ++digits;
    const std::string unit = text.substr(digits);
    if (digits == 0 || digits > 9) throw std::invalid_argument("expected a duration like 30s, got '" + text + "'");
    const long long n = std::stoll(text.substr(0, digits));
    if (unit == "ms") return std::chrono::milliseconds(n);
    if (unit == "s") return std::chrono::seconds(n);
    if (unit == "m") return std::chrono::minutes(n);
    if (unit == "h") return std::chrono::hours(n);
    throw std::invalid_argument("unknown duration unit in '" + text + "' (use ms, s, m or h)");
}

inline std::vector<std::string> parse_list(const std::string& text) {
    std::vector<std::string> items;
    std::istringstream in(text);
    for (std::string item; std::getline(in, item, ',');) {
        item.erase(0, item.find_first_not_of(' '));
        item.erase(item.find_last_not_of(' ') + 1);
        if (!item.empty()) items.push_back(item);
    }
    if (items.empty()) throw std::invalid_argument("expected a comma-separated list");
    return items;
}

inline std::string parse_url(const std::string& text) {
    const auto scheme = text.find("://");
    if (scheme == std::string::npos || scheme == 0 || scheme + 3 == text.size()) throw std::invalid_argument("expected a URL like postgres://host/db");
    return text;
}

/// One line per problem, all of them.
class ConfigError : public std::runtime_error {
public:
    explicit ConfigError(std::vector<std::string> problems) : std::runtime_error(join(problems)), problems_(std::move(problems)) {}
    const std::vector<std::string>& problems() const { return problems_; }

private:
    static std::string join(const std::vector<std::string>& p) {
        std::string out = std::to_string(p.size()) + " configuration problem(s):";
        for (const auto& line : p) out += "\n  " + line;
        return out;
    }
    std::vector<std::string> problems_;
};

/// Binds environment variables to members of @p Config.
template <typename Config>
class Schema {
public:
    explicit Schema(std::string prefix) : prefix_(std::move(prefix)) {}

    /// @p member is where the value goes; @p parse converts the text; no default means required.
    template <typename T>
    Schema& field(const std::string& name, T Config::*member, std::function<T(const std::string&)> parse, std::string help,
                  std::optional<std::string> fallback = std::nullopt) {
        const std::string var = prefix_ + name;
        fields_.push_back({var, std::move(help), fallback, false,
                           [member, parse, var](Config& config, const std::string& text) { config.*member = parse(text); }});
        return *this;
    }

    /// Secrets are always required and never shown.
    Schema& secret(const std::string& name, Secret Config::*member, std::string help) {
        const std::string var = prefix_ + name;
        fields_.push_back({var, std::move(help), std::nullopt, true, [member](Config& config, const std::string& text) {
                               if (text.size() < 8) throw std::invalid_argument("must be at least 8 characters");
                               config.*member = Secret(text);
                           }});
        return *this;
    }

    /// Parse every field, collecting all problems; throws ConfigError if there are any.
    /// Unknown variables with the prefix are reported too – usually a typo.
    Config load(const Env& env) const {
        Config config{};
        std::vector<std::string> problems;
        for (const auto& f : fields_) {
            const auto it = env.find(f.var);
            const bool present = it != env.end() && !it->second.empty();
            if (!present && !f.fallback) {
                problems.push_back(f.var + ": required (" + f.help + ")");
                continue;
            }
            try {
                f.assign(config, present ? it->second : *f.fallback);
            } catch (const std::exception& e) {
                problems.push_back(f.var + ": " + e.what());  // the bad value itself is not repeated for secrets
            }
        }
        for (const auto& [var, value] : env) {
            if (!var.starts_with(prefix_)) continue;
            const bool known = std::any_of(fields_.begin(), fields_.end(), [&](const auto& f) { return f.var == var; });
            if (!known) problems.push_back(var + ": unknown setting");
        }
        if (!problems.empty()) throw ConfigError(problems);
        return config;
    }

    /// "VAR=value" lines for logs, with secrets shown as ***.
    std::string describe(const Env& env) const {
        std::string out;
        for (const auto& f : fields_) {
            const auto it = env.find(f.var);
            const std::string value = it != env.end() && !it->second.empty() ? it->second : f.fallback.value_or("");
            out += f.var + "=" + (f.secret ? std::string("***") : value) + (it == env.end() && f.fallback ? " (default)" : "") + "\n";
        }
        return out;
    }

    /// A documented template for operators: every variable, its help and its default.
    std::string env_example() const {
        std::string out;
        for (const auto& f : fields_) {
            out += "# " + f.help + (f.fallback ? "" : " (required)") + "\n" + f.var + "=" + (f.secret ? "" : f.fallback.value_or("")) + "\n";
        }
        return out;
    }

private:
    struct Field {
        std::string var;
        std::string help;
        std::optional<std::string> fallback;
        bool secret;
        std::function<void(Config&, const std::string&)> assign;
    };
    std::string prefix_;
    std::vector<Field> fields_;
};

enum class Mode { live, shadow };

struct DispatchConfig {
    std::string database_url;
    int workers = 0;
    std::chrono::milliseconds courier_timeout{};
    bool dry_run = false;
    Mode mode = Mode::live;
    std::vector<std::string> regions;
    Secret maps_api_key;
};

inline Schema<DispatchConfig> dispatch_schema() {
    Schema<DispatchConfig> schema("DISPATCH_");
    schema.field<std::string>("DATABASE_URL", &DispatchConfig::database_url, parse_url, "PostgreSQL connection URL")
        .field<int>("WORKERS", &DispatchConfig::workers, [](const std::string& t) { return parse_int(t, 1, 64); }, "worker threads, 1-64", "4")
        .field<std::chrono::milliseconds>("COURIER_TIMEOUT", &DispatchConfig::courier_timeout, parse_duration, "how long a courier has to accept", "45s")
        .field<bool>("DRY_RUN", &DispatchConfig::dry_run, parse_bool, "log assignments without sending them", "false")
        .field<Mode>("MODE", &DispatchConfig::mode, [](const std::string& t) {
            if (t == "live") return Mode::live;
            if (t == "shadow") return Mode::shadow;
            throw std::invalid_argument("expected live or shadow, got '" + t + "'");
        }, "live or shadow (compare with the old dispatcher)", "live")
        .field<std::vector<std::string>>("REGIONS", &DispatchConfig::regions, parse_list, "comma-separated city codes")
        .secret("MAPS_API_KEY", &DispatchConfig::maps_api_key, "routing API key");
    return schema;
}

/// The interactive demo: type VAR=value lines; a blank line loads and reports.
inline int run(std::istream& in, std::ostream& out) {
    out << "Day 91 – Capstone: Type-safe Configuration\n";
    const auto schema = dispatch_schema();
    Env env;
    while (auto line = prompt_line(in, out, "VAR=value (blank to load)> ")) {
        if (line->empty()) break;
        const auto eq = line->find('=');
        if (eq != std::string::npos) env[line->substr(0, eq)] = line->substr(eq + 1);
    }
    try {
        const auto config = schema.load(env);
        out << "loaded: " << config.workers << " worker(s), timeout " << config.courier_timeout.count() << " ms, " << config.regions.size()
            << " region(s)\n" << schema.describe(env);
    } catch (const ConfigError& error) {
        out << error.what() << "\n\nexample:\n" << schema.env_example();
    }
    return 0;
}

}  // namespace cppm::day91
