// Tests for Day 22 – Documentation vs Comments.
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

#include "cppm/testing.hpp"
#include "day_22_documentation/lesson.hpp"

using namespace cppm::day22;

TEST_CASE("convert honours its documented contract for mass and volume") {
    CHECK_NEAR(convert(1, Unit::kilogram, Unit::gram), 1000.0, 1e-9);
    CHECK_NEAR(convert(16, Unit::ounce, Unit::pound), 1.0, 1e-9);
    CHECK_NEAR(convert(3, Unit::teaspoon, Unit::tablespoon), 1.0, 1e-3);
    CHECK_NEAR(convert(2, Unit::cup, Unit::millilitre), 473.176, 1e-9);
}

TEST_CASE("convert throws exactly what its @throws lines promise") {
    CHECK_THROWS_AS(convert(-1, Unit::gram, Unit::kilogram), std::invalid_argument);
    CHECK_THROWS_AS(convert(1, Unit::gram, Unit::cup), std::domain_error);
}

TEST_CASE("cups use the US customary size the why-comment explains") {
    CHECK_NEAR(cups_to_millilitres(1), 236.588, 1e-9);
    CHECK(cups_to_millilitres(1) < 250.0);
}

TEST_CASE("scale_recipe checks its precondition and leaves the input alone") {
    const std::vector<double> recipe{200, 3, 0.5};
    const auto doubled = scale_recipe(recipe, 2.0);
    CHECK(doubled == std::vector<double>{400, 6, 1});
    CHECK(recipe == std::vector<double>{200, 3, 0.5});
    CHECK_THROWS_AS(scale_recipe(recipe, 0), std::invalid_argument);
}

TEST_CASE("find_undocumented lists functions without a doc comment") {
    const std::string header =
        "/// Documented with a triple slash.\n"
        "int documented_one(int a);\n"
        "\n"
        "/**\n"
        " * @brief Block style.\n"
        " */\n"
        "double documented_two(double x);\n"
        "// a plain comment is not documentation\n"
        "void forgotten(int x);\n"
        "inline bool also_forgotten() { return true; }\n";
    CHECK(find_undocumented(header) == std::vector<std::string>{"forgotten", "also_forgotten"});
}

TEST_CASE("this lesson's own public functions are all documented") {
    const std::string own =
        "/// Parse a unit name.\nstd::optional<Unit> parse_unit(std::string_view name);\n"
        "/// The demo.\nint run(std::istream& in, std::ostream& out);\n";
    CHECK(find_undocumented(own).empty());
}

TEST_CASE("is_what_comment spots comments that repeat the code") {
    CHECK(is_what_comment("i++;", "increment i"));
    CHECK(is_what_comment("total = price * quantity;", "total is price times quantity") == false);
    CHECK(is_what_comment("total = price * quantity;", "total = price * quantity"));
    CHECK(!is_what_comment("retries = 3;", "the API rate-limits after three fast calls"));
}

TEST_CASE("parse_unit and run convert typed requests") {
    CHECK(parse_unit("tbsp") == Unit::tablespoon);
    CHECK(!parse_unit("pinch").has_value());
    std::istringstream in("1 kg lb\n1 g cup\n1 pinch g\nx\n\n");
    std::ostringstream out;
    CHECK_EQ(run(in, out), 0);
    const auto text = out.str();
    CHECK(text.find("1 kg = 2.20462 lb") != std::string::npos);
    CHECK(text.find("without a density") != std::string::npos);
    CHECK(text.find("units: g kg") != std::string::npos);
    CHECK(text.find("e.g. 8 oz g") != std::string::npos);
}
