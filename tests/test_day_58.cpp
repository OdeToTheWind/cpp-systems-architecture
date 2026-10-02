// Tests for Day 58 – HTTP Requests. A scripted transport replaces the network; sleeping is recorded, not done.
#include <chrono>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

#include "cppm/testing.hpp"
#include "day_58_http_client/lesson.hpp"

using namespace cppm::day58;
using namespace std::chrono_literals;

TEST_CASE("requests use the HTTP/1.1 wire format") {
    const auto request =
        format_request("GET", "api.transit.example", "/departures?stop=7", {{"Accept", "application/json"}});
    CHECK_EQ(request,
             "GET /departures?stop=7 HTTP/1.1\r\nHost: api.transit.example\r\nAccept: application/json\r\n"
             "Connection: close\r\n\r\n");
}

TEST_CASE("responses are parsed into status, reason, headers and body") {
    const auto response = parse_response(
        "HTTP/1.1 404 Not Found\r\nContent-Type: text/plain\r\nX-Trace:   abc\r\n"
        "Content-Length: 4\r\n\r\nnopeEXTRA");
    CHECK_EQ(response.status, 404);
    CHECK_EQ(response.reason, "Not Found");
    CHECK_EQ(response.header("content-type").value_or(""), "text/plain");
    CHECK_EQ(response.header("X-TRACE").value_or(""), "abc");
    CHECK_EQ(response.body, "nope");
}

TEST_CASE("malformed responses are rejected") {
    CHECK_THROWS_AS(parse_response("HTTP/1.1 200 OK\r\n"), std::runtime_error);
    CHECK_THROWS_AS(parse_response("SPDY 200 OK\r\n\r\n"), std::runtime_error);
    CHECK_THROWS_AS(parse_response("HTTP/1.1 999 Weird\r\n\r\n"), std::runtime_error);
    CHECK_THROWS_AS(parse_response("HTTP/1.1 200 OK\r\nNoColon\r\n\r\n"), std::runtime_error);
    CHECK_THROWS_AS(parse_response("HTTP/1.1 200 OK\r\nContent-Length: 10\r\n\r\nshort"), std::runtime_error);
}

TEST_CASE("chunked bodies are reassembled") {
    CHECK_EQ(decode_chunked("4\r\nWiki\r\n5\r\npedia\r\nE\r\n in\r\n\r\nchunks.\r\n0\r\n\r\n"),
             "Wikipedia in\r\n\r\nchunks.");
    const auto response = parse_response("HTTP/1.1 200 OK\r\nTransfer-Encoding: chunked\r\n\r\n3\r\nabc\r\n0\r\n\r\n");
    CHECK_EQ(response.body, "abc");
    CHECK_THROWS_AS(decode_chunked("zz\r\n"), std::runtime_error);
    CHECK_THROWS_AS(decode_chunked("5\r\nab"), std::runtime_error);
}

TEST_CASE("timeouts and 503s are retried with exponential back-off") {
    ScriptedTransport transport({"TIMEOUT", "HTTP/1.1 503 Busy\r\nContent-Length: 0\r\n\r\n", ok_json("[]")});
    std::vector<std::chrono::milliseconds> sleeps;
    HttpClient client(transport, "h", {3, 200ms, 1000ms}, [&](auto d) { sleeps.push_back(d); });
    const auto response = client.get("/x");
    CHECK_EQ(response.status, 200);
    CHECK_EQ(transport.requests.size(), 3u);
    CHECK(sleeps == std::vector<std::chrono::milliseconds>{200ms, 400ms});
}

TEST_CASE("Retry-After overrides the back-off delay") {
    ScriptedTransport transport({"HTTP/1.1 503 Busy\r\nRetry-After: 2\r\nContent-Length: 0\r\n\r\n", ok_json("[]")});
    std::vector<std::chrono::milliseconds> sleeps;
    HttpClient client(transport, "h", {}, [&](auto d) { sleeps.push_back(d); });
    client.get("/x");
    CHECK(sleeps == std::vector<std::chrono::milliseconds>{2000ms});
}

TEST_CASE("client errors are not retried; persistent failures give up") {
    ScriptedTransport not_found({"HTTP/1.1 404 Not Found\r\nContent-Length: 0\r\n\r\n", ok_json("[]")});
    HttpClient a(not_found, "h", {}, [](auto) {});
    CHECK_EQ(a.get("/x").status, 404);
    CHECK_EQ(not_found.requests.size(), 1u);
    ScriptedTransport dead({"REFUSED", "REFUSED", "REFUSED"});
    HttpClient b(dead, "h", {}, [](auto) {});
    CHECK_THROWS_AS(b.get("/x"), ConnectionError);
    CHECK_EQ(dead.requests.size(), 3u);
    ScriptedTransport busy(
        {"HTTP/1.1 503 Busy\r\nContent-Length: 0\r\n\r\n", "HTTP/1.1 504 Gateway\r\nContent-Length: 0\r\n\r\n"});
    HttpClient c(busy, "h", {2, 10ms, 10ms}, [](auto) {});
    CHECK_EQ(c.get("/x").status, 504);
}

TEST_CASE("run fetches departures through a scripted server") {
    std::istringstream in("TIMEOUT 503 OK\n");
    std::ostringstream out;
    CHECK_EQ(run(in, out), 0);
    CHECK(out.str().find("200 OK after 3 attempt(s), waited 600 ms") != std::string::npos);
    std::istringstream bad("TIMEOUT TIMEOUT TIMEOUT\n");
    std::ostringstream out2;
    run(bad, out2);
    CHECK(out2.str().find("gave up after 3 attempt(s): timed out") != std::string::npos);
}
