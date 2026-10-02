// Tests for Day 88 – Capstone: Report Generator.
#include <sstream>
#include <stdexcept>
#include <string>

#include "cppm/testing.hpp"
#include "day_88_reports/lesson.hpp"

using namespace cppm::day88;

TEST_CASE("money formats with separators and keeps exact cents") {
    CHECK_EQ(Money(123456789).str(), "€1,234,567.89");
    CHECK_EQ(Money(-5).str(), "-€0.05");
    CHECK_EQ(Money(100000).plain(), "1000.00");
    Money total;
    for (int i = 0; i < 10; ++i) total += Money(10);  // 10 x 0.10 is exactly 1.00
    CHECK(total == Money(100));
}

TEST_CASE("scaling rounds half away from zero") {
    CHECK_EQ(Money(250).scaled(1, 100).cents(), 3);  // 2.5 -> 3
    CHECK_EQ(Money(-250).scaled(1, 100).cents(), -3);
    CHECK_EQ(Money(9000).scaled(50, 60).cents(), 7500);
    CHECK_EQ(Money(1000).scaled(23, 100).cents(), 230);
    CHECK_THROWS_AS(Money(1).scaled(1, 0), std::invalid_argument);
}

TEST_CASE("HTML escaping neutralises markup") {
    CHECK_EQ(html_escape(R"(<script>alert("x")</script> & 'y')"),
             "&lt;script&gt;alert(&quot;x&quot;)&lt;/script&gt; &amp; &#39;y&#39;");
}

TEST_CASE("templates escape by default, loop over rows and reject unknown keys") {
    Context ctx;
    ctx.values = {{"title", "<Q2>"}, {"badge", "<b>new</b>"}};
    ctx.lists["items"] = {{{"name", "a&b"}}, {{"name", "c"}}};
    CHECK_EQ(render_template("{{title}} {{&badge}}: {{#items}}[{{name}}|{{title}}]{{/items}}!", ctx),
             "&lt;Q2&gt; <b>new</b>: [a&amp;b|&lt;Q2&gt;][c|&lt;Q2&gt;]!");
    CHECK_THROWS_AS(render_template("{{missing}}", ctx), std::runtime_error);
    CHECK_THROWS_AS(render_template("{{#items}}open", ctx), std::runtime_error);
    CHECK_THROWS_AS(render_template("{{title", ctx), std::runtime_error);
}

TEST_CASE("CSV fields are quoted and defused") {
    CHECK_EQ(csv_field("plain"), "plain");
    CHECK_EQ(csv_field("Smith, J"), "\"Smith, J\"");
    CHECK_EQ(csv_field("say \"hi\""), "\"say \"\"hi\"\"\"");
    CHECK_EQ(csv_field("=HYPERLINK(\"x\")"), "\"'=HYPERLINK(\"\"x\"\")\"");
    CHECK_EQ(csv_field("-5"), "'-5");
}

TEST_CASE("the report adds up exactly as printed") {
    const auto report = build_report("2026-06",
                                     {{"Acme", "Logo", 90, Money(8000)},
                                      {"Acme", "Fonts", 20, Money(8000)},
                                      {"Bolt <Ltd>", "Site", 50, Money(9000)}},
                                     23);
    CHECK_EQ(report.lines[1].net.cents(), 2667);  // 80 * 20/60 = 26.666… -> 26.67
    CHECK_EQ(report.net_by_client.at("Acme").cents(), 12000 + 2667);
    CHECK_EQ(report.net_total.cents(), 12000 + 2667 + 7500);
    CHECK_EQ(report.vat_total.cents(), 2760 + 613 + 1725);
    CHECK_EQ(report.gross_total().str(), "€272.65");
    const auto html = to_html(report);
    CHECK(html.find("<td>Bolt &lt;Ltd&gt;</td><td>Site</td><td>0:50</td><td>€75.00</td>") != std::string::npos);
    CHECK(to_csv(report).find("Acme,Fonts,20,26.67,6.13\r\n") != std::string::npos);
    CHECK_THROWS_AS(build_report("x", {{"A", "B", 0, Money(1)}}, 23), std::invalid_argument);
}

TEST_CASE("run prints HTML and CSV for the entered hours") {
    std::istringstream in("Acme 60 10000 Branding, round 2\n\n");
    std::ostringstream out;
    CHECK_EQ(run(in, out), 0);
    CHECK(out.str().find("<h1>Billing 2026-06</h1>") != std::string::npos);
    CHECK(out.str().find("= <strong>€123.00</strong>") != std::string::npos);
    CHECK(out.str().find("Acme,\"Branding, round 2\",60,100.00,23.00") != std::string::npos);
}
