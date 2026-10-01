// Tests for Day 55 – Hosting C++ Online. CGI requests are simulated with plain maps.
#include <sstream>
#include <stdexcept>
#include <string>

#include "cppm/testing.hpp"
#include "day_55_hosting/lesson.hpp"

using namespace cppm::day55;

namespace {
Response call(const Router& app, const std::string& method, const std::string& path, const std::string& query = "") {
    return app.handle(request_from_cgi({{"REQUEST_METHOD", method}, {"PATH_INFO", path}, {"QUERY_STRING", query}}));
}
}  // namespace

TEST_CASE("query strings are split and percent-decoded") {
    const auto q = parse_query("word=hello%20world&lang=fr&empty=&flag&plus=a+b&bad=%zz");
    CHECK_EQ(q.at("word"), "hello world");
    CHECK_EQ(q.at("lang"), "fr");
    CHECK_EQ(q.at("empty"), "");
    CHECK_EQ(q.at("flag"), "");
    CHECK_EQ(q.at("plus"), "a b");
    CHECK_EQ(q.at("bad"), "%zz");
    CHECK(parse_query("").empty());
}

TEST_CASE("the request comes from CGI variables with sensible defaults") {
    const Request request = request_from_cgi({{"REQUEST_METHOD", "POST"}, {"PATH_INFO", "/word"}, {"QUERY_STRING", "a=1"}});
    CHECK_EQ(request.method, "POST");
    CHECK_EQ(request.path, "/word");
    CHECK_EQ(request.query.at("a"), "1");
    const Request bare = request_from_cgi({});
    CHECK_EQ(bare.method, "GET");
    CHECK_EQ(bare.path, "/");
}

TEST_CASE("the router answers 200, 404 and 405") {
    const Router app = make_app(Config{});
    CHECK_EQ(call(app, "GET", "/health").status, 200);
    CHECK_EQ(call(app, "GET", "/nope").status, 404);
    CHECK_EQ(call(app, "DELETE", "/health").status, 405);
}

TEST_CASE("the word endpoint uses query parameters and the configured default") {
    const Router app = make_app(Config{"Test School", "es", false});
    CHECK_EQ(call(app, "GET", "/word").body, R"({"word":"hola","meaning":"hello"})");
    CHECK_EQ(call(app, "GET", "/word", "lang=de&day=1").body, R"({"word":"danke","meaning":"thank you"})");
    CHECK_EQ(call(app, "GET", "/word", "lang=fr&day=3").body, R"({"word":"merci","meaning":"thank you"})");
    CHECK_EQ(call(app, "GET", "/word", "lang=xx").status, 400);
    CHECK_EQ(call(app, "GET", "/word", "day=-1").status, 400);
    CHECK_EQ(call(app, "GET", "/word", "day=soon").status, 400);
    CHECK(call(app, "GET", "/").body.find("Test School") != std::string::npos);
}

TEST_CASE("a throwing handler becomes a 500 without leaking details") {
    Router router;
    router.add("GET", "/boom", [](const Request&) -> Response { throw std::runtime_error("db password wrong"); });
    const Response response = router.handle({"GET", "/boom", {}});
    CHECK_EQ(response.status, 500);
    CHECK(response.body.find("password") == std::string::npos);
}

TEST_CASE("render_cgi writes status, headers, a blank line and the body") {
    CHECK_EQ(render_cgi({404, "text/plain; charset=utf-8", "not found"}),
             "Status: 404 Not Found\r\nContent-Type: text/plain; charset=utf-8\r\nContent-Length: 9\r\n\r\nnot found");
}

TEST_CASE("configuration comes from the environment and is validated") {
    const Config defaults = load_config({});
    CHECK_EQ(defaults.school_name, "Lingua Lab");
    CHECK_EQ(defaults.default_language, "fr");
    const Config custom = load_config({{"WOTD_SCHOOL", "Polyglot"}, {"WOTD_LANGUAGE", "de"}, {"WOTD_DEBUG", "true"}});
    CHECK_EQ(custom.school_name, "Polyglot");
    CHECK(custom.debug);
    CHECK_THROWS_AS(load_config({{"WOTD_LANGUAGE", "klingon"}}), std::invalid_argument);
}

TEST_CASE("run serves simulated CGI requests") {
    std::istringstream in("GET /health\nGET /word?lang=es&day=1\nPUT /\nstop\n");
    std::ostringstream out;
    CHECK_EQ(run(in, out), 0);
    const auto text = out.str();
    CHECK(text.find("Status: 200 OK\r\nContent-Type: text/plain; charset=utf-8\r\nContent-Length: 2\r\n\r\nok") !=
          std::string::npos);
    CHECK(text.find("\"word\":\"gracias\"") != std::string::npos);
    CHECK(text.find("Status: 405 Method Not Allowed") != std::string::npos);
}
