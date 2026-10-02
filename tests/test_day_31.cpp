// Tests for Day 31 – Member Functions.
#include <sstream>
#include <stdexcept>
#include <type_traits>

#include "cppm/testing.hpp"
#include "day_31_member_functions/lesson.hpp"

using namespace cppm::day31;

TEST_CASE("static member functions are called on the class, not an object") {
    CHECK(RoastBatch::is_known_bean("kenya"));
    CHECK(!RoastBatch::is_known_bean("mars"));
}

TEST_CASE("static data numbers every batch in creation order") {
    const int before = RoastBatch::batches_created();
    const RoastBatch first("brazil");
    const RoastBatch second("kenya");
    CHECK_EQ(second.number(), first.number() + 1);
    CHECK_EQ(RoastBatch::batches_created(), before + 2);
}

TEST_CASE("mutators chain because they return *this") {
    RoastBatch batch("colombia");
    RoastBatch& result = batch.add_beans(1'000).add_beans(500).set_roast(Roast::dark).record_output(1'260);
    CHECK(&result == &batch);
    CHECK_EQ(batch.green_grams(), 1'500);
    CHECK(batch.roast() == Roast::dark);
    CHECK_NEAR(batch.weight_loss_percent(), 16.0, 1e-9);
}

TEST_CASE("const member functions work on const objects") {
    const RoastBatch batch = RoastBatch("ethiopia").add_beans(200);
    CHECK_EQ(batch.origin(), "ethiopia");
    CHECK_NEAR(batch.weight_loss_percent(), 0.0, 1e-12);
    static_assert(std::is_same_v<decltype(batch.green_grams()), int>);
}

TEST_CASE("invalid batches are rejected") {
    CHECK_THROWS_AS(RoastBatch("mars"), std::invalid_argument);
    RoastBatch batch("brazil");
    CHECK_THROWS_AS(batch.add_beans(0), std::invalid_argument);
    batch.add_beans(100);
    CHECK_THROWS_AS(batch.record_output(150), std::invalid_argument);
}

TEST_CASE("parse is a static factory that may return nothing") {
    const auto batch = RoastBatch::parse("kenya 800 light");
    REQUIRE(batch.has_value());
    CHECK_EQ(batch->green_grams(), 800);
    CHECK(batch->roast() == Roast::light);
    CHECK(!RoastBatch::parse("mars 800 light").has_value());
    CHECK(!RoastBatch::parse("kenya -5 dark").has_value());
}

TEST_CASE("same_origin_as compares this with another object") {
    const RoastBatch a("brazil");
    const RoastBatch b("brazil");
    const RoastBatch c("kenya");
    CHECK(a.same_origin_as(b));
    CHECK(!a.same_origin_as(c));
    CHECK(!a.same_origin_as(a));
}

TEST_CASE("run tracks batches and spots shared origins") {
    std::istringstream in(
        "brazil 1000 dark 800\nkenya 500 light\nbrazil 400 medium\nmars 1 dark\nkenya 10 light 99\n\n");
    std::ostringstream out;
    CHECK_EQ(run(in, out), 0);
    const auto text = out.str();
    CHECK(text.find("brazil, loss 20%") != std::string::npos);
    CHECK(text.find("same origin as batch #") != std::string::npos);
    CHECK(text.find("could not parse that batch") != std::string::npos);
    CHECK(text.find("roasted weight must be between") != std::string::npos);
    CHECK(text.find("batch number(s) issued so far") != std::string::npos);
}
