// Tests for Day 47 – Desktop GUI Architecture. The whole UI is tested with a fake view: no window.
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

#include "cppm/testing.hpp"
#include "day_47_gui_architecture/lesson.hpp"

using namespace cppm::day47;

namespace {
/// A headless view that records what the presenter told it to show.
class FakeView final : public ConverterView {
public:
    std::string amount = "100";
    std::string from = "EUR";
    std::string to = "USD";
    std::vector<std::string> results;
    std::vector<std::string> errors;
    bool enabled = false;

    std::string amount_text() const override { return amount; }
    std::string from_currency() const override { return from; }
    std::string to_currency() const override { return to; }
    void show_result(const std::string& text) override { results.push_back(text); }
    void show_error(const std::string& text) override { errors.push_back(text); }
    void set_convert_enabled(bool value) override { enabled = value; }
};
}  // namespace

TEST_CASE("the model converts through the euro") {
    const RatesModel model;
    CHECK_NEAR(model.convert(100, "EUR", "USD"), 108.0, 1e-9);
    CHECK_NEAR(model.convert(108, "USD", "EUR"), 100.0, 1e-9);
    CHECK_NEAR(model.convert(85, "GBP", "JPY"), 16'200.0, 1e-6);
    CHECK_THROWS_AS(model.convert(1, "EUR", "XYZ"), std::invalid_argument);
    CHECK_EQ(model.currencies().size(), 5u);
}

TEST_CASE("parse_amount accepts friendly formats and rejects bad input") {
    CHECK_NEAR(parse_amount("1,250.50").value_or(-1), 1250.5, 1e-9);
    CHECK_NEAR(parse_amount("  12 ").value_or(-1), 12.0, 1e-9);
    CHECK(!parse_amount("").has_value());
    CHECK(!parse_amount("ten").has_value());
    CHECK(!parse_amount("-5").has_value());
    CHECK(!parse_amount("1.234").has_value());
    CHECK(!parse_amount("12abc").has_value());
}

TEST_CASE("the presenter enables Convert only for usable input") {
    FakeView view;
    const RatesModel model;
    ConverterPresenter presenter(view, model);
    CHECK(view.enabled);
    view.amount = "abc";
    presenter.on_input_changed();
    CHECK(!view.enabled);
    view.amount = "50";
    view.to = "XYZ";
    presenter.on_input_changed();
    CHECK(!view.enabled);
}

TEST_CASE("converting shows a formatted result") {
    FakeView view;
    const RatesModel model;
    ConverterPresenter presenter(view, model);
    presenter.on_convert_clicked();
    REQUIRE_EQ(view.results.size(), 1u);
    CHECK_EQ(view.results[0], "100.00 EUR = 108.00 USD");
    view.to = "JPY";
    presenter.on_convert_clicked();
    CHECK_EQ(view.results[1], "100.00 EUR = 16200 JPY");
}

TEST_CASE("invalid input produces an error message instead of a result") {
    FakeView view;
    view.amount = "lots";
    const RatesModel model;
    ConverterPresenter presenter(view, model);
    presenter.on_convert_clicked();
    CHECK(view.results.empty());
    CHECK_EQ(view.errors.at(0), "Enter an amount such as 1,250.00");
    view.amount = "10";
    view.from = "ABC";
    presenter.on_convert_clicked();
    CHECK_EQ(view.errors.at(1), "Choose two supported currencies");
}

TEST_CASE("the console view is just another implementation") {
    std::ostringstream out;
    ConsoleView view(out);
    view.set_fields("20", "GBP", "EUR");
    const RatesModel model;
    ConverterPresenter presenter(view, model);
    CHECK(view.convert_enabled());
    presenter.on_convert_clicked();
    CHECK_EQ(out.str(), "  20.00 GBP = 23.53 EUR\n");
}

TEST_CASE("run drives the presenter through the console view") {
    std::istringstream in("1,250.00 EUR JPY\nabc EUR USD\n\n");
    std::ostringstream out;
    CHECK_EQ(run(in, out), 0);
    const auto text = out.str();
    CHECK(text.find("1250.00 EUR = 202500 JPY") != std::string::npos);
    CHECK(text.find("[Convert button disabled]") != std::string::npos);
    CHECK(text.find("! Enter an amount") != std::string::npos);
}
