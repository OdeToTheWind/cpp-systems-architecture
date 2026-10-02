/**
 * @file
 * Day 88 – Capstone: Report Generator.
 *
 * Scenario: the *monthly billing report of a freelance design studio*. Logged hours become one
 * report per month: amounts are integer cents (never floating point), VAT is rounded once per
 * invoice line by a documented rule, and the same data is rendered as an HTML page through a
 * small template engine and as a CSV for the accountant – both safely escaped.
 *
 * Deliverables (syllabus):
 * - Integer money arithmetic
 * - Templated HTML
 * - CSV export
 * - Escaping
 */
#pragma once

#include <cstdint>
#include <istream>
#include <map>
#include <ostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

#include "cppm/lesson.hpp"

namespace cppm::day88 {

inline constexpr Deliverable DELIVERABLES[] = {
    {"money as integer cents with explicit rounding", "Money"},
    {"HTML escaping for untrusted text", "html_escape"},
    {"a template engine with escaped values and loops", "render_template"},
    {"CSV fields safe against injection", "csv_field"},
    {"building the report data once for both outputs", "build_report"},
};

/// An amount in cents. Arithmetic stays in integers; only division needs a rounding rule.
class Money {
  public:
    constexpr Money() = default;  // not explicit, so aggregates holding Money can be value-initialised
    constexpr explicit Money(std::int64_t cents) : cents_(cents) {}
    constexpr std::int64_t cents() const { return cents_; }
    constexpr Money operator+(Money o) const { return Money(cents_ + o.cents_); }
    constexpr Money& operator+=(Money o) {
        cents_ += o.cents_;
        return *this;
    }
    constexpr bool operator==(const Money&) const = default;
    /// Multiply by numerator/denominator, rounding half away from zero (the usual invoicing rule).
    constexpr Money scaled(std::int64_t numerator, std::int64_t denominator) const {
        if (denominator <= 0) throw std::invalid_argument("denominator must be positive");
        const std::int64_t product = cents_ * numerator;
        const std::int64_t half = denominator / 2;
        return Money(product >= 0 ? (product + half) / denominator : (product - half) / denominator);
    }
    /// "€1,234.56" / "-€0.05"
    std::string str() const {
        const std::int64_t abs = cents_ < 0 ? -cents_ : cents_;
        std::string whole = std::to_string(abs / 100);
        for (auto i = static_cast<std::ptrdiff_t>(whole.size()) - 3; i > 0; i -= 3)
            whole.insert(static_cast<std::size_t>(i), ",");
        const auto rest = abs % 100;
        return (cents_ < 0 ? "-" : "") + std::string("€") + whole + "." + (rest < 10 ? "0" : "") + std::to_string(rest);
    }
    /// "1234.56" for machines (CSV): no currency sign, no thousands separators.
    std::string plain() const {
        const std::int64_t abs = cents_ < 0 ? -cents_ : cents_;
        return (cents_ < 0 ? "-" : "") + std::to_string(abs / 100) + "." + (abs % 100 < 10 ? "0" : "") +
               std::to_string(abs % 100);
    }

  private:
    std::int64_t cents_ = 0;
};

inline std::string html_escape(const std::string& text) {
    std::string out;
    for (const char c : text) {
        switch (c) {
            case '&': out += "&amp;"; break;
            case '<': out += "&lt;"; break;
            case '>': out += "&gt;"; break;
            case '"': out += "&quot;"; break;
            case '\'': out += "&#39;"; break;
            default: out += c;
        }
    }
    return out;
}

/// Data for a template: named values, and named lists of rows for loops.
struct Context {
    std::map<std::string, std::string> values;
    std::map<std::string, std::vector<std::map<std::string, std::string>>> lists;
};

/// A Mustache-like subset:
///   {{name}}            value, HTML-escaped (the safe default)
///   {{&name}}           value inserted raw – only for HTML the program generated itself
///   {{#list}}…{{/list}} repeated for each row; inside, row keys shadow outer values
/// A missing key is an error, so a typo never ships as an empty cell.
inline std::string render_template(const std::string& tpl, const Context& ctx,
                                   const std::map<std::string, std::string>* row = nullptr) {
    std::string out;
    std::size_t pos = 0;
    while (true) {
        const auto open = tpl.find("{{", pos);
        out.append(tpl, pos, open == std::string::npos ? std::string::npos : open - pos);
        if (open == std::string::npos) break;
        const auto close = tpl.find("}}", open);
        if (close == std::string::npos) throw std::runtime_error("unclosed {{ at offset " + std::to_string(open));
        std::string tag = tpl.substr(open + 2, close - open - 2);
        pos = close + 2;
        if (tag.starts_with('#')) {
            const std::string name = tag.substr(1);
            const std::string end_tag = "{{/" + name + "}}";
            const auto end = tpl.find(end_tag, pos);
            if (end == std::string::npos) throw std::runtime_error("section " + name + " is not closed");
            const auto list = ctx.lists.find(name);
            if (list == ctx.lists.end()) throw std::runtime_error("unknown list " + name);
            const std::string body = tpl.substr(pos, end - pos);
            for (const auto& r : list->second) out += render_template(body, ctx, &r);
            pos = end + end_tag.size();
            continue;
        }
        const bool raw = tag.starts_with('&');
        if (raw) tag.erase(0, 1);
        std::string value;
        if (row && row->contains(tag))
            value = row->at(tag);
        else if (ctx.values.contains(tag))
            value = ctx.values.at(tag);
        else
            throw std::runtime_error("unknown key " + tag);
        out += raw ? value : html_escape(value);
    }
    return out;
}

/// One CSV field: quoted when needed, and cells starting with = + - @ are prefixed with ' so a
/// spreadsheet never runs them as formulas (CSV injection).
inline std::string csv_field(const std::string& value) {
    std::string v = value;
    if (!v.empty() && (v[0] == '=' || v[0] == '+' || v[0] == '-' || v[0] == '@')) v = "'" + v;
    if (v.find_first_of(",\"\n\r") == std::string::npos) return v;
    std::string out = "\"";
    for (const char c : v) out += c == '"' ? std::string("\"\"") : std::string(1, c);
    return out + "\"";
}

struct Entry {
    std::string client;
    std::string task;
    int minutes;
    Money hourly_rate;
};

struct ReportLine {
    std::string client;
    std::string task;
    int minutes;
    Money net;
    Money vat;
};

struct Report {
    std::string month;
    std::vector<ReportLine> lines;
    std::map<std::string, Money> net_by_client;
    Money net_total;
    Money vat_total;
    Money gross_total() const { return net_total + vat_total; }
};

/// Net = rate x minutes / 60 and VAT = net x rate% are each rounded per line; totals are sums of
/// the rounded lines, so the report always adds up exactly as printed.
inline Report build_report(const std::string& month, const std::vector<Entry>& entries, int vat_percent) {
    Report report{month, {}, {}, Money{}, Money{}};
    for (const auto& e : entries) {
        if (e.minutes <= 0) throw std::invalid_argument("minutes must be positive for " + e.task);
        const Money net = e.hourly_rate.scaled(e.minutes, 60);
        const Money vat = net.scaled(vat_percent, 100);
        report.lines.push_back({e.client, e.task, e.minutes, net, vat});
        report.net_by_client[e.client] += net;
        report.net_total += net;
        report.vat_total += vat;
    }
    return report;
}

inline const char* const REPORT_TEMPLATE =
    "<h1>Billing {{month}}</h1>\n<table>\n<tr><th>Client</th><th>Task</th><th>Hours</th><th>Net</th></tr>\n"
    "{{#lines}}<tr><td>{{client}}</td><td>{{task}}</td><td>{{hours}}</td><td>{{net}}</td></tr>\n{{/lines}}"
    "</table>\n<p>Net {{net_total}} + VAT {{vat_total}} = <strong>{{gross_total}}</strong></p>\n";

inline std::string hours(int minutes) {
    return std::to_string(minutes / 60) + ":" + (minutes % 60 < 10 ? "0" : "") + std::to_string(minutes % 60);
}

inline std::string to_html(const Report& report) {
    Context ctx;
    ctx.values = {{"month", report.month},
                  {"net_total", report.net_total.str()},
                  {"vat_total", report.vat_total.str()},
                  {"gross_total", report.gross_total().str()}};
    for (const auto& l : report.lines) {
        ctx.lists["lines"].push_back(
            {{"client", l.client}, {"task", l.task}, {"hours", hours(l.minutes)}, {"net", l.net.str()}});
    }
    ctx.lists.try_emplace("lines");
    return render_template(REPORT_TEMPLATE, ctx);
}

inline std::string to_csv(const Report& report) {
    std::string out = "client,task,minutes,net,vat\r\n";  // RFC 4180 line endings
    for (const auto& l : report.lines) {
        out += csv_field(l.client) + "," + csv_field(l.task) + "," + std::to_string(l.minutes) + "," + l.net.plain() +
               "," + l.vat.plain() + "\r\n";
    }
    return out;
}

/// The interactive demo: "client minutes rate_cents task…" lines; a blank line prints the report.
inline int run(std::istream& in, std::ostream& out) {
    out << "Day 88 – Capstone: Report Generator\n";
    std::vector<Entry> entries;
    while (auto line = prompt_line(in, out, "client minutes rate_cents task> ")) {
        std::istringstream words(*line);
        Entry e{};
        std::int64_t rate = 0;
        if (!(words >> e.client >> e.minutes >> rate)) break;
        e.hourly_rate = Money(rate);
        std::getline(words >> std::ws, e.task);
        entries.push_back(e);
    }
    try {
        const Report report = build_report("2026-06", entries, 23);
        out << to_html(report) << to_csv(report);
    } catch (const std::exception& error) {
        out << "  " << error.what() << '\n';
    }
    return 0;
}

}  // namespace cppm::day88
