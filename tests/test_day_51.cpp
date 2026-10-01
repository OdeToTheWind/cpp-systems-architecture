// Tests for Day 51 – Working with JSON.
#include <limits>
#include <sstream>
#include <stdexcept>
#include <string>

#include "cppm/testing.hpp"
#include "day_51_json/lesson.hpp"

using namespace cppm::day51;

TEST_CASE("scalars parse into the right alternatives") {
    CHECK(parse("null").is_null());
    CHECK(parse(" true ").as_bool());
    CHECK_NEAR(parse("-12.5e1").as_number(), -125.0, 1e-12);
    CHECK_EQ(parse("\"hi\"").as_string(), "hi");
    CHECK_THROWS_AS(parse("42").as_string(), std::invalid_argument);
}

TEST_CASE("objects and arrays nest and keep key order") {
    const Json doc = parse(R"({"b": [1, 2, {"c": null}], "a": {"deep": true}})");
    CHECK_EQ(doc.as_object()[0].first, "b");
    CHECK_EQ(doc.at("b").as_array().size(), 3u);
    CHECK(doc.at("b").as_array()[2].at("c").is_null());
    CHECK(doc.at("a").at("deep").as_bool());
    CHECK_THROWS_AS(doc.at("missing"), std::out_of_range);
}

TEST_CASE("escapes in strings are decoded") {
    CHECK_EQ(parse(R"("line\nbreak \"quoted\" back\\slash")").as_string(), "line\nbreak \"quoted\" back\\slash");
    CHECK_EQ(parse(R"("café")").as_string(), "caf\xC3\xA9");
    CHECK_EQ(parse(R"("A")").as_string(), "A");
}

TEST_CASE("syntax errors report line and column") {
    try {
        parse("{\n  \"a\": 1,\n  \"b\": tru\n}");
        CHECK(false);
    } catch (const JsonError& error) {
        CHECK_EQ(error.line(), 3);
        CHECK_EQ(error.column(), 8);
    }
    CHECK_THROWS_AS(parse("[1, 2"), JsonError);
    CHECK_THROWS_AS(parse("{\"a\" 1}"), JsonError);
    CHECK_THROWS_AS(parse("01"), JsonError);
    CHECK_THROWS_AS(parse("\"open"), JsonError);
    CHECK_THROWS_AS(parse("[1] extra"), JsonError);
    CHECK_THROWS_AS(parse(std::string(100, '[') + std::string(100, ']')), JsonError);
}

TEST_CASE("escape handles quotes, backslashes and control characters") {
    CHECK_EQ(escape("plain"), "\"plain\"");
    CHECK_EQ(escape("say \"hi\"\\"), "\"say \\\"hi\\\"\\\\\"");
    CHECK_EQ(escape(std::string("bell\x07")), "\"bell\\u0007\"");
}

TEST_CASE("dump produces compact or pretty output that parses back") {
    const Json doc = Json::Object{{"name", "Roses"}, {"minutes", 15}, {"tags", Json::Array{"south", true, nullptr}}};
    CHECK_EQ(dump(doc), R"({"name":"Roses","minutes":15,"tags":["south",true,null]})");
    CHECK_EQ(dump(Json::Object{{"a", Json::Array{}}}, 2), "{\n  \"a\": []\n}");
    CHECK(parse(dump(doc, 4)) == doc);
    CHECK_THROWS_AS(dump(Json(std::numeric_limits<double>::infinity())), std::domain_error);
}

TEST_CASE("load_zones reads typed, validated settings") {
    const auto zones = load_zones(parse(R"({"zones": [{"name": "Roses", "minutes": 15},
                                                      {"name": "Lawn", "minutes": 30, "enabled": false}]})"));
    REQUIRE_EQ(zones.size(), 2u);
    CHECK_EQ(zones[0].minutes, 15);
    CHECK(zones[0].enabled);
    CHECK(!zones[1].enabled);
    CHECK_THROWS_AS(load_zones(parse(R"({"zones": [{"name": "X", "minutes": 7.5}]})")), std::invalid_argument);
    CHECK_THROWS_AS(load_zones(parse(R"({"zones": [{"name": 3, "minutes": 5}]})")), std::invalid_argument);
}

TEST_CASE("run prints a pretty status report or a precise error") {
    std::istringstream good("{\"zones\": [{\"name\": \"Roses\", \"minutes\": 15},\n{\"name\": \"Lawn\", \"minutes\": 30, \"enabled\": false}]}\nEND\n");
    std::ostringstream out;
    CHECK_EQ(run(good, out), 0);
    CHECK(out.str().find("\"total_minutes\": 15") != std::string::npos);
    CHECK(out.str().find("\"zone\": \"Lawn\"") != std::string::npos);
    std::istringstream bad("{\"zones\": [\n  {\"name\": \"Roses\" \"minutes\": 15}\n]}\nEND\n");
    std::ostringstream out2;
    run(bad, out2);
    CHECK(out2.str().find("syntax error at line 2, column 20") != std::string::npos);
}
