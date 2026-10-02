// Tests for Day 71 – Functional Tools.
#include <functional>
#include <map>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

#include "cppm/testing.hpp"
#include "day_71_functional/lesson.hpp"

using namespace cppm::day71;

TEST_CASE("captured settings outlive the caller's variables") {
    Rule rule = [] {
        double percent = 20;
        auto r = percent_off(percent);
        percent = 90;  // too late: the rule captured 20 by value
        return r;
    }();
    CHECK_NEAR(rule(50), 40.0, 1e-9);
    CHECK_THROWS_AS(percent_off(120), std::invalid_argument);
}

TEST_CASE("by-reference captures see later changes") {
    double rate = 0.1;
    auto by_ref = [&rate](double p) { return p * (1 + rate); };
    auto by_value = [rate](double p) { return p * (1 + rate); };
    rate = 0.5;
    CHECK_NEAR(by_ref(100), 150.0, 1e-9);
    CHECK_NEAR(by_value(100), 110.0, 1e-9);
}

TEST_CASE("mutable init-captures keep private state per counter") {
    auto a = make_ticket_counter(1);
    auto b = make_ticket_counter(100);
    CHECK_EQ(a(), 1);
    CHECK_EQ(a(), 2);
    CHECK_EQ(b(), 100);
    auto copy = a;  // copies the state too
    CHECK_EQ(copy(), 3);
    CHECK_EQ(a(), 3);
}

TEST_CASE("compose applies functions left to right") {
    auto pipeline = compose(percent_off(10), [](double p) { return p + 2; }, round_to_cents());
    CHECK_NEAR(pipeline(19.99), 19.99, 1e-9);  // 17.991 + 2 = 19.991 -> 19.99
    auto twice = compose([](int x) { return x * 2; }, [](int x) { return x * 2; });
    CHECK_EQ(twice(3), 12);
    CHECK_NEAR(apply_rules(4, {percent_off(50), minimum_price(5)}), 5.0, 1e-9);
}

TEST_CASE("std::invoke handles member data, member functions and lambdas") {
    const std::vector<Item> cart{{"A", 2.5, 4}, {"B", 10, 1}};
    CHECK_NEAR(total_of(cart, &Item::price), 12.5, 1e-9);
    CHECK_NEAR(total_of(cart, &Item::line_total), 20.0, 1e-9);
    CHECK_NEAR(total_of(cart, [](const Item& i) { return i.quantity; }), 5.0, 1e-9);
}

TEST_CASE("bind_front fixes the leading arguments") {
    std::map<std::string, double> rates{{"PT", 0.23}};
    const Rule pt = regional_tax(rates, "PT");
    rates["PT"] = 0.5;  // the bound copy is unaffected
    CHECK_NEAR(pt(100), 123.0, 1e-9);
    CHECK_THROWS_AS(regional_tax(rates, "XX")(1), std::out_of_range);
}

TEST_CASE("memoisation evaluates each argument set once") {
    Memoised<double, std::string, int> rate(shipping_rate);
    CHECK_NEAR(rate("PT", 1200), 4.5, 1e-9);
    CHECK_NEAR(rate("PT", 1200), 4.5, 1e-9);
    CHECK_NEAR(rate("DE", 1200), 6.0, 1e-9);
    CHECK_EQ(rate.calls(), 2);
    std::istringstream in("100 spring PT 500\n100 spring PT 500\n10 vip XX 1\n3 spring DE 0\nstop\n");
    std::ostringstream out;
    CHECK_EQ(run(in, out), 0);
    CHECK(out.str().find("quote #1001: 108.05") != std::string::npos);  // 85 * 1.23 = 104.55 + 3.5
    CHECK(out.str().find("unknown campaign or region") != std::string::npos);
    CHECK(out.str().find("quote #1003: 10.45") != std::string::npos);  // floor 5 * 1.19 = 5.95 + 4.5
    CHECK(out.str().find("2 shipping lookup(s)") != std::string::npos);
}
