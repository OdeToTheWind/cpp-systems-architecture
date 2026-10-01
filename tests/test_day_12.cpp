// Tests for Day 12 – Functions.
#include <sstream>
#include <stdexcept>

#include "cppm/testing.hpp"
#include "day_12_functions/lesson.hpp"

using namespace cppm::day12;

TEST_CASE("the default argument picks the medium size") {
    CHECK_EQ(unit_price("croissant"), 250);
    CHECK_EQ(unit_price("croissant", Size::medium), unit_price("croissant"));
    CHECK_EQ(unit_price("croissant", Size::small), 200);
    CHECK_EQ(unit_price("croissant", Size::large), 325);
    CHECK_THROWS_AS(unit_price("pizza"), std::invalid_argument);
}

TEST_CASE("add_item modifies the caller's order through a reference") {
    Order order;
    CHECK_EQ(add_item(order, "muffin", 2), 580);
    CHECK_EQ(add_item(order, "baguette", 1, Size::large), 416);
    REQUIRE_EQ(order.lines.size(), 2u);
    CHECK_EQ(order.lines[1].item, "baguette");
    CHECK_THROWS_AS(add_item(order, "muffin", 0), std::invalid_argument);
    CHECK_EQ(order.lines.size(), 2u);
}

TEST_CASE("order_total reads a const order") {
    Order order;
    const Order& view = order;
    CHECK_EQ(order_total(view), 0);
    add_item(order, "coffee", 3);
    CHECK_EQ(order_total(view), 900);
}

TEST_CASE("apply_loyalty works on a copy and compounds per 10 stamps") {
    const long long total = 10'000;
    CHECK_EQ(apply_loyalty(total, 9), 10'000);
    CHECK_EQ(apply_loyalty(total, 10), 9'000);
    CHECK_EQ(apply_loyalty(total, 25), 8'100);
    CHECK_EQ(total, 10'000);
    CHECK_THROWS_AS(apply_loyalty(total, -1), std::invalid_argument);
}

TEST_CASE("format_price overloads share one formatting rule") {
    CHECK_EQ(format_price(1'205), "12.05");
    CHECK_EQ(format_price(99, "GBP"), "0.99 GBP");
    Order order;
    add_item(order, "croissant", 2);
    CHECK_EQ(format_price(order), "5.00 EUR");
    CHECK_EQ(format_price(order, "USD"), "5.00 USD");
}

TEST_CASE("parse_size falls back to medium") {
    CHECK(parse_size("small") == Size::small);
    CHECK(parse_size("large") == Size::large);
    CHECK(parse_size("huge") == Size::medium);
}

TEST_CASE("run builds an order with multi-word items and loyalty stamps") {
    std::istringstream in("2 croissant large\n1 cinnamon roll\n3 pizza\nhello\n\n10\n");
    std::ostringstream out;
    CHECK_EQ(run(in, out), 0);
    const auto text = out.str();
    CHECK(text.find("+ 2 x croissant = 6.50") != std::string::npos);
    CHECK(text.find("+ 1 x cinnamon roll = 3.80") != std::string::npos);
    CHECK(text.find("we do not bake 'pizza'") != std::string::npos);
    CHECK(text.find("Subtotal: 10.30 EUR") != std::string::npos);
    CHECK(text.find("To pay: 9.27 EUR") != std::string::npos);
}
