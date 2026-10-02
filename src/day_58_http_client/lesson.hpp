/**
 * @file
 * Day 58 – HTTP Requests.
 *
 * Scenario: a *public-transport departures client* for a station display. It writes correct
 * HTTP/1.1 requests, parses status lines, headers and both plain and chunked bodies, and retries
 * timeouts and temporary server errors with exponential back-off – all over an injectable
 * transport, so tests use canned responses and never wait or touch the network.
 *
 * Deliverables (syllabus):
 * - HTTP/1.1 request and response formats
 * - Parsing status lines and headers
 * - Timeouts and retries over an injectable transport
 */
#pragma once

#include <algorithm>
#include <cctype>
#include <chrono>
#include <cstddef>
#include <functional>
#include <istream>
#include <map>
#include <optional>
#include <ostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

#include "cppm/lesson.hpp"

namespace cppm::day58 {

inline constexpr Deliverable DELIVERABLES[] = {
    {"writing an HTTP/1.1 request", "format_request"},
    {"parsing the status line and case-insensitive headers", "parse_response"},
    {"decoding a chunked body", "decode_chunked"},
    {"the transport interface and its failure types", "Transport"},
    {"retries with exponential back-off and Retry-After", "HttpClient::get"},
};

/// Header names are case-insensitive, so they are stored lower-cased.
inline std::string lower(std::string_view text) {
    std::string out(text);
    std::transform(out.begin(), out.end(), out.begin(),
                   [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    return out;
}

/// "GET /path HTTP/1.1" + Host + headers + a blank line, every line ending in CRLF.
inline std::string format_request(const std::string& method, const std::string& host, const std::string& target,
                                  const std::map<std::string, std::string>& headers = {}) {
    std::string request = method + " " + target + " HTTP/1.1\r\nHost: " + host + "\r\n";
    for (const auto& [name, value] : headers) request += name + ": " + value + "\r\n";
    return request + "Connection: close\r\n\r\n";
}

struct HttpResponse {
    int status{0};
    std::string reason;
    std::map<std::string, std::string> headers;  // lower-case names
    std::string body;
    std::optional<std::string> header(std::string_view name) const {
        const auto it = headers.find(lower(name));
        return it == headers.end() ? std::nullopt : std::optional<std::string>(it->second);
    }
};

/// "4\r\nWiki\r\n5\r\npedia\r\n0\r\n\r\n" -> "Wikipedia".
inline std::string decode_chunked(std::string_view raw) {
    std::string body;
    std::size_t at = 0;
    while (true) {
        const auto line_end = raw.find("\r\n", at);
        if (line_end == std::string_view::npos) throw std::runtime_error("truncated chunk size");
        const std::string size_text(raw.substr(at, line_end - at));
        std::size_t size = 0;
        try {
            size = std::stoul(size_text, nullptr, 16);
        } catch (const std::exception&) {
            throw std::runtime_error("bad chunk size '" + size_text + "'");
        }
        at = line_end + 2;
        if (size == 0) return body;
        if (at + size + 2 > raw.size()) throw std::runtime_error("truncated chunk");
        body.append(raw.substr(at, size));
        at += size + 2;  // skip the chunk and its CRLF
    }
}

/// Parse a raw HTTP/1.1 response. Throws std::runtime_error for malformed input.
inline HttpResponse parse_response(std::string_view raw) {
    const auto head_end = raw.find("\r\n\r\n");
    if (head_end == std::string_view::npos) throw std::runtime_error("no end of headers");
    std::istringstream head{std::string(raw.substr(0, head_end))};
    HttpResponse response;
    std::string version;
    std::string status_line;
    std::getline(head, status_line);
    std::istringstream status(status_line);
    if (!(status >> version >> response.status) || version.rfind("HTTP/1.", 0) != 0 || response.status < 100 ||
        response.status > 599) {
        throw std::runtime_error("bad status line: " + status_line);
    }
    std::getline(status >> std::ws, response.reason);
    if (!response.reason.empty() && response.reason.back() == '\r') response.reason.pop_back();
    for (std::string line; std::getline(head, line);) {
        if (!line.empty() && line.back() == '\r') line.pop_back();
        const auto colon = line.find(':');
        if (colon == std::string::npos) throw std::runtime_error("bad header line: " + line);
        std::string value = line.substr(colon + 1);
        value.erase(0, value.find_first_not_of(' '));
        response.headers[lower(line.substr(0, colon))] = value;
    }
    const std::string_view rest = raw.substr(head_end + 4);
    if (lower(response.header("transfer-encoding").value_or("")) == "chunked") {
        response.body = decode_chunked(rest);
    } else if (const auto length = response.header("content-length")) {
        const auto n = std::stoul(*length);
        if (n > rest.size()) throw std::runtime_error("body shorter than Content-Length");
        response.body = std::string(rest.substr(0, n));
    } else {
        response.body = std::string(rest);
    }
    return response;
}

class TimeoutError : public std::runtime_error {
  public:
    using std::runtime_error::runtime_error;
};
class ConnectionError : public std::runtime_error {
  public:
    using std::runtime_error::runtime_error;
};

/// Sends raw request bytes and returns raw response bytes; may throw TimeoutError or ConnectionError.
class Transport {
  public:
    virtual ~Transport() = default;
    virtual std::string exchange(const std::string& host, const std::string& request,
                                 std::chrono::milliseconds timeout) = 0;
};

struct RetryPolicy {
    int max_attempts{3};
    std::chrono::milliseconds first_delay{200};
    std::chrono::milliseconds timeout{2000};
};

class HttpClient {
  public:
    using Sleeper = std::function<void(std::chrono::milliseconds)>;

    HttpClient(Transport& transport, std::string host, RetryPolicy policy, Sleeper sleep)
        : transport_(transport), host_(std::move(host)), policy_(policy), sleep_(std::move(sleep)) {}

    /// GET with retries on timeouts, connection errors and 502/503/504. 4xx answers are returned at once,
    /// because repeating a bad request cannot make it good. A Retry-After header overrides the back-off.
    HttpResponse get(const std::string& target) {
        auto delay = policy_.first_delay;
        for (int attempt = 1;; ++attempt) {
            std::optional<std::chrono::milliseconds> wait;
            try {
                const auto raw = transport_.exchange(
                    host_,
                    format_request("GET", host_, target,
                                   {{"Accept", "application/json"}, {"User-Agent", "departures/1.0"}}),
                    policy_.timeout);
                HttpResponse response = parse_response(raw);
                if (response.status != 502 && response.status != 503 && response.status != 504) return response;
                if (attempt == policy_.max_attempts) return response;
                if (const auto after = response.header("retry-after")) wait = std::chrono::seconds{std::stoi(*after)};
            } catch (const TimeoutError&) {
                if (attempt == policy_.max_attempts) throw;
            } catch (const ConnectionError&) {
                if (attempt == policy_.max_attempts) throw;
            }
            sleep_(wait.value_or(delay));
            delay *= 2;  // exponential back-off: 200 ms, 400 ms, 800 ms …
        }
    }

  private:
    Transport& transport_;
    std::string host_;
    RetryPolicy policy_;
    Sleeper sleep_;
};

/// A canned server for the demo and the tests: replays scripted outcomes in order.
class ScriptedTransport final : public Transport {
  public:
    explicit ScriptedTransport(std::vector<std::string> script) : script_(std::move(script)) {}
    std::string exchange(const std::string&, const std::string& request, std::chrono::milliseconds) override {
        requests.push_back(request);
        if (next_ >= script_.size()) throw ConnectionError("no more scripted responses");
        const std::string& step = script_[next_++];
        if (step == "TIMEOUT") throw TimeoutError("timed out");
        if (step == "REFUSED") throw ConnectionError("connection refused");
        return step;
    }
    std::vector<std::string> requests;

  private:
    std::vector<std::string> script_;
    std::size_t next_{0};
};

inline std::string ok_json(const std::string& body) {
    return "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\nContent-Length: " + std::to_string(body.size()) +
           "\r\n\r\n" + body;
}

/// The interactive demo: describe a sequence of server behaviours, then fetch departures through them.
inline int run(std::istream& in, std::ostream& out) {
    out << "Day 58 – HTTP Requests\nServer script (e.g. TIMEOUT 503 OK), blank for OK: ";
    std::vector<std::string> script;
    if (auto line = prompt_line(in, out, "")) {
        std::istringstream words(*line);
        for (std::string step; words >> step;) {
            if (step == "OK")
                script.push_back(ok_json(R"([{"line":"S1","in_min":3}])"));
            else if (step == "503")
                script.push_back("HTTP/1.1 503 Service Unavailable\r\nContent-Length: 0\r\n\r\n");
            else
                script.push_back(step);
        }
    }
    if (script.empty()) script.push_back(ok_json(R"([{"line":"S1","in_min":3}])"));
    ScriptedTransport transport(script);
    std::chrono::milliseconds waited{0};
    HttpClient client(transport, "api.transit.example", {}, [&](std::chrono::milliseconds d) { waited += d; });
    try {
        const HttpResponse response = client.get("/departures?stop=central");
        out << "  " << response.status << ' ' << response.reason << " after " << transport.requests.size()
            << " attempt(s), waited " << waited.count() << " ms: " << response.body << '\n';
    } catch (const std::exception& error) {
        out << "  gave up after " << transport.requests.size() << " attempt(s): " << error.what() << '\n';
    }
    return 0;
}

}  // namespace cppm::day58
