// Tests for Day 32 – Constructors and Initialiser Lists.
#include <sstream>
#include <stdexcept>
#include <type_traits>

#include "cppm/testing.hpp"
#include "day_32_constructors/lesson.hpp"

using namespace cppm::day32;

TEST_CASE("the parameterised constructor computes the total") {
    const Reservation r("Ada", CarClass::estate, 10, 12);
    CHECK_EQ(r.days(), 3);
    CHECK_EQ(r.total_cents(), 16'500);
    CHECK_EQ(r.customer(), "Ada");
}

TEST_CASE("invalid reservations can never be constructed") {
    CHECK_THROWS_AS(Reservation("", CarClass::van, 1, 2), std::invalid_argument);
    CHECK_THROWS_AS(Reservation("Bo", CarClass::van, 5, 4), std::invalid_argument);
    CHECK_THROWS_AS(Reservation("Bo", CarClass::van, 0, 4), std::invalid_argument);
    CHECK_THROWS_AS(Reservation("Bo", CarClass::van, 10, 0u), std::invalid_argument);
}

TEST_CASE("the delegating constructor reuses the main one") {
    const int main_before = ConstructionLog::parameterised;
    const int delegated_before = ConstructionLog::delegated;
    const Reservation r("Cy", CarClass::compact, 100, 4u);
    CHECK_EQ(r.days(), 4);
    CHECK_EQ(r.total_cents(), 15'600);
    CHECK_EQ(ConstructionLog::parameterised, main_before + 1);
    CHECK_EQ(ConstructionLog::delegated, delegated_before + 1);
}

TEST_CASE("each reservation gets a fresh confirmation number, copies included") {
    const Reservation a("Di", CarClass::van, 1, 1);
    const Reservation b("Ed", CarClass::van, 1, 1);
    CHECK_EQ(b.confirmation(), a.confirmation() + 1);
    const int copies_before = ConstructionLog::copied;
    const Reservation c = a.copy_for_rebooking();
    CHECK(c.confirmation() > b.confirmation());
    CHECK_EQ(c.customer(), a.customer());
    CHECK_EQ(c.total_cents(), a.total_cents());
    CHECK(ConstructionLog::copied >= copies_before + 1);
}

TEST_CASE("const members make the class copy-constructible but not assignable") {
    static_assert(std::is_copy_constructible_v<Reservation>);
    static_assert(!std::is_copy_assignable_v<Reservation>);
    static_assert(!std::is_default_constructible_v<Reservation>);
    static_assert(std::is_default_constructible_v<WalkInForm>);
    CHECK(Reservation("Fi", CarClass::compact, 1, 1).confirmation() >= 1000);
}

TEST_CASE("a default-constructed form has sensible defaults and validates on submit") {
    WalkInForm form;
    CHECK(form.car == CarClass::compact);
    CHECK_EQ(form.days, 1);
    CHECK_THROWS_AS(form.submit(), std::invalid_argument);  // no customer yet
    form.customer = "Gus";
    form.days = 2;
    CHECK_EQ(form.submit().total_cents(), 7'800);
}

TEST_CASE("run books from typed forms") {
    std::istringstream in("Ada estate 120 3\nBo van 400 1\nnonsense\n\n");
    std::ostringstream out;
    CHECK_EQ(run(in, out), 0);
    const auto text = out.str();
    CHECK(text.find("Ada, 3 day(s), 16500 cents (next year: #") != std::string::npos);
    CHECK(text.find("rental days must be") != std::string::npos);
    CHECK(text.find("e.g. Ada estate 120 3") != std::string::npos);
}
