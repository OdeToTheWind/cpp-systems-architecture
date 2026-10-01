// Tests for Day 59 – Query Parameters, Headers & Payloads. Requests are prepared and inspected offline.
#include <sstream>
#include <stdexcept>
#include <string>

#include "cppm/testing.hpp"
#include "day_59_request_payloads/lesson.hpp"

using namespace cppm::day59;

TEST_CASE("percent-encoding keeps unreserved characters and encodes the rest") {
    CHECK_EQ(percent_encode("abc-XYZ_0.9~"), "abc-XYZ_0.9~");
    CHECK_EQ(percent_encode("a b&c=d"), "a%20b%26c%3Dd");
    CHECK_EQ(percent_encode("S\xC3\xA3o"), "S%C3%A3o");  // UTF-8 bytes of "São"
    CHECK_EQ(percent_encode("a b", true), "a+b");
    CHECK_EQ(percent_encode("100%"), "100%25");
}

TEST_CASE("query strings keep order and allow repeated keys") {
    CHECK_EQ(query_string({{"amenity", "pool"}, {"amenity", "gym"}, {"q", "sea view"}}),
             "amenity=pool&amenity=gym&q=sea%20view");
    CHECK_EQ(query_string({}), "");
}

TEST_CASE("headers are case-insensitive and replace on set") {
    Headers headers;
    headers.set("Accept", "text/html");
    headers.set("accept", "application/json");
    CHECK_EQ(headers.items().size(), 1u);
    CHECK_EQ(headers.get("ACCEPT").value_or(""), "application/json");
    CHECK(!headers.get("X-Missing").has_value());
    CHECK_THROWS_AS(headers.set("X-Bad", "a\r\nInjected: yes"), std::invalid_argument);
}

TEST_CASE("search requests encode every filter into the target") {
    const auto request = search_request("São Paulo", "2026-07-01", 2, {"pool", "wifi"});
    CHECK_EQ(request.target, "/v2/search?city=S%C3%A3o%20Paulo&check_in=2026-07-01&guests=2&amenity=pool&amenity=wifi");
    CHECK_EQ(request.headers.get("Accept").value_or(""), "application/json");
    CHECK(!request.headers.get("Content-Length").has_value());
}

TEST_CASE("form bodies are url-encoded with plus signs and sized") {
    const auto request =
        RequestBuilder("POST", "h", "/contact").form({{"name", "Ada Lovelace"}, {"msg", "50% off?"}}).prepare();
    CHECK_EQ(request.body, "name=Ada+Lovelace&msg=50%25+off%3F");
    CHECK_EQ(request.headers.get("Content-Type").value_or(""), "application/x-www-form-urlencoded");
    CHECK_EQ(request.headers.get("Content-Length").value_or(""), std::to_string(request.body.size()));
}

TEST_CASE("JSON bodies set their content headers") {
    const auto request = RequestBuilder("POST", "h", "/b").json(R"({"a":1})").header("Accept", "text/plain").prepare();
    CHECK_EQ(request.headers.get("content-type").value_or(""), "application/json");
    CHECK_EQ(request.headers.get("Content-Length").value_or(""), "7");
    CHECK_EQ(request.headers.get("Accept").value_or(""), "text/plain");  // not overridden
}

TEST_CASE("the wire format is exact and secrets can be masked") {
    auto request = RequestBuilder("GET", "api.example", "/x").param("a", "1").prepare();
    request.headers.set("Authorization", "Bearer secret-token");
    const auto wire = request.wire();
    CHECK(wire.rfind("GET /x?a=1 HTTP/1.1\r\nHost: api.example\r\n", 0) == 0);
    CHECK(wire.find("Authorization: Bearer secret-token\r\n") != std::string::npos);
    CHECK(wire.substr(wire.size() - 4) == "\r\n\r\n");
    CHECK(request.wire(true).find("secret-token") == std::string::npos);
}

TEST_CASE("run prints masked search requests and a JSON booking") {
    std::istringstream in("Lisbon 2026-08-01 3 pool\nstop\n");
    std::ostringstream out;
    CHECK_EQ(run(in, out), 0);
    const auto text = out.str();
    CHECK(text.find("GET /v2/search?city=Lisbon&check_in=2026-08-01&guests=3&amenity=pool HTTP/1.1") != std::string::npos);
    CHECK(text.find("X-Api-Key: ***") != std::string::npos);
    CHECK(text.find("sk_live") == std::string::npos);
    CHECK(text.find("Content-Length: 26") != std::string::npos);
}
