// Tests for Day 53 – Sending Email (SMTP & MIME). No test touches the network.
#include <deque>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

#include "cppm/testing.hpp"
#include "day_53_email_smtp/lesson.hpp"

using namespace cppm::day53;

namespace {
/// A scripted server: replies come from a queue, every line the client sends is recorded.
class FakeServer final : public Transport {
  public:
    explicit FakeServer(std::deque<std::string> replies) : replies_(std::move(replies)) {}
    void send_line(const std::string& line) override { sent.push_back(line); }
    std::string read_reply() override {
        std::string reply = replies_.front();
        replies_.pop_front();
        return reply;
    }
    std::vector<std::string> sent;

  private:
    std::deque<std::string> replies_;
};
}  // namespace

TEST_CASE("address validation accepts real addresses and rejects broken ones") {
    CHECK(is_valid_address("ada@example.org"));
    CHECK(is_valid_address("first.last+tag@mail.co.uk"));
    CHECK(!is_valid_address("ada.example.org"));
    CHECK(!is_valid_address("@example.org"));
    CHECK(!is_valid_address("ada@@example.org"));
    CHECK(!is_valid_address("ada@localhost"));
    CHECK(!is_valid_address("ada @example.org"));
    CHECK(!is_valid_address("ada@example.org\r\nBcc: x@y.z"));
}

TEST_CASE("base64 matches the RFC 4648 test vectors") {
    CHECK_EQ(base64_encode(""), "");
    CHECK_EQ(base64_encode("f"), "Zg==");
    CHECK_EQ(base64_encode("fo"), "Zm8=");
    CHECK_EQ(base64_encode("foo"), "Zm9v");
    CHECK_EQ(base64_encode("foobar"), "Zm9vYmFy");
    CHECK(base64_encode(std::string(100, 'x')).find("\r\n") == 76u);
}

TEST_CASE("the MIME message has both bodies and an encoded attachment") {
    const Email email = monthly_statement("Ada", "ada@example.org", 12'345);
    const std::string mime = build_mime(email);
    CHECK(mime.find("Content-Type: multipart/mixed; boundary=\"=_coop_boundary\"") != std::string::npos);
    CHECK(mime.find("text/plain") < mime.find("text/html"));
    CHECK(mime.find("you owe 123.45 EUR") != std::string::npos);
    CHECK(mime.find("filename=\"statement.csv\"") != std::string::npos);
    CHECK(mime.find(base64_encode(email.attachments[0].bytes)) != std::string::npos);
    CHECK(mime.substr(mime.size() - 21) == "--=_coop_boundary--\r\n");
}

TEST_CASE("headers cannot be injected through the subject") {
    Email email = monthly_statement("Ada", "ada@example.org", 100);
    email.subject = "Hi\r\nBcc: everyone@example.org";
    const std::string mime = build_mime(email);
    CHECK(mime.find("\r\nBcc:") == std::string::npos);
    CHECK(mime.find("Subject: Hi  Bcc: everyone@example.org") != std::string::npos);
    email.to = {"not-an-address"};
    CHECK_THROWS_AS(build_mime(email), std::invalid_argument);
    email.to.clear();
    CHECK_THROWS_AS(build_mime(email), std::invalid_argument);
}

TEST_CASE("the SMTP client follows the protocol and dot-stuffs the body") {
    FakeServer server({"220 hi", "250 ok", "250 ok", "250 ok", "250 ok", "354 go", "250 queued", "221 bye"});
    Email email = monthly_statement("Ada", "ada@example.org", 100);
    email.to.push_back("bo@example.org");
    SmtpClient(server).send(email, "Subject: x\r\n\r\n.hidden line\r\nok\r\n");
    CHECK_EQ(server.sent[0], "EHLO statements.coop.example");
    CHECK_EQ(server.sent[1], "MAIL FROM:<accounts@coop.example>");
    CHECK_EQ(server.sent[3], "RCPT TO:<bo@example.org>");
    CHECK_EQ(server.sent[4], "DATA");
    CHECK_EQ(server.sent[7], "..hidden line");
    CHECK_EQ(server.sent[server.sent.size() - 2], ".");
    CHECK_EQ(server.sent.back(), "QUIT");
}

TEST_CASE("an unexpected reply code stops the dialogue with SmtpError") {
    FakeServer server({"220 hi", "250 ok", "250 ok", "550 mailbox unavailable"});
    const Email email = monthly_statement("Ada", "ghost@example.org", 100);
    try {
        SmtpClient(server).send(email, build_mime(email));
        CHECK(false);
    } catch (const SmtpError& error) {
        CHECK_EQ(error.code(), 550);
    }
    CHECK_EQ(server.sent.back(), "RCPT TO:<ghost@example.org>");
}

TEST_CASE("the dry-run transport records a complete, successful conversation") {
    DryRunTransport transport;
    const Email email = monthly_statement("Bo", "bo@example.org", 500);
    SmtpClient(transport).send(email, build_mime(email));
    CHECK_EQ(transport.sent.front(), "EHLO statements.coop.example");
    CHECK_EQ(transport.sent.back(), "QUIT");
}

TEST_CASE("run mails statements in dry-run mode and reports bad addresses") {
    std::istringstream in("Ada ada@example.org 12345\nBo bo-at-example 100\nstop\n");
    std::ostringstream out;
    CHECK_EQ(run(in, out), 0);
    const auto text = out.str();
    CHECK(text.find("dry run OK") != std::string::npos);
    CHECK(text.find("not sent: invalid recipient bo-at-example") != std::string::npos);
}
