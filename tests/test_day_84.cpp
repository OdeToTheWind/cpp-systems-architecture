// Tests for Day 84 – Capstone: Data Pipeline (ETL).
#include <sstream>
#include <stdexcept>
#include <string>
#include <variant>
#include <vector>

#include "cppm/testing.hpp"
#include "day_84_etl/lesson.hpp"

using namespace cppm::day84;

namespace {
const std::string export_a =
    "order_id,date,email,amount,currency\n"
    "A1,2026-06-01,Ana@Example.com,\"1,234.50\",EUR\n"
    "A2,02/06/2026,ben@example.com,10,GBP\n"
    "A3,2026-02-30,cleo@example.com,5,EUR\n"
    "A4,2026-06-03,no-at-sign,5,EUR\n"
    "A5,2026-06-03,dev@example.com,5.999,EUR\n"
    "A6,2026-06-03,eve@example.com,5,JPY\n"
    "A7,2026-06-04,\"fay, jr\"@example.com\n"
    ",2026-06-04,gil@example.com,1,EUR\n";
}  // namespace

TEST_CASE("CSV lines honour quotes and doubled quotes") {
    CHECK(parse_csv_line(R"(a,"b, c","say ""hi""",)") == std::vector<std::string>{"a", "b, c", "say \"hi\"", ""});
    CHECK_THROWS_AS(parse_csv_line("\"open"), std::runtime_error);
}

TEST_CASE("extract maps fields by header and rejects malformed lines") {
    std::istringstream in(" Email,ORDER_ID\nx@y.z,7\n\nbroken\n");
    const auto rows = extract(in);
    CHECK_EQ(rows.size(), 2u);
    CHECK_EQ(std::get<RawRow>(rows[0]).field.at("order_id"), "7");
    CHECK_EQ(std::get<Rejection>(rows[1]).line, 4);
    CHECK_EQ(std::get<Rejection>(rows[1]).reason, "expected 2 fields, got 1");
}

TEST_CASE("dates and amounts are validated strictly") {
    CHECK(valid_date("2024-02-29"));
    CHECK(!valid_date("2026-02-29"));
    CHECK(!valid_date("2026-13-01"));
    CHECK(!valid_date("2026-6-01"));
    CHECK_EQ(parse_cents("1,234.5"), 123450);
    CHECK_EQ(parse_cents("7"), 700);
    CHECK_THROWS_AS(parse_cents("-3"), std::invalid_argument);
    CHECK_THROWS_AS(parse_cents("1.234"), std::invalid_argument);
}

TEST_CASE("transform normalises dates, emails and currencies") {
    const RawRow row{2, {{"order_id", "9"}, {"date", "31/12/2026"}, {"email", "X@Y.COM"}, {"amount", "10.00"}, {"currency", "USD"}}};
    const auto order = std::get<Order>(transform(row, demo_rates(), "mkt"));
    CHECK_EQ(order.id, "mkt:9");
    CHECK_EQ(order.date, "2026-12-31");
    CHECK_EQ(order.email, "x@y.com");
    CHECK_EQ(order.eur_cents, 920);
    const RawRow missing{3, {{"order_id", "1"}, {"date", "2026-01-01"}, {"email", "a@b.c"}, {"amount", "1"}, {"currency", "XYZ"}}};
    CHECK_EQ(std::get<Rejection>(transform(missing, demo_rates(), "mkt")).reason, "unknown currency XYZ");
}

TEST_CASE("bad rows go to quarantine and good rows are loaded") {
    Warehouse warehouse;
    std::istringstream in(export_a);
    const auto summary = run_pipeline(in, "alpha", warehouse, demo_rates());
    CHECK_EQ(summary.read, 8);
    CHECK_EQ(summary.inserted, 2);
    CHECK_EQ(summary.quarantine.size(), 6u);
    CHECK_EQ(warehouse.find("alpha:A2")->eur_cents, 1170);
    CHECK_EQ(warehouse.revenue_cents(), 123450 + 1170);
    const auto text = summary.report();
    CHECK(text.find("line 4: invalid date '2026-02-30'") != std::string::npos);
    CHECK(text.find("line 6: bad amount '5.999'") != std::string::npos);
    CHECK(text.find("line 8: expected 5 fields, got 3") != std::string::npos);
    CHECK(text.find("line 9: missing order_id") != std::string::npos);
}

TEST_CASE("re-running an import is idempotent and corrections update") {
    Warehouse warehouse;
    std::istringstream first(export_a);
    run_pipeline(first, "alpha", warehouse, demo_rates());
    std::istringstream again(export_a);
    const auto rerun = run_pipeline(again, "alpha", warehouse, demo_rates());
    CHECK_EQ(rerun.inserted, 0);
    CHECK_EQ(rerun.unchanged, 2);
    CHECK_EQ(warehouse.size(), 2u);
    std::istringstream fixed("order_id,date,email,amount,currency\nA1,2026-06-01,ana@example.com,1200,EUR\n");
    CHECK_EQ(run_pipeline(fixed, "alpha", warehouse, demo_rates()).updated, 1);
    std::istringstream other("order_id,date,email,amount\nA1,2026-06-01,zed@example.com,1\n");
    CHECK_EQ(run_pipeline(other, "beta", warehouse, demo_rates()).inserted, 1);  // ids are per source
}

TEST_CASE("run imports pasted batches") {
    std::istringstream in("order_id,date,email,amount\n1,2026-06-01,a@b.c,2.50\n2,bad,a@b.c,1\n\n\n");
    std::ostringstream out;
    CHECK_EQ(run(in, out), 0);
    CHECK(out.str().find("shop: read 2, inserted 1, updated 0, unchanged 0, quarantined 1") != std::string::npos);
    CHECK(out.str().find("warehouse: 1 order(s), EUR 2.50") != std::string::npos);
}
