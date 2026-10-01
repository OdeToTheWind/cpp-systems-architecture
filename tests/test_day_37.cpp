// Tests for Day 37 – Graphics Programming.
#include <sstream>
#include <stdexcept>
#include <string>

#include "cppm/testing.hpp"
#include "day_37_graphics/lesson.hpp"

using namespace cppm::day37;

TEST_CASE("a canvas starts in the background colour and clips writes") {
    Canvas canvas(4, 3, black);
    CHECK_EQ(canvas.count(black), 12);
    canvas.set(-1, 0, red);
    canvas.set(4, 2, red);
    CHECK_EQ(canvas.count(red), 0);
    CHECK_THROWS_AS(canvas.get(4, 0), std::out_of_range);
    CHECK_THROWS_AS(Canvas(0, 10), std::invalid_argument);
}

TEST_CASE("Bresenham lines include both end points and have no gaps") {
    Canvas canvas(10, 10);
    canvas.line(1, 1, 8, 4, black);
    CHECK(canvas.get(1, 1) == black);
    CHECK(canvas.get(8, 4) == black);
    CHECK_EQ(canvas.count(black), 8);  // one pixel per column along the major axis
    Canvas steep(10, 10);
    steep.line(2, 9, 3, 0, black);  // works in every direction
    CHECK_EQ(steep.count(black), 10);
}

TEST_CASE("horizontal, vertical and single-point lines") {
    Canvas canvas(5, 5);
    canvas.line(0, 2, 4, 2, black);
    canvas.line(2, 0, 2, 4, black);
    CHECK_EQ(canvas.count(black), 9);
    Canvas dot(3, 3);
    dot.line(1, 1, 1, 1, red);
    CHECK_EQ(dot.count(red), 1);
}

TEST_CASE("the midpoint circle is symmetric") {
    Canvas canvas(21, 21);
    canvas.circle(10, 10, 6, red);
    CHECK(canvas.get(16, 10) == red);
    CHECK(canvas.get(4, 10) == red);
    CHECK(canvas.get(10, 16) == red);
    CHECK(canvas.get(10, 4) == red);
    CHECK(canvas.get(10, 10) == white);
    CHECK_THROWS_AS(canvas.circle(1, 1, -1, red), std::invalid_argument);
}

TEST_CASE("flood fill stays inside the closed shape") {
    Canvas canvas(21, 21);
    canvas.circle(10, 10, 6, red);
    const int inside = canvas.flood_fill(10, 10, gold);
    CHECK(inside > 80 && inside < 120);
    CHECK(canvas.get(0, 0) == white);
    CHECK_EQ(canvas.flood_fill(10, 10, gold), 0);  // already that colour
    CHECK_EQ(canvas.flood_fill(-5, 0, gold), 0);
}

TEST_CASE("to_ppm writes a valid P3 header and one triple per pixel") {
    Canvas canvas(2, 1);
    canvas.set(1, 0, red);
    CHECK_EQ(to_ppm(canvas), "P3\n2 1\n255\n255 255 255 220 30 40\n");
}

TEST_CASE("the same canvas can be displayed as ASCII") {
    Canvas canvas(3, 2);
    canvas.set(0, 0, black);
    canvas.set(2, 1, gold);
    CHECK_EQ(to_ascii(canvas), "#  \n  +\n");
}

TEST_CASE("run draws the poster at the chosen size") {
    std::istringstream in("12\n");
    std::ostringstream out;
    CHECK_EQ(run(in, out), 0);
    const auto text = out.str();
    CHECK(text.find("############") != std::string::npos);
    CHECK(text.find("PPM starts with: P3\n12 12") != std::string::npos);
    std::istringstream bad("500\n");
    std::ostringstream out2;
    run(bad, out2);
    CHECK(out2.str().find("using 20 instead") != std::string::npos);
}
