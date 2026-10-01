// Tests for Day 21 – Return vs Print.
#include <sstream>
#include <stdexcept>
#include <string>

#include "cppm/testing.hpp"
#include "day_21_return_vs_print/lesson.hpp"

using namespace cppm::day21;

TEST_CASE("energy_charge_cents applies each tier") {
    CHECK_EQ(energy_charge_cents(0), 0);
    CHECK_EQ(energy_charge_cents(100), 2'000);
    CHECK_EQ(energy_charge_cents(150), 3'250);
    CHECK_EQ(energy_charge_cents(300), 7'000);
    CHECK_EQ(energy_charge_cents(400), 10'200);
    CHECK_THROWS_AS(energy_charge_cents(-1), std::invalid_argument);
}

TEST_CASE("a pure function gives the same answer every time") {
    const Bill a = compute_bill("3B", 1'200, 1'450);
    const Bill b = compute_bill("3B", 1'200, 1'450);
    CHECK_EQ(a.total_cents(), b.total_cents());
    CHECK_EQ(a.kwh, 250);
    CHECK_EQ(a.energy_cents, 5'750);
    CHECK_EQ(a.vat_cents, 332);
    CHECK_EQ(a.total_cents(), 6'982);
}

TEST_CASE("format_bill presents a returned bill without recalculating") {
    const auto text = format_bill(compute_bill("1A", 0, 100));
    CHECK(text.find("Flat 1A: 100 kWh") != std::string::npos);
    CHECK(text.find("energy      20.00") != std::string::npos);
    CHECK(text.find("total       30.45") != std::string::npos);
}

TEST_CASE("building_total reuses the returned numbers") {
    const std::vector<Bill> bills{compute_bill("1A", 0, 100), compute_bill("3B", 1'200, 1'450)};
    CHECK_EQ(building_total(bills), 3'045 + 6'982);
    CHECK_EQ(building_total({}), 0);
}

TEST_CASE("the print-only version can only be tested by capturing and parsing text") {
    std::ostringstream out;
    print_bill_badly(out, "3B", 1'200, 1'450);
    CHECK_EQ(out.str(), "Flat 3B: 250 kWh, total 69.82\n");
    CHECK_EQ(total_from_printed_bill(out.str()), compute_bill("3B", 1'200, 1'450).total_cents());
}

TEST_CASE("parsing printed output breaks as soon as the wording changes") {
    CHECK_THROWS_AS(total_from_printed_bill("Flat 3B: 250 kWh, amount due 69.82"), std::runtime_error);
    CHECK_THROWS_AS(total_from_printed_bill("total: n/a"), std::runtime_error);
}

TEST_CASE("run bills each flat and totals the building") {
    std::istringstream in("1A 0 100\n3B 1200 1450\n2C 50 10\nbad\n\n");
    std::ostringstream out;
    CHECK_EQ(run(in, out), 0);
    const auto text = out.str();
    CHECK(text.find("meter readings cannot go backwards") != std::string::npos);
    CHECK(text.find("e.g. 3B 1200 1450") != std::string::npos);
    CHECK(text.find("Building total: 100.27 (2 flats)") != std::string::npos);
}
