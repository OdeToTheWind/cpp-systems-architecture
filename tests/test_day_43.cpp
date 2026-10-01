// Tests for Day 43 – Reading and Writing CSV.
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

#include "cppm/testing.hpp"
#include "day_43_csv/lesson.hpp"

using namespace cppm::day43;

namespace {
std::vector<std::string> one(const std::string& text) {
    std::istringstream in(text);
    int line = 0;
    return read_record(in, line).value();
}
}  // namespace

TEST_CASE("plain and quoted fields, including commas and doubled quotes") {
    CHECK(one("a,b,c") == std::vector<std::string>{"a", "b", "c"});
    CHECK(one("\"Lovelace, Ada\",ada@x.org,36,10") ==
          std::vector<std::string>{"Lovelace, Ada", "ada@x.org", "36", "10"});
    CHECK(one("\"Dwayne \"\"The Rock\"\" J\",d@x,40,5")[0] == "Dwayne \"The Rock\" J");
    CHECK(one("a,,c") == std::vector<std::string>{"a", "", "c"});
    CHECK(one("a,b\r") == std::vector<std::string>{"a", "b"});
}

TEST_CASE("a quoted field may span several lines") {
    std::istringstream in("\"line one\nline two\",x\nnext,row\n");
    int line = 0;
    const auto first = read_record(in, line);
    REQUIRE(first.has_value());
    CHECK_EQ(first->at(0), "line one\nline two");
    CHECK_EQ(line, 2);
    CHECK_EQ(read_record(in, line)->at(0), "next");
    CHECK(!read_record(in, line).has_value());
}

TEST_CASE("an unterminated quote is an error, not an endless read") {
    std::istringstream in("\"never closed,1,2\n");
    int line = 0;
    CHECK_THROWS_AS(read_record(in, line), std::runtime_error);
}

TEST_CASE("validate_row accepts good rows and explains bad ones") {
    CHECK(validate_row({"Ada", "ada@x.org", "36", "10"}).first.has_value());
    CHECK_EQ(validate_row({"Ada", "ada@x.org", "36"}).second, "expected 4 fields, found 3");
    CHECK_EQ(validate_row({"", "a@b", "36", "10"}).second, "name is empty");
    CHECK_EQ(validate_row({"Bo", "nope", "36", "10"}).second, "email 'nope' has no @");
    CHECK_EQ(validate_row({"Bo", "b@x", "36y", "10"}).second, "age and distance must be whole numbers");
    CHECK_EQ(validate_row({"Bo", "b@x", "120", "10"}).second, "age 120 is outside 8-99");
    CHECK_EQ(validate_row({"Bo", "b@x", "30", "42"}).second, "distance must be 5, 10 or 21 km");
}

TEST_CASE("import keeps going and reports bad rows by line number") {
    std::istringstream csv(
        "name,email,age,distance_km\n"
        "\"Lovelace, Ada\",ada@x.org,36,10\n"
        "Broken row\n"
        "\n"
        "Grace,grace@navy.mil,85,5\n"
        "Kid,kid@x.org,6,5\n");
    const auto result = import_runners(csv);
    REQUIRE_EQ(result.runners.size(), 2u);
    CHECK_EQ(result.runners[0].name, "Lovelace, Ada");
    REQUIRE_EQ(result.errors.size(), 2u);
    CHECK_EQ(result.errors[0].first, 3);
    CHECK_EQ(result.errors[1].first, 6);
}

TEST_CASE("csv_escape quotes only when needed") {
    CHECK_EQ(csv_escape("plain"), "plain");
    CHECK_EQ(csv_escape("Lovelace, Ada"), "\"Lovelace, Ada\"");
    CHECK_EQ(csv_escape("say \"hi\""), "\"say \"\"hi\"\"\"");
    CHECK_EQ(csv_escape("two\nlines"), "\"two\nlines\"");
}

TEST_CASE("written CSV parses back to the same runners") {
    const std::vector<Runner> runners{{"Lovelace, Ada", "ada@x.org", 36, 10}, {"O\"Neil", "o@x.ie", 50, 21}};
    std::ostringstream out;
    write_runners(out, runners);
    std::istringstream back(out.str());
    const auto result = import_runners(back);
    REQUIRE_EQ(result.runners.size(), 2u);
    CHECK_EQ(result.runners[0].name, "Lovelace, Ada");
    CHECK_EQ(result.runners[1].name, "O\"Neil");
    CHECK(result.errors.empty());
}

TEST_CASE("run reports errors, counts per distance and prints clean CSV") {
    std::istringstream in("name,email,age,distance_km\nAda,a@x,36,10\nBo,b@x,7,5\nCy,c@x,30,10\nEND\n");
    std::ostringstream out;
    CHECK_EQ(run(in, out), 0);
    const auto text = out.str();
    CHECK(text.find("line 3: age 7 is outside 8-99") != std::string::npos);
    CHECK(text.find("10 km: 2 runner(s)") != std::string::npos);
    CHECK(text.find("name,email,age,distance_km\nAda,a@x,36,10\nCy,c@x,30,10\n") != std::string::npos);
}
