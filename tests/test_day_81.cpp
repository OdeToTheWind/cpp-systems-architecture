// Tests for Day 81 – Regular Expressions. All personal data below is fictional; card numbers are
// public test numbers.
#include <regex>
#include <sstream>
#include <string>
#include <vector>

#include "cppm/testing.hpp"
#include "day_81_regex/lesson.hpp"

using namespace cppm::day81;

TEST_CASE("regex_match validates the whole string") {
    CHECK(is_ticket_id("SUP-20417"));
    CHECK(is_ticket_id("BUG-1234"));
    CHECK(!is_ticket_id("sup-20417"));
    CHECK(!is_ticket_id("SUP-123"));
    CHECK(!is_ticket_id("see SUP-20417"));  // search would find it; match does not
}

TEST_CASE("capture groups split a log line") {
    const auto line = parse_log_line("2026-05-30 14:02:11 [ERROR] payment failed: card declined");
    CHECK(line.has_value());
    CHECK_EQ(line->date, "2026-05-30");
    CHECK_EQ(line->time, "14:02:11");
    CHECK_EQ(line->level, "ERROR");
    CHECK_EQ(line->message, "payment failed: card declined");
    CHECK(!parse_log_line("2026-05-30 14:02 [ERROR] short time").has_value());
    CHECK(!parse_log_line("2026-05-30 14:02:11 [FATAL] unknown level").has_value());
}

TEST_CASE("sregex_iterator finds every e-mail") {
    CHECK(find_emails("Contact jo.smith+billing@mail.example.co.uk or help@example.com.") ==
          std::vector<std::string>{"jo.smith+billing@mail.example.co.uk", "help@example.com"});
    CHECK(find_emails("no addresses @ here").empty());
}

TEST_CASE("replace_with computes each replacement") {
    const std::regex number(R"(\d+)");
    CHECK_EQ(replace_with("3 apples and 12 pears", number,
                          [](const std::smatch& m) { return std::to_string(std::stoi(m.str()) * 2); }),
             "6 apples and 24 pears");
    CHECK_EQ(replace_with("none", number, [](const std::smatch&) { return std::string("x"); }), "none");
}

TEST_CASE("Luhn separates card numbers from order numbers") {
    CHECK(luhn_valid("4242424242424242"));
    CHECK(luhn_valid("5555 5555 5555 4444"));
    CHECK(!luhn_valid("4242424242424241"));
    CHECK(!luhn_valid("1234"));  // too short to be a card
}

TEST_CASE("redaction masks personal data and keeps the rest") {
    CHECK_EQ(redact("Mail jane.doe@example.com now"), "Mail j***@example.com now");
    CHECK_EQ(redact("Card 4242 4242 4242 4242 was charged"), "Card **** **** **** 4242 was charged");
    CHECK_EQ(redact("Order 1234567890123456 shipped"), "Order 1234567890123456 shipped");  // fails Luhn: kept
    CHECK_EQ(redact("Call (555) 010-4477 or 555.010.4477"), "Call [phone] or [phone]");
    // Known gap: the pattern only knows North-American layouts, so this UK number survives.
    // A regex catches the formats it was written for – review real data before trusting it.
    CHECK_EQ(redact("Call +44 20 7946 0958"), "Call +44 20 7946 0958");
    CHECK_EQ(redact("from 203.0.113.42 via 10.0.0.1"), "from 203.0.113.x via 10.0.0.x");
    CHECK_EQ(redact("version 1.2.3 stays"), "version 1.2.3 stays");
}

TEST_CASE("run scrubs lines, parses logs and spots tickets") {
    std::istringstream in("2026-05-30 14:02:11 [WARN] user ana@example.com from 192.0.2.7\nre SUP-20417: thanks!\n\n");
    std::ostringstream out;
    CHECK_EQ(run(in, out), 0);
    const auto text = out.str();
    CHECK(text.find("[WARN at 14:02:11]") != std::string::npos);
    CHECK(text.find("user a***@example.com from 192.0.2.x") != std::string::npos);
    CHECK(text.find("ticket SUP-20417") == std::string::npos);  // "SUP-20417:" has a colon attached
    CHECK(text.find("1 line(s) redacted") != std::string::npos);
}
