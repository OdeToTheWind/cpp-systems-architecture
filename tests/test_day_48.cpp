// Tests for Day 48 – Static vs Dynamic Typing.
#include <any>
#include <sstream>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <variant>

#include "cppm/testing.hpp"
#include "day_48_static_dynamic_typing/lesson.hpp"

using namespace cppm::day48;

template <typename A, typename B>
concept Addable = requires(A a, B b) { a + b; };

TEST_CASE("strong unit types are checked at compile time") {
    static_assert(Addable<Length<Metres>, Length<Metres>>);
    static_assert(!Addable<Length<Metres>, Length<Feet>>);  // mixing units does not compile
    constexpr auto total = Length<Metres>{1} + to_metres(Length<Feet>{10});
    CHECK_NEAR(total.value, 4.048, 1e-12);
}

TEST_CASE("parse_cell picks the alternative from the text") {
    CHECK(std::holds_alternative<std::monostate>(parse_cell("")));
    CHECK(std::get<double>(parse_cell("42.5")) == 42.5);
    CHECK(std::get<bool>(parse_cell("TRUE")));
    CHECK_EQ(std::get<std::string>(parse_cell("Rent")), "Rent");
    CHECK_EQ(std::get<std::string>(parse_cell("12 apples")), "12 apples");
    CHECK(std::get<CellError>(parse_cell("#N/A")) == CellError{"#N/A"});
}

TEST_CASE("formulas produce numbers or spreadsheet errors") {
    CHECK_NEAR(std::get<double>(parse_cell("=10/4")), 2.5, 1e-12);
    CHECK_EQ(std::get<CellError>(parse_cell("=1/0")).code, "#DIV/0!");
    CHECK_EQ(std::get<CellError>(parse_cell("=SUM(A1)")).code, "#NAME?");
}

TEST_CASE("asking for the wrong alternative is caught at run time") {
    const Cell cell = parse_cell("hello");
    CHECK_THROWS_AS(std::get<double>(cell), std::bad_variant_access);
    CHECK(std::get_if<double>(&cell) == nullptr);
}

TEST_CASE("display handles every alternative through std::visit") {
    CHECK_EQ(display(parse_cell("")), "");
    CHECK_EQ(display(parse_cell("3")), "3");
    CHECK_EQ(display(parse_cell("FALSE")), "FALSE");
    CHECK_EQ(display(parse_cell("#REF!")), "#REF!");
}

TEST_CASE("sum_numbers skips text and propagates errors") {
    CHECK(std::get<double>(sum_numbers({parse_cell("10"), parse_cell("Rent"), parse_cell(""), parse_cell("=5/2")})) ==
          12.5);
    CHECK(std::holds_alternative<CellError>(sum_numbers({parse_cell("1"), parse_cell("=1/0")})));
    CHECK(std::get<double>(sum_numbers({})) == 0.0);
}

TEST_CASE("std::any holds anything but is checked when read") {
    PluginInfo info;
    info.set("name", std::string("csv-export"));
    info.set("version", 3);
    CHECK_EQ(info.get<std::string>("name"), "csv-export");
    CHECK_EQ(info.get<int>("version"), 3);
    CHECK_THROWS_AS(info.get<long>("version"), std::bad_any_cast);
    CHECK(info.try_get<double>("version") == nullptr);
    CHECK(info.try_get<int>("missing") == nullptr);
    CHECK_THROWS_AS(info.get<int>("missing"), std::out_of_range);
}

TEST_CASE("run classifies each cell and sums the column") {
    std::istringstream in("10\nRent\n=9/3\nTRUE\n\nEND\n");
    std::ostringstream out;
    CHECK_EQ(run(in, out), 0);
    const auto text = out.str();
    CHECK(text.find("track + 1000 ft = 704.8 m") != std::string::npos);
    CHECK(text.find("number: '10'") != std::string::npos);
    CHECK(text.find("text: 'Rent'") != std::string::npos);
    CHECK(text.find("boolean: 'TRUE'") != std::string::npos);
    CHECK(text.find("empty: ''") != std::string::npos);
    CHECK(text.find("SUM = 13") != std::string::npos);
}
