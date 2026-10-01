/**
 * @file
 * Day 51 – Working with JSON.
 *
 * Scenario: a *smart-garden irrigation controller* that is configured with JSON and reports
 * its status as JSON. The lesson builds the JSON support itself – a value model, a
 * recursive-descent parser that reports the line and column of every syntax error, and a
 * serialiser that escapes strings correctly – the same job nlohmann/json does in production.
 *
 * Deliverables (syllabus):
 * - JSON value model
 * - Recursive-descent parsing
 * - Serialisation with escaping
 * - Error positions
 */
#pragma once

#include <cctype>
#include <cmath>
#include <cstddef>
#include <iomanip>
#include <istream>
#include <ostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>
#include <variant>
#include <vector>

#include "cppm/lesson.hpp"

namespace cppm::day51 {

inline constexpr Deliverable DELIVERABLES[] = {
    {"a JSON value model built on std::variant", "Json"},
    {"a recursive-descent parser", "Parser"},
    {"syntax errors with line and column", "JsonError"},
    {"escaping strings for output", "escape"},
    {"compact and pretty serialisation", "dump"},
    {"reading typed settings from parsed JSON", "load_zones"},
};

/// One JSON value. Objects keep their keys in insertion order (like nlohmann::ordered_json).
class Json {
public:
    using Array = std::vector<Json>;
    using Object = std::vector<std::pair<std::string, Json>>;
    using Value = std::variant<std::nullptr_t, bool, double, std::string, Array, Object>;

    Json() : value_(nullptr) {}
    Json(std::nullptr_t) : value_(nullptr) {}
    Json(bool b) : value_(b) {}
    Json(double d) : value_(d) {}
    Json(int i) : value_(static_cast<double>(i)) {}
    Json(const char* s) : value_(std::string(s)) {}
    Json(std::string s) : value_(std::move(s)) {}
    Json(Array a) : value_(std::move(a)) {}
    Json(Object o) : value_(std::move(o)) {}

    bool is_null() const { return std::holds_alternative<std::nullptr_t>(value_); }
    bool is_object() const { return std::holds_alternative<Object>(value_); }
    bool is_array() const { return std::holds_alternative<Array>(value_); }
    double as_number() const { return get<double>("a number"); }
    bool as_bool() const { return get<bool>("a boolean"); }
    const std::string& as_string() const { return get<std::string>("a string"); }
    const Array& as_array() const { return get<Array>("an array"); }
    const Object& as_object() const { return get<Object>("an object"); }
    const Value& value() const { return value_; }

    /// Object member lookup; throws std::out_of_range for a missing key.
    const Json& at(std::string_view key) const {
        for (const auto& [name, member] : as_object()) {
            if (name == key) return member;
        }
        throw std::out_of_range("missing key \"" + std::string(key) + "\"");
    }
    bool contains(std::string_view key) const {
        if (!is_object()) return false;
        for (const auto& [name, member] : as_object()) {
            if (name == key) return true;
        }
        return false;
    }
    bool operator==(const Json&) const = default;

private:
    template <typename T>
    const T& get(const char* what) const {
        if (const T* found = std::get_if<T>(&value_)) return *found;
        throw std::invalid_argument(std::string("JSON value is not ") + what);
    }
    Value value_;
};

/// A syntax error with its position, so an editor can jump straight to it.
class JsonError : public std::runtime_error {
public:
    JsonError(const std::string& message, int line, int column)
        : std::runtime_error("line " + std::to_string(line) + ", column " + std::to_string(column) + ": " + message),
          line_(line),
          column_(column) {}
    int line() const noexcept { return line_; }
    int column() const noexcept { return column_; }

private:
    int line_;
    int column_;
};

/// Recursive descent: one member function per grammar rule (value, object, array, string, number).
class Parser {
public:
    explicit Parser(std::string_view text) : text_(text) {}

    Json parse_document() {
        Json value = parse_value(0);
        skip_whitespace();
        if (position_ != text_.size()) fail("unexpected trailing characters");
        return value;
    }

private:
    static constexpr int max_depth = 64;  // deeply nested input must not overflow the stack

    [[noreturn]] void fail(const std::string& message) const {
        int line = 1;
        int column = 1;
        for (std::size_t i = 0; i < position_ && i < text_.size(); ++i) {
            if (text_[i] == '\n') {
                ++line;
                column = 1;
            } else {
                ++column;
            }
        }
        throw JsonError(message, line, column);
    }
    void skip_whitespace() {
        while (position_ < text_.size() && std::isspace(static_cast<unsigned char>(text_[position_]))) ++position_;
    }
    char peek() const { return position_ < text_.size() ? text_[position_] : '\0'; }
    void expect(char c) {
        if (peek() != c) fail(std::string("expected '") + c + "'");
        ++position_;
    }
    bool consume_word(std::string_view word) {
        if (text_.substr(position_, word.size()) == word) {
            position_ += word.size();
            return true;
        }
        return false;
    }

    Json parse_value(int depth) {
        if (depth > max_depth) fail("nesting too deep");
        skip_whitespace();
        const char c = peek();
        if (c == '{') return parse_object(depth);
        if (c == '[') return parse_array(depth);
        if (c == '"') return parse_string();
        if (c == '-' || std::isdigit(static_cast<unsigned char>(c))) return parse_number();
        if (consume_word("true")) return true;
        if (consume_word("false")) return false;
        if (consume_word("null")) return nullptr;
        fail(c == '\0' ? "unexpected end of input" : "unexpected character");
    }

    Json parse_object(int depth) {
        expect('{');
        Json::Object members;
        skip_whitespace();
        if (peek() == '}') {
            ++position_;
            return members;
        }
        while (true) {
            skip_whitespace();
            if (peek() != '"') fail("object keys must be strings");
            std::string key = parse_string().as_string();
            skip_whitespace();
            expect(':');
            members.emplace_back(std::move(key), parse_value(depth + 1));
            skip_whitespace();
            if (peek() == ',') {
                ++position_;
                continue;
            }
            expect('}');
            return members;
        }
    }

    Json parse_array(int depth) {
        expect('[');
        Json::Array items;
        skip_whitespace();
        if (peek() == ']') {
            ++position_;
            return items;
        }
        while (true) {
            items.push_back(parse_value(depth + 1));
            skip_whitespace();
            if (peek() == ',') {
                ++position_;
                continue;
            }
            expect(']');
            return items;
        }
    }

    Json parse_string() {
        expect('"');
        std::string out;
        while (true) {
            if (position_ >= text_.size()) fail("unterminated string");
            const char c = text_[position_++];
            if (c == '"') return out;
            if (static_cast<unsigned char>(c) < 0x20) fail("control character in string");
            if (c != '\\') {
                out += c;
                continue;
            }
            const char escaped = peek();
            ++position_;
            switch (escaped) {
                case '"': out += '"'; break;
                case '\\': out += '\\'; break;
                case '/': out += '/'; break;
                case 'n': out += '\n'; break;
                case 't': out += '\t'; break;
                case 'r': out += '\r'; break;
                case 'b': out += '\b'; break;
                case 'f': out += '\f'; break;
                case 'u': out += parse_unicode_escape(); break;
                default: --position_; fail("invalid escape sequence");
            }
        }
    }

    /// \uXXXX for code points below U+0800, encoded as UTF-8 (enough for this controller).
    std::string parse_unicode_escape() {
        if (position_ + 4 > text_.size()) fail("incomplete \\u escape");
        unsigned code = 0;
        for (int i = 0; i < 4; ++i) {
            const char h = text_[position_++];
            code <<= 4;
            if (h >= '0' && h <= '9') code |= static_cast<unsigned>(h - '0');
            else if (h >= 'a' && h <= 'f') code |= static_cast<unsigned>(h - 'a' + 10);
            else if (h >= 'A' && h <= 'F') code |= static_cast<unsigned>(h - 'A' + 10);
            else fail("bad hex digit in \\u escape");
        }
        if (code < 0x80) return std::string(1, static_cast<char>(code));
        if (code < 0x800) {
            return {static_cast<char>(0xC0 | (code >> 6)), static_cast<char>(0x80 | (code & 0x3F))};
        }
        fail("\\u escapes above U+07FF are not supported here");
    }

    Json parse_number() {
        const std::size_t start = position_;
        if (peek() == '-') ++position_;
        if (!std::isdigit(static_cast<unsigned char>(peek()))) fail("digit expected");
        if (peek() == '0' && position_ + 1 < text_.size() && std::isdigit(static_cast<unsigned char>(text_[position_ + 1]))) {
            fail("leading zeros are not allowed");
        }
        while (std::isdigit(static_cast<unsigned char>(peek())) || peek() == '.' || peek() == 'e' || peek() == 'E' ||
               peek() == '+' || peek() == '-') {
            ++position_;
        }
        std::istringstream number{std::string(text_.substr(start, position_ - start))};
        number.imbue(std::locale::classic());
        double value = 0;
        if (!(number >> value) || !(number >> std::ws).eof()) {
            position_ = start;
            fail("malformed number");
        }
        return value;
    }

    std::string_view text_;
    std::size_t position_{0};
};

inline Json parse(std::string_view text) { return Parser(text).parse_document(); }

/// Quote and escape a string for JSON output; control characters become \u00XX.
inline std::string escape(std::string_view text) {
    std::ostringstream out;
    out << '"';
    for (const char c : text) {
        switch (c) {
            case '"': out << "\\\""; break;
            case '\\': out << "\\\\"; break;
            case '\n': out << "\\n"; break;
            case '\t': out << "\\t"; break;
            case '\r': out << "\\r"; break;
            default:
                if (static_cast<unsigned char>(c) < 0x20) {
                    out << "\\u" << std::hex << std::setw(4) << std::setfill('0') << static_cast<int>(c) << std::dec;
                } else {
                    out << c;
                }
        }
    }
    out << '"';
    return out.str();
}

/// Serialise; @p indent < 0 gives compact output, otherwise that many spaces per level.
inline std::string dump(const Json& json, int indent = -1, int level = 0) {
    const std::string newline = indent < 0 ? "" : "\n";
    const auto pad = [&](int depth) { return indent < 0 ? std::string() : std::string(static_cast<std::size_t>(indent * depth), ' '); };
    const std::string colon = indent < 0 ? ":" : ": ";
    return std::visit(
        [&](const auto& v) -> std::string {
            using T = std::decay_t<decltype(v)>;
            if constexpr (std::is_same_v<T, std::nullptr_t>) {
                return "null";
            } else if constexpr (std::is_same_v<T, bool>) {
                return v ? "true" : "false";
            } else if constexpr (std::is_same_v<T, double>) {
                if (!std::isfinite(v)) throw std::domain_error("JSON cannot represent NaN or infinity");
                std::ostringstream out;
                out.imbue(std::locale::classic());
                out << std::setprecision(15) << v;
                return out.str();
            } else if constexpr (std::is_same_v<T, std::string>) {
                return escape(v);
            } else if constexpr (std::is_same_v<T, Json::Array>) {
                if (v.empty()) return "[]";
                std::string out = "[" + newline;
                for (std::size_t i = 0; i < v.size(); ++i) {
                    out += pad(level + 1) + dump(v[i], indent, level + 1) + (i + 1 < v.size() ? "," : "") + newline;
                }
                return out + pad(level) + "]";
            } else {
                if (v.empty()) return "{}";
                std::string out = "{" + newline;
                for (std::size_t i = 0; i < v.size(); ++i) {
                    out += pad(level + 1) + escape(v[i].first) + colon + dump(v[i].second, indent, level + 1) +
                           (i + 1 < v.size() ? "," : "") + newline;
                }
                return out + pad(level) + "}";
            }
        },
        json.value());
}

/// One irrigation zone from the controller's configuration.
struct Zone {
    std::string name;
    int minutes;
    bool enabled;
};

/// Read {"zones": [{"name": …, "minutes": …, "enabled": …}, …]} with type and range checks.
inline std::vector<Zone> load_zones(const Json& config) {
    std::vector<Zone> zones;
    for (const Json& item : config.at("zones").as_array()) {
        const double minutes = item.at("minutes").as_number();
        if (minutes < 0 || minutes > 120 || minutes != std::floor(minutes)) {
            throw std::invalid_argument("minutes must be a whole number 0-120");
        }
        zones.push_back({item.at("name").as_string(), static_cast<int>(minutes),
                         item.contains("enabled") ? item.at("enabled").as_bool() : true});
    }
    return zones;
}

/// The interactive demo: paste a configuration document, finish with END; prints a status report.
inline int run(std::istream& in, std::ostream& out) {
    out << "Day 51 – Working with JSON\nPaste the controller configuration, then END\n";
    std::string text;
    while (auto line = prompt_line(in, out, "")) {
        if (*line == "END") break;
        text += *line + '\n';
    }
    try {
        const auto zones = load_zones(parse(text));
        Json::Array report;
        int total = 0;
        for (const auto& zone : zones) {
            report.push_back(Json::Object{{"zone", zone.name}, {"minutes", zone.enabled ? zone.minutes : 0}});
            total += zone.enabled ? zone.minutes : 0;
        }
        out << dump(Json::Object{{"status", "scheduled"}, {"total_minutes", total}, {"zones", report}}, 2) << '\n';
    } catch (const JsonError& error) {
        out << "syntax error at " << error.what() << '\n';
    } catch (const std::exception& error) {
        out << "invalid configuration: " << error.what() << '\n';
    }
    return 0;
}

}  // namespace cppm::day51
