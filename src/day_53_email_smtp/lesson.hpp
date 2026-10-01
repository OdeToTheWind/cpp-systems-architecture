/**
 * @file
 * Day 53 – Sending Email (SMTP & MIME).
 *
 * Scenario: a *housing co-op's monthly statement mailer*. Each member gets a MIME message with
 * a plain-text and an HTML version plus a CSV attachment. Addresses are validated, header
 * injection is impossible, and the SMTP conversation runs over an injectable transport – a real
 * socket in production, a scripted fake in tests, or a dry run that only prints the dialogue.
 *
 * Deliverables (syllabus):
 * - Building MIME messages
 * - Address validation
 * - The SMTP dialogue over an injectable transport
 * - Dry runs
 */
#pragma once

#include <algorithm>
#include <cctype>
#include <cstddef>
#include <deque>
#include <istream>
#include <ostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

#include "cppm/lesson.hpp"

namespace cppm::day53 {

inline constexpr Deliverable DELIVERABLES[] = {
    {"validating email addresses", "is_valid_address"},
    {"base64 for attachments", "base64_encode"},
    {"assembling a multipart MIME message", "build_mime"},
    {"the transport interface that hides the network", "Transport"},
    {"the SMTP client state machine", "SmtpClient::send"},
    {"a dry-run transport that only records", "DryRunTransport"},
};

/// Pragmatic checks: one '@', a non-empty local part, a dotted domain, no spaces or control characters.
inline bool is_valid_address(std::string_view address) {
    const auto at = address.find('@');
    if (at == std::string_view::npos || at == 0 || address.find('@', at + 1) != std::string_view::npos) return false;
    const auto domain = address.substr(at + 1);
    const auto dot = domain.find('.');
    if (dot == std::string_view::npos || dot == 0 || domain.back() == '.') return false;
    return std::none_of(address.begin(), address.end(),
                        [](char c) { return std::isspace(static_cast<unsigned char>(c)) || std::iscntrl(static_cast<unsigned char>(c)); });
}

/// RFC 4648 base64, wrapped at 76 characters per line as MIME requires.
inline std::string base64_encode(std::string_view data) {
    static constexpr char alphabet[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    std::string out;
    std::size_t line_length = 0;
    for (std::size_t i = 0; i < data.size(); i += 3) {
        const auto byte = [&](std::size_t k) { return k < data.size() ? static_cast<unsigned char>(data[k]) : 0u; };
        const unsigned triple = (byte(i) << 16) | (byte(i + 1) << 8) | byte(i + 2);
        const std::size_t real = std::min<std::size_t>(3, data.size() - i);
        for (std::size_t k = 0; k < 4; ++k) {
            out += k <= real ? alphabet[(triple >> (18 - 6 * k)) & 0x3F] : '=';
            if (++line_length == 76) {
                out += "\r\n";
                line_length = 0;
            }
        }
    }
    return out;
}

struct Attachment {
    std::string filename;
    std::string content_type;
    std::string bytes;
};

struct Email {
    std::string from;
    std::vector<std::string> to;
    std::string subject;
    std::string text;
    std::string html;
    std::vector<Attachment> attachments;
};

/// A header value must never contain CR or LF – otherwise a crafted subject could add headers.
inline std::string header_safe(std::string value) {
    std::replace_if(value.begin(), value.end(), [](char c) { return c == '\r' || c == '\n'; }, ' ');
    return value;
}

/// multipart/mixed { multipart/alternative { text, html }, attachments… } with CRLF line endings.
inline std::string build_mime(const Email& email, const std::string& boundary = "=_coop_boundary") {
    if (!is_valid_address(email.from)) throw std::invalid_argument("invalid sender " + email.from);
    if (email.to.empty()) throw std::invalid_argument("no recipients");
    for (const auto& address : email.to) {
        if (!is_valid_address(address)) throw std::invalid_argument("invalid recipient " + address);
    }
    const std::string alt = boundary + "_alt";
    std::string to_header;
    for (const auto& address : email.to) to_header += (to_header.empty() ? "" : ", ") + address;
    std::ostringstream m;
    m << "From: " << email.from << "\r\nTo: " << to_header << "\r\nSubject: " << header_safe(email.subject)
      << "\r\nMIME-Version: 1.0\r\nContent-Type: multipart/mixed; boundary=\"" << boundary << "\"\r\n\r\n"
      << "--" << boundary << "\r\nContent-Type: multipart/alternative; boundary=\"" << alt << "\"\r\n\r\n"
      << "--" << alt << "\r\nContent-Type: text/plain; charset=utf-8\r\n\r\n" << email.text << "\r\n"
      << "--" << alt << "\r\nContent-Type: text/html; charset=utf-8\r\n\r\n" << email.html << "\r\n"
      << "--" << alt << "--\r\n";
    for (const auto& file : email.attachments) {
        m << "--" << boundary << "\r\nContent-Type: " << file.content_type << "; name=\"" << header_safe(file.filename)
          << "\"\r\nContent-Transfer-Encoding: base64\r\nContent-Disposition: attachment; filename=\""
          << header_safe(file.filename) << "\"\r\n\r\n" << base64_encode(file.bytes) << "\r\n";
    }
    m << "--" << boundary << "--\r\n";
    return m.str();
}

/// The only thing SmtpClient knows about the network.
class Transport {
public:
    virtual ~Transport() = default;
    virtual void send_line(const std::string& line) = 0;
    virtual std::string read_reply() = 0;  // e.g. "250 OK"
};

/// An SMTP reply code that was not the one expected.
class SmtpError : public std::runtime_error {
public:
    SmtpError(int code, const std::string& reply) : std::runtime_error("SMTP error: " + reply), code_(code) {}
    int code() const noexcept { return code_; }

private:
    int code_;
};

class SmtpClient {
public:
    explicit SmtpClient(Transport& transport) : transport_(transport) {}

    /// The SMTP dialogue: greeting, EHLO, MAIL FROM, RCPT TO per recipient, DATA, message, ".", QUIT.
    void send(const Email& email, const std::string& mime) {
        expect(220);
        command("EHLO statements.coop.example", 250);
        command("MAIL FROM:<" + email.from + ">", 250);
        for (const auto& recipient : email.to) {
            command("RCPT TO:<" + recipient + ">", 250);
        }
        command("DATA", 354);
        std::istringstream lines(mime);
        std::string line;
        while (std::getline(lines, line)) {
            if (!line.empty() && line.back() == '\r') line.pop_back();
            transport_.send_line(line.rfind('.', 0) == 0 ? "." + line : line);  // dot-stuffing
        }
        command(".", 250);
        command("QUIT", 221);
    }

private:
    void command(const std::string& line, int expected) {
        transport_.send_line(line);
        expect(expected);
    }
    void expect(int expected) {
        const std::string reply = transport_.read_reply();
        const int code = reply.size() >= 3 ? std::stoi(reply.substr(0, 3)) : 0;
        if (code != expected) throw SmtpError(code, reply);
    }
    Transport& transport_;
};

/// Pretends to be a happy SMTP server and records the conversation instead of sending anything.
class DryRunTransport final : public Transport {
public:
    void send_line(const std::string& line) override {
        sent.push_back(line);
        if (line == "DATA") reply_ = "354 End data with <CR><LF>.<CR><LF>";
        else if (line == "QUIT") reply_ = "221 Bye";
        else reply_ = "250 OK";
    }
    std::string read_reply() override {
        if (first_) {
            first_ = false;
            return "220 dry-run ready";
        }
        return reply_;
    }
    std::vector<std::string> sent;

private:
    bool first_{true};
    std::string reply_;
};

inline Email monthly_statement(const std::string& member, const std::string& address, long long due_cents) {
    const std::string amount = std::to_string(due_cents / 100) + "." + (due_cents % 100 < 10 ? "0" : "") + std::to_string(due_cents % 100);
    return {"accounts@coop.example", {address}, "Your statement for May",
            "Hello " + member + ",\nyou owe " + amount + " EUR this month.",
            "<p>Hello " + member + ",</p><p>you owe <b>" + amount + " EUR</b> this month.</p>",
            {{"statement.csv", "text/csv", "item,amount\nservice charge," + amount + "\n"}}};
}

/// The interactive demo: "member address cents" lines are mailed through a dry-run transport.
inline int run(std::istream& in, std::ostream& out) {
    out << "Day 53 – Sending Email (SMTP & MIME)\n";
    while (auto line = prompt_line(in, out, "member address cents> ")) {
        std::istringstream words(*line);
        std::string member;
        std::string address;
        long long cents = 0;
        if (!(words >> member >> address >> cents)) break;
        try {
            const Email email = monthly_statement(member, address, cents);
            DryRunTransport transport;
            SmtpClient(transport).send(email, build_mime(email));
            out << "  dry run OK: " << transport.sent.size() << " lines sent, first: " << transport.sent.front() << '\n';
        } catch (const std::exception& error) {
            out << "  not sent: " << error.what() << '\n';
        }
    }
    return 0;
}

}  // namespace cppm::day53
