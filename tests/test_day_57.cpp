// Tests for Day 57 – REST APIs & JSON. The API lives in memory; nothing touches the network.
#include <sstream>
#include <string>

#include "cppm/testing.hpp"
#include "day_57_rest_api/lesson.hpp"

using namespace cppm::day57;

namespace {
const std::string dune = R"({"title": "Dune", "author": "Frank Herbert", "year": 1965})";
}

TEST_CASE("HTTP methods differ in safety and idempotence") {
    CHECK(method_semantics("GET")->safe);
    CHECK(method_semantics("PUT")->idempotent);
    CHECK(!method_semantics("PUT")->safe);
    CHECK(!method_semantics("POST")->idempotent);
    CHECK(!method_semantics("BREW").has_value());
}

TEST_CASE("flat JSON objects parse and malformed bodies are rejected") {
    const auto fields = parse_object(R"( {"title": "A \"quoted\" title", "year": -44} )");
    REQUIRE(fields.has_value());
    CHECK_EQ(fields->at("title"), "A \"quoted\" title");
    CHECK_EQ(fields->at("year"), "-44");
    CHECK(parse_object("{}").has_value());
    CHECK(!parse_object("not json").has_value());
    CHECK(!parse_object(R"({"a": 1,})").has_value());
    CHECK(!parse_object(R"({"a": 1} trailing)").has_value());
}

TEST_CASE("POST creates a book with 201 and a Location header") {
    BookApi api;
    const auto response = api.handle({"POST", "/books", dune});
    CHECK_EQ(response.status, 201);
    CHECK_EQ(response.headers.at("Location"), "/books/1");
    CHECK_EQ(response.body, R"({"id":1,"title":"Dune","author":"Frank Herbert","year":1965})");
}

TEST_CASE("bad requests get 400, 409 and 422 as appropriate") {
    BookApi api;
    CHECK_EQ(api.handle({"POST", "/books", "{oops"}).status, 400);
    CHECK_EQ(api.handle({"POST", "/books", R"({"title": "Dune"})"}).status, 422);
    CHECK_EQ(api.handle({"POST", "/books", R"({"title": "X", "author": "Y", "year": 99})"}).status, 422);
    CHECK_EQ(api.handle({"POST", "/books", R"({"title": "X", "author": "Y", "year": "soon"})"}).status, 422);
    api.handle({"POST", "/books", dune});
    CHECK_EQ(api.handle({"POST", "/books", dune}).status, 409);
}

TEST_CASE("GET reads one book or the whole collection") {
    BookApi api;
    CHECK_EQ(api.handle({"GET", "/books", ""}).body, "[]");
    api.handle({"POST", "/books", dune});
    CHECK_EQ(api.handle({"GET", "/books/1", ""}).status, 200);
    CHECK_EQ(api.handle({"GET", "/books/2", ""}).status, 404);
    CHECK_EQ(api.handle({"GET", "/books/abc", ""}).status, 400);
    CHECK(api.handle({"GET", "/books", ""}).body.find("\"id\":1") != std::string::npos);
}

TEST_CASE("PUT replaces and is idempotent; DELETE answers 204 then 404") {
    BookApi api;
    api.handle({"POST", "/books", dune});
    const std::string updated = R"({"title": "Dune", "author": "Frank Herbert", "year": 1966})";
    CHECK_EQ(api.handle({"PUT", "/books/1", updated}).status, 200);
    CHECK_EQ(api.handle({"PUT", "/books/1", updated}).body, api.handle({"GET", "/books/1", ""}).body);
    CHECK_EQ(api.handle({"PUT", "/books/9", updated}).status, 404);
    CHECK_EQ(api.handle({"DELETE", "/books/1", ""}).status, 204);
    CHECK_EQ(api.handle({"DELETE", "/books/1", ""}).status, 404);
}

TEST_CASE("unknown paths and methods get 404 and 405") {
    BookApi api;
    CHECK_EQ(api.handle({"GET", "/authors", ""}).status, 404);
    CHECK_EQ(api.handle({"DELETE", "/books", ""}).status, 405);
    CHECK_EQ(api.handle({"BREW", "/books", ""}).status, 405);
    CHECK_EQ(api.handle({"POST", "/books/1", ""}).status, 405);
}

TEST_CASE("run prints status, headers and body for each request") {
    std::istringstream in("POST /books " + dune + "\nGET /books/1\nDELETE /books/1\nGET /books/1\n\n");
    std::ostringstream out;
    CHECK_EQ(run(in, out), 0);
    const auto text = out.str();
    CHECK(text.find("201  Location: /books/1") != std::string::npos);
    CHECK(text.find("  204\n") != std::string::npos);
    CHECK(text.find("404  {\"error\":\"book not found\"}") != std::string::npos);
}
