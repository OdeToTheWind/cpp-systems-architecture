// Tests for Day 39 – Inheritance.
#include <memory>
#include <sstream>
#include <stdexcept>
#include <type_traits>

#include "cppm/testing.hpp"
#include "day_39_inheritance/lesson.hpp"

using namespace cppm::day39;

TEST_CASE("the hierarchy has the intended shape") {
    static_assert(std::is_base_of_v<Charger, AcCharger>);
    static_assert(std::is_base_of_v<AcCharger, SolarCanopyCharger>);
    static_assert(std::is_base_of_v<Reportable, SolarCanopyCharger>);
    static_assert(std::is_final_v<DcFastCharger>);
    static_assert(std::has_virtual_destructor_v<Charger>);
    CHECK_THROWS_AS(Charger("X", 0), std::invalid_argument);
}

TEST_CASE("virtual functions dispatch on the real type behind a base reference") {
    const AcCharger ac("A");
    const DcFastCharger dc("D");
    const Charger& base_ac = ac;
    const Charger& base_dc = dc;
    CHECK_EQ(base_ac.kind(), "AC 11 kW");
    CHECK_EQ(base_dc.kind(), "DC 150 kW");
    CHECK_EQ(Charger("G", 7).kind(), "generic charger");
}

TEST_CASE("the template method combines the overridden hooks") {
    const AcCharger ac("A");
    CHECK_EQ(ac.session_cost_cents(10, 60), 350);
    CHECK_EQ(ac.session_cost_cents(50, 60), 385);  // an 11 kW post cannot deliver 50 kWh in an hour
    const DcFastCharger dc("D");
    CHECK_EQ(dc.session_cost_cents(40, 30), 2'360);
    CHECK_EQ(dc.session_cost_cents(40, 60), 2'360 + 600);  // 15 idle minutes
    CHECK_THROWS_AS(dc.session_cost_cents(-1, 10), std::invalid_argument);
}

TEST_CASE("the solar charger blends prices and implements both interfaces") {
    const SolarCanopyCharger solar("S", 0.5, 3);
    CHECK_EQ(solar.session_cost_cents(10, 60), 225);  // (35 * 0.5 + 10 * 0.5) * 10
    CHECK_EQ(solar.status_report(), "S: 50% solar today");
    CHECK_EQ(solar.days_until_service(), 3);
}

TEST_CASE("dynamic_cast discovers optional capabilities") {
    const auto site = sample_site();
    CHECK(!maintenance_due(*site[0], 7));
    CHECK(!maintenance_due(*site[1], 7));
    CHECK(maintenance_due(*site[2], 7));
    CHECK(!maintenance_due(*site[2], 2));
}

TEST_CASE("deleting through a base pointer runs every destructor") {
    const int base_before = DestructionLog::base;
    const int derived_before = DestructionLog::derived;
    { std::unique_ptr<Charger> charger = std::make_unique<DcFastCharger>("D"); }
    CHECK_EQ(DestructionLog::base, base_before + 1);
    CHECK_EQ(DestructionLog::derived, derived_before + 1);
}

TEST_CASE("run prices sessions on every charger and lists capabilities") {
    std::istringstream in("10 60\nstop\n");
    std::ostringstream out;
    CHECK_EQ(run(in, out), 0);
    const auto text = out.str();
    CHECK(text.find("AC-1 (AC 11 kW): 350 cents") != std::string::npos);
    CHECK(text.find("DC-1 (DC 150 kW): 1190 cents") != std::string::npos);
    CHECK(text.find("SOL-1: 50% solar today") != std::string::npos);
    CHECK(text.find("SOL-1 needs service this week") != std::string::npos);
}
