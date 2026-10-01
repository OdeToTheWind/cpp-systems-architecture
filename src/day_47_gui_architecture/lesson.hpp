/**
 * @file
 * Day 47 – Desktop GUI Architecture.
 *
 * Scenario: a *travel currency converter* desktop app built with Model-View-Presenter. The
 * model knows exchange rates, the view is a thin interface any toolkit (Qt, wxWidgets, a
 * console) can implement, and the presenter holds every UI rule – so the whole app is tested
 * headlessly with a fake view, no window needed.
 *
 * Deliverables (syllabus):
 * - Model-View-Presenter
 * - Widget-free presenters
 * - Input validation
 * - Headless testing of UI logic
 */
#pragma once

#include <cmath>
#include <iomanip>
#include <istream>
#include <map>
#include <optional>
#include <ostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

#include "cppm/lesson.hpp"

namespace cppm::day47 {

inline constexpr Deliverable DELIVERABLES[] = {
    {"the model: data and rules, no UI", "RatesModel"},
    {"the view: a passive interface any toolkit can implement", "ConverterView"},
    {"the presenter: every UI rule, no widgets", "ConverterPresenter"},
    {"validating text typed into a field", "parse_amount"},
    {"a console view as one concrete implementation", "ConsoleView"},
};

/// Exchange rates against EUR.
class RatesModel {
public:
    RatesModel() : per_euro_{{"EUR", 1.0}, {"USD", 1.08}, {"GBP", 0.85}, {"JPY", 162.0}, {"INR", 90.0}} {}
    bool supports(const std::string& code) const { return per_euro_.contains(code); }
    std::vector<std::string> currencies() const {
        std::vector<std::string> codes;
        for (const auto& [code, rate] : per_euro_) codes.push_back(code);
        return codes;
    }
    double convert(double amount, const std::string& from, const std::string& to) const {
        if (!supports(from) || !supports(to)) {
            throw std::invalid_argument("unsupported currency");
        }
        return amount / per_euro_.at(from) * per_euro_.at(to);
    }
    void set_rate(const std::string& code, double per_euro) {
        if (!(per_euro > 0)) throw std::invalid_argument("rates must be positive");
        per_euro_[code] = per_euro;
    }

private:
    std::map<std::string, double> per_euro_;
};

/// Everything the presenter needs from a screen – and nothing toolkit-specific.
class ConverterView {
public:
    virtual ~ConverterView() = default;
    virtual std::string amount_text() const = 0;
    virtual std::string from_currency() const = 0;
    virtual std::string to_currency() const = 0;
    virtual void show_result(const std::string& text) = 0;
    virtual void show_error(const std::string& text) = 0;
    virtual void set_convert_enabled(bool enabled) = 0;
};

/// Accepts "1234.5", "1,234.50" and "  12 "; rejects text, negatives, NaN and more than 2 decimals.
inline std::optional<double> parse_amount(std::string text) {
    std::string cleaned;
    for (const char c : text) {
        if (c != ',' && c != ' ') cleaned += c;
    }
    if (cleaned.empty()) return std::nullopt;
    const auto dot = cleaned.find('.');
    if (dot != std::string::npos && cleaned.size() - dot - 1 > 2) return std::nullopt;
    std::istringstream in(cleaned);
    double value = 0;
    if (!(in >> value) || !(in >> std::ws).eof() || !std::isfinite(value) || value < 0) {
        return std::nullopt;
    }
    return value;
}

/// The presenter reacts to view events and decides what the view shows.
class ConverterPresenter {
public:
    ConverterPresenter(ConverterView& view, const RatesModel& model) : view_(view), model_(model) {
        on_input_changed();
    }

    /// Called on every keystroke: enable the button only when the input is usable.
    void on_input_changed() {
        const bool valid = parse_amount(view_.amount_text()).has_value() && model_.supports(view_.from_currency()) &&
                           model_.supports(view_.to_currency());
        view_.set_convert_enabled(valid);
    }

    /// Called when the user presses Convert.
    void on_convert_clicked() {
        const auto amount = parse_amount(view_.amount_text());
        if (!amount) {
            view_.show_error("Enter an amount such as 1,250.00");
            return;
        }
        if (!model_.supports(view_.from_currency()) || !model_.supports(view_.to_currency())) {
            view_.show_error("Choose two supported currencies");
            return;
        }
        const double converted = model_.convert(*amount, view_.from_currency(), view_.to_currency());
        std::ostringstream left;
        left << std::fixed << std::setprecision(2) << *amount;
        std::ostringstream right;
        right << std::fixed << std::setprecision(view_.to_currency() == "JPY" ? 0 : 2) << converted;  // yen has no cents
        view_.show_result(left.str() + ' ' + view_.from_currency() + " = " + right.str() + ' ' + view_.to_currency());
    }

private:
    ConverterView& view_;
    const RatesModel& model_;
};

/// A text-mode implementation of the view; a Qt version would wrap QLineEdit/QComboBox instead.
class ConsoleView final : public ConverterView {
public:
    explicit ConsoleView(std::ostream& out) : out_(out) {}
    void set_fields(std::string amount, std::string from, std::string to) {
        amount_ = std::move(amount);
        from_ = std::move(from);
        to_ = std::move(to);
    }
    std::string amount_text() const override { return amount_; }
    std::string from_currency() const override { return from_; }
    std::string to_currency() const override { return to_; }
    void show_result(const std::string& text) override { out_ << "  " << text << '\n'; }
    void show_error(const std::string& text) override { out_ << "  ! " << text << '\n'; }
    void set_convert_enabled(bool enabled) override { enabled_ = enabled; }
    bool convert_enabled() const { return enabled_; }

private:
    std::ostream& out_;
    std::string amount_;
    std::string from_{"EUR"};
    std::string to_{"USD"};
    bool enabled_{false};
};

/// The interactive demo: "amount FROM TO" lines are "typed" into the console view.
inline int run(std::istream& in, std::ostream& out) {
    out << "Day 47 – Desktop GUI Architecture\nType 'amount FROM TO', e.g. 1,250.00 EUR JPY\n";
    const RatesModel model;
    ConsoleView view(out);
    ConverterPresenter presenter(view, model);
    while (auto line = prompt_line(in, out, "convert> ")) {
        if (line->empty()) break;
        std::istringstream words(*line);
        std::string amount;
        std::string from;
        std::string to;
        words >> amount >> from >> to;
        view.set_fields(amount, from, to);
        presenter.on_input_changed();
        out << "  [Convert button " << (view.convert_enabled() ? "enabled" : "disabled") << "]\n";
        presenter.on_convert_clicked();
    }
    return 0;
}

}  // namespace cppm::day47
