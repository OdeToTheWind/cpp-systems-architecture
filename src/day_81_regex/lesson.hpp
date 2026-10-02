/**
 * @file
 * Day 81 – Regular Expressions.
 *
 * Scenario: a *support-ticket scrubber*. Before customer messages are copied into the bug
 * tracker, personal data has to go: e-mail addresses, phone numbers, payment-card numbers and IP
 * addresses are found with regular expressions and masked, while ticket IDs and log timestamps
 * are parsed with capture groups.
 *
 * Deliverables (syllabus):
 * - std::regex matching and searching
 * - Capture groups
 * - Replacement
 * - Redacting sensitive data
 */
#pragma once

#include <cctype>
#include <functional>
#include <istream>
#include <optional>
#include <ostream>
#include <regex>
#include <sstream>
#include <string>
#include <vector>

#include "cppm/lesson.hpp"

namespace cppm::day81 {

inline constexpr Deliverable DELIVERABLES[] = {
    {"whole-string validation with regex_match", "is_ticket_id"},
    {"capture groups that split a log line", "parse_log_line"},
    {"finding every match with sregex_iterator", "find_emails"},
    {"replacement with a callback for each match", "replace_with"},
    {"redacting e-mails, phones, cards and IP addresses", "redact"},
};

/// Ticket IDs look like "SUP-20417": three capitals, a dash, four to six digits – nothing else.
inline bool is_ticket_id(const std::string& text) {
    static const std::regex pattern(R"([A-Z]{3}-\d{4,6})");
    return std::regex_match(text, pattern);  // match: the *whole* string must fit
}

struct LogLine {
    std::string date;
    std::string time;
    std::string level;
    std::string message;
};

/// "2026-05-30 14:02:11 [ERROR] payment failed" -> its parts, via numbered capture groups.
inline std::optional<LogLine> parse_log_line(const std::string& line) {
    static const std::regex pattern(R"(^(\d{4}-\d{2}-\d{2}) (\d{2}:\d{2}:\d{2}) \[(DEBUG|INFO|WARN|ERROR)\] (.*)$)");
    std::smatch m;
    if (!std::regex_match(line, m, pattern)) return std::nullopt;
    return LogLine{m[1].str(), m[2].str(), m[3].str(), m[4].str()};
}

inline const std::regex& email_pattern() {
    static const std::regex pattern(R"(([A-Za-z0-9._%+-]+)@([A-Za-z0-9-]+(?:\.[A-Za-z0-9-]+)*\.[A-Za-z]{2,}))");
    return pattern;
}

/// Every e-mail address in @p text, in order.
inline std::vector<std::string> find_emails(const std::string& text) {
    std::vector<std::string> found;
    for (std::sregex_iterator it(text.begin(), text.end(), email_pattern()), end; it != end; ++it)
        found.push_back(it->str());
    return found;
}

/// Like std::regex_replace, but the replacement for each match is computed by a function –
/// needed when the output depends on the match (keep the domain, check a checksum, …).
inline std::string replace_with(const std::string& text, const std::regex& pattern,
                                const std::function<std::string(const std::smatch&)>& replacement) {
    std::string out;
    auto last = text.cbegin();
    for (std::sregex_iterator it(text.begin(), text.end(), pattern), end; it != end; ++it) {
        out.append(last, (*it)[0].first);
        out += replacement(*it);
        last = (*it)[0].second;
    }
    out.append(last, text.cend());
    return out;
}

/// The Luhn checksum used by payment cards: catches most typos, and avoids masking order numbers.
inline bool luhn_valid(const std::string& digits) {
    int sum = 0;
    bool twice = false;
    int count = 0;
    for (auto it = digits.rbegin(); it != digits.rend(); ++it) {
        if (!std::isdigit(static_cast<unsigned char>(*it))) continue;
        int d = *it - '0';
        if (twice && (d *= 2) > 9) d -= 9;
        sum += d;
        twice = !twice;
        ++count;
    }
    return count >= 13 && sum % 10 == 0;
}

/// Mask personal data:
/// - e-mails keep the first letter and the domain: "j***@example.com"
/// - card numbers that pass Luhn keep the last four digits: "**** **** **** 4242"
/// - North-American style phone numbers become "[phone]" (other layouts are a known gap)
/// - IPv4 addresses lose their last octet: "203.0.113.x"
inline std::string redact(const std::string& text) {
    static const std::regex card(R"(\b(?:\d[ -]?){12,18}\d\b)");
    // ECMAScript regex has no look-behind, so the character before a phone number is captured
    // as group 1 and put back: this stops a match starting inside a longer number.
    static const std::regex phone(R"((^|[^\d+])(?:\+\d{1,3}[ .-]?)?\(?\d{3}\)?[ .-]?\d{3}[ .-]?\d{3,4}\b)");
    static const std::regex ipv4(
        R"(\b((?:25[0-5]|2[0-4]\d|1?\d?\d)\.(?:25[0-5]|2[0-4]\d|1?\d?\d)\.(?:25[0-5]|2[0-4]\d|1?\d?\d))\.(?:25[0-5]|2[0-4]\d|1?\d?\d)\b)");
    std::string out = replace_with(text, email_pattern(),
                                   [](const std::smatch& m) { return m[1].str().substr(0, 1) + "***@" + m[2].str(); });
    out = replace_with(out, card, [](const std::smatch& m) {
        std::string digits;
        for (const char c : m.str()) {
            if (std::isdigit(static_cast<unsigned char>(c))) digits += c;
        }
        return luhn_valid(digits) ? "**** **** **** " + digits.substr(digits.size() - 4) : m.str();
    });
    out = std::regex_replace(out, ipv4, "$1.x");  // $1 refers to the first capture group
    return std::regex_replace(out, phone, "$1[phone]");
}

/// The interactive demo: each line is scrubbed; log lines are also parsed and ticket IDs checked.
inline int run(std::istream& in, std::ostream& out) {
    out << "Day 81 – Regular Expressions\n";
    int changed = 0;
    while (auto line = prompt_line(in, out, "text> ")) {
        if (line->empty()) break;
        if (const auto log = parse_log_line(*line)) out << "  [" << log->level << " at " << log->time << "] ";
        const std::string clean = redact(*line);
        if (clean != *line) ++changed;
        out << "  " << clean << '\n';
        std::istringstream words(*line);
        for (std::string w; words >> w;) {
            if (is_ticket_id(w)) out << "  ticket " << w << '\n';
        }
    }
    out << changed << " line(s) redacted\n";
    return 0;
}

}  // namespace cppm::day81
