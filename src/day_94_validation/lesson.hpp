/**
 * @file
 * Day 94 – Capstone: Validation Library.
 *
 * Scenario: the *order API of an event-ticketing site*. A request contains a buyer and a list of
 * attendees. Small validators (string length, e-mail, integer range, one-of) are composed into a
 * schema for the whole nested payload; validation reports *every* problem with a path such as
 * `attendees[1].email`, and returns a normalised copy (trimmed text, lower-cased e-mails,
 * "2" -> 2, defaults filled in) that the rest of the system can trust.
 *
 * Deliverables (syllabus):
 * - Composable validators
 * - Error paths
 * - Normalising input
 * - Validating nested data
 */
#pragma once

#include <algorithm>
#include <cctype>
#include <cmath>
#include <functional>
#include <istream>
#include <optional>
#include <ostream>
#include <sstream>
#include <string>
#include <utility>
#include <variant>
#include <vector>

#include "cppm/lesson.hpp"

namespace cppm::day94 {

inline constexpr Deliverable DELIVERABLES[] = {
    {"a dynamic value for untrusted input", "Value"},
    {"validators that normalise as they check", "text"},
    {"composing validators in sequence", "all_of"},
    {"objects with required, optional and unknown fields", "object"},
    {"lists whose errors carry the item index", "list"},
};

/// JSON-like data: null, bool, number, string, list, or object (ordered key/value pairs).
struct Value {
    using List = std::vector<Value>;
    using Object = std::vector<std::pair<std::string, Value>>;
    std::variant<std::monostate, bool, double, std::string, List, Object> data;

    Value() = default;
    Value(bool b) : data(b) {}
    Value(int n) : data(static_cast<double>(n)) {}
    Value(double n) : data(n) {}
    Value(const char* s) : data(std::string(s)) {}
    Value(std::string s) : data(std::move(s)) {}
    Value(List l) : data(std::move(l)) {}
    Value(Object o) : data(std::move(o)) {}

    template <typename T>
    const T* get() const { return std::get_if<T>(&data); }
    const Value* field(const std::string& key) const {
        if (const auto* o = get<Object>()) {
            for (const auto& [k, v] : *o) {
                if (k == key) return &v;
            }
        }
        return nullptr;
    }
    bool operator==(const Value&) const = default;
};

struct Problem {
    std::string path;
    std::string message;
};
using Problems = std::vector<Problem>;

/// Check a value at a path; return the normalised value, or nullopt after recording problems.
using Validator = std::function<std::optional<Value>(const Value&, const std::string& path, Problems&)>;

inline std::optional<Value> fail(Problems& p, const std::string& path, std::string message) {
    p.push_back({path.empty() ? "(root)" : path, std::move(message)});
    return std::nullopt;
}

/// Text of @p min..@p max characters after trimming surrounding spaces.
inline Validator text(std::size_t min, std::size_t max) {
    return [min, max](const Value& v, const std::string& path, Problems& p) -> std::optional<Value> {
        const auto* s = v.get<std::string>();
        if (!s) return fail(p, path, "must be text");
        const auto first = s->find_first_not_of(" \t");
        const std::string trimmed = first == std::string::npos ? "" : s->substr(first, s->find_last_not_of(" \t") - first + 1);
        if (trimmed.size() < min) return fail(p, path, min == 1 ? "is required" : "must be at least " + std::to_string(min) + " characters");
        if (trimmed.size() > max) return fail(p, path, "must be at most " + std::to_string(max) + " characters");
        return Value(trimmed);
    };
}

/// A whole number in [low, high]; numeric text such as "2" is accepted and converted.
inline Validator integer(long long low, long long high) {
    return [low, high](const Value& v, const std::string& path, Problems& p) -> std::optional<Value> {
        double n = 0;
        if (const auto* d = v.get<double>()) {
            n = *d;
        } else if (const auto* s = v.get<std::string>(); s && !s->empty() && s->size() < 16 &&
                                                         s->find_first_not_of("-0123456789") == std::string::npos) {
            n = static_cast<double>(std::stoll(*s));
        } else {
            return fail(p, path, "must be a whole number");
        }
        if (n != std::floor(n)) return fail(p, path, "must be a whole number");
        if (n < static_cast<double>(low) || n > static_cast<double>(high)) {
            return fail(p, path, "must be between " + std::to_string(low) + " and " + std::to_string(high));
        }
        return Value(n);
    };
}

inline Validator one_of(std::vector<std::string> allowed) {
    return [allowed](const Value& v, const std::string& path, Problems& p) -> std::optional<Value> {
        const auto* s = v.get<std::string>();
        if (s && std::find(allowed.begin(), allowed.end(), *s) != allowed.end()) return v;
        std::string list;
        for (const auto& a : allowed) list += (list.empty() ? "" : ", ") + a;
        return fail(p, path, "must be one of: " + list);
    };
}

/// A custom rule on an already-normalised value.
inline Validator check(std::function<bool(const Value&)> ok, std::string message) {
    return [ok, message](const Value& v, const std::string& path, Problems& p) -> std::optional<Value> {
        return ok(v) ? std::optional<Value>(v) : fail(p, path, message);
    };
}

/// Run validators in order, each receiving the previous one's normalised output; stop at the first failure.
inline Validator all_of(std::vector<Validator> steps) {
    return [steps](const Value& v, const std::string& path, Problems& p) -> std::optional<Value> {
        std::optional<Value> current = v;
        for (const auto& step : steps) {
            current = step(*current, path, p);
            if (!current) return std::nullopt;
        }
        return current;
    };
}

/// Lower-cased address with one @ and a dot in the domain.
inline Validator email() {
    return all_of({text(3, 254), [](const Value& v, const std::string& path, Problems& p) -> std::optional<Value> {
                       std::string s = *v.get<std::string>();
                       std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
                       const auto at = s.find('@');
                       if (at == std::string::npos || at == 0 || s.find('@', at + 1) != std::string::npos ||
                           s.find('.', at) == std::string::npos || s.back() == '.' || s.find(' ') != std::string::npos) {
                           return fail(p, path, "must be an e-mail address");
                       }
                       return Value(s);
                   }});
}

struct FieldRule {
    std::string name;
    Validator validator;
    std::optional<Value> fallback;  // nullopt: required
};

inline FieldRule required(std::string name, Validator v) { return {std::move(name), std::move(v), std::nullopt}; }
inline FieldRule optional_field(std::string name, Validator v, Value fallback) { return {std::move(name), std::move(v), std::move(fallback)}; }

/// An object with the given fields. Every field is checked (errors are collected, not stopped at),
/// missing optional fields get their default, and unknown fields are rejected.
inline Validator object(std::vector<FieldRule> rules) {
    return [rules](const Value& v, const std::string& path, Problems& p) -> std::optional<Value> {
        const auto* obj = v.get<Value::Object>();
        if (!obj) return fail(p, path, "must be an object");
        const auto before = p.size();
        Value::Object out;
        for (const auto& rule : rules) {
            const std::string child = path.empty() ? rule.name : path + "." + rule.name;
            const Value* given = v.field(rule.name);
            if (!given) {
                if (rule.fallback) out.emplace_back(rule.name, *rule.fallback);
                else fail(p, child, "is required");
                continue;
            }
            if (auto ok = rule.validator(*given, child, p)) out.emplace_back(rule.name, std::move(*ok));
        }
        for (const auto& [key, value] : *obj) {
            if (std::none_of(rules.begin(), rules.end(), [&](const FieldRule& r) { return r.name == key; })) {
                fail(p, path.empty() ? key : path + "." + key, "is not allowed");
            }
        }
        if (p.size() != before) return std::nullopt;
        return Value(out);
    };
}

/// A list of @p min..@p max items, each checked by @p item; errors carry the index.
inline Validator list(Validator item, std::size_t min, std::size_t max) {
    return [item, min, max](const Value& v, const std::string& path, Problems& p) -> std::optional<Value> {
        const auto* l = v.get<Value::List>();
        if (!l) return fail(p, path, "must be a list");
        if (l->size() < min || l->size() > max) {
            return fail(p, path, "must have " + std::to_string(min) + " to " + std::to_string(max) + " items");
        }
        const auto before = p.size();
        Value::List out;
        for (std::size_t i = 0; i < l->size(); ++i) {
            if (auto ok = item((*l)[i], path + "[" + std::to_string(i) + "]", p)) out.push_back(std::move(*ok));
        }
        if (p.size() != before) return std::nullopt;
        return Value(out);
    };
}

struct Result {
    std::optional<Value> value;
    Problems problems;
};

inline Result validate(const Validator& schema, const Value& input) {
    Result r;
    r.value = schema(input, "", r.problems);
    return r;
}

/// The ticket order schema used by the API.
inline Validator order_schema() {
    const Validator attendee = object({required("name", text(2, 60)), required("email", email()),
                                       optional_field("ticket", one_of({"standard", "student", "vip"}), "standard")});
    return object({required("event", text(1, 40)), required("buyer_email", email()),
                   required("attendees", list(attendee, 1, 10)),
                   optional_field("donation_eur", all_of({integer(0, 500), check([](const Value& v) { return *v.get<double>() != 13; }, "must not be 13 (we are superstitious)")}), 0)});
}

inline std::string report(const Result& r) {
    if (r.value) return "valid\n";
    std::string out;
    for (const auto& p : r.problems) out += p.path + ": " + p.message + "\n";
    return out;
}

/// The interactive demo: "event buyer_email name1:email1[:ticket] name2:email2 …" builds an order.
inline int run(std::istream& in, std::ostream& out) {
    out << "Day 94 – Capstone: Validation Library\n";
    const auto schema = order_schema();
    while (auto line = prompt_line(in, out, "event buyer_email name:email[:ticket]...> ")) {
        std::istringstream words(*line);
        std::string event;
        std::string buyer;
        if (!(words >> event >> buyer)) break;
        Value::List attendees;
        for (std::string a; words >> a;) {
            std::vector<std::string> parts;
            std::istringstream ps(a);
            for (std::string part; std::getline(ps, part, ':');) parts.push_back(part);
            Value::Object person{{"name", parts.size() > 0 ? parts[0] : ""}, {"email", parts.size() > 1 ? parts[1] : ""}};
            if (parts.size() > 2) person.emplace_back("ticket", parts[2]);
            attendees.emplace_back(person);
        }
        const auto result = validate(schema, Value::Object{{"event", event}, {"buyer_email", buyer}, {"attendees", attendees}});
        out << report(result);
        if (result.value) {
            for (const auto& a : *result.value->field("attendees")->get<Value::List>()) {
                out << "  " << *a.field("name")->get<std::string>() << " <" << *a.field("email")->get<std::string>() << "> " << *a.field("ticket")->get<std::string>() << '\n';
            }
        }
    }
    return 0;
}

}  // namespace cppm::day94
