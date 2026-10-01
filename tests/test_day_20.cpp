// Tests for Day 20 – Returning Functions.
#include <sstream>
#include <stdexcept>
#include <vector>

#include "cppm/testing.hpp"
#include "day_20_returning_functions/lesson.hpp"

using namespace cppm::day20;

TEST_CASE("rate_function_for returns callable pointers or nullptr") {
    const RateFunction post = rate_function_for("post");
    REQUIRE(post != nullptr);
    CHECK_EQ(post(1'000), 750);
    CHECK(rate_function_for("courier") == &courier_rate);
    CHECK(rate_function_for("pigeon") == nullptr);
}

TEST_CASE("the tariffs grow with weight") {
    CHECK_EQ(courier_rate(1'000), 1'200);
    CHECK_EQ(freight_rate(4'999), 4'000);
    CHECK_EQ(freight_rate(8'000), 4'450);
}

TEST_CASE("returned lambdas remember their captured values") {
    const PriceRule ten_off = make_discount(10);
    const PriceRule half_off = make_discount(50);
    CHECK_EQ(ten_off(1'000), 900);
    CHECK_EQ(half_off(1'000), 500);
    CHECK_EQ(make_surcharge(300)(1'000), 1'300);
    CHECK_THROWS_AS(make_discount(120), std::invalid_argument);
}

TEST_CASE("compose applies the first rule, then the second") {
    const PriceRule discount_then_fee = compose(make_discount(10), make_surcharge(300));
    const PriceRule fee_then_discount = compose(make_surcharge(300), make_discount(10));
    CHECK_EQ(discount_then_fee(1'000), 1'200);
    CHECK_EQ(fee_then_discount(1'000), 1'170);
}

TEST_CASE("quote returns a struct and validates its input") {
    const Quote q = quote("courier", 1'000, make_discount(50));
    CHECK_EQ(q.carrier, "courier");
    CHECK_EQ(q.cents, 600);
    CHECK_EQ(q.days, 1);
    CHECK_THROWS_AS(quote("pigeon", 100), std::invalid_argument);
    CHECK_THROWS_AS(quote("post", 0), std::invalid_argument);
}

TEST_CASE("cheapest_quote picks the lowest price or returns nothing") {
    CHECK_EQ(cheapest_quote(1'000, {"post", "courier"})->carrier, "post");
    CHECK_EQ(cheapest_quote(30'000, {"post", "courier", "freight"})->carrier, "freight");
    CHECK(!cheapest_quote(1'000, {"pigeon"}).has_value());
}

TEST_CASE("quote_all hands every result to the callback") {
    std::vector<long long> seen;
    const int count = quote_all({100, 200, 300}, "post", [&](const Quote& q) { seen.push_back(q.cents); });
    CHECK_EQ(count, 3);
    CHECK(seen == std::vector<long long>{480, 510, 540});
}

TEST_CASE("run combines promotions and prints callback results") {
    std::istringstream in("1000 10 300\n1000 150\nheavy\n\n");
    std::ostringstream out;
    CHECK_EQ(run(in, out), 0);
    const auto text = out.str();
    CHECK(text.find("cheapest: post 975 cents, 3 day(s)") != std::string::npos);
    CHECK(text.find("discount must be 0-100 %") != std::string::npos);
    CHECK(text.find("callback: post 600 cents") != std::string::npos);
    CHECK(text.find("Batch total: 1500 cents") != std::string::npos);
}
