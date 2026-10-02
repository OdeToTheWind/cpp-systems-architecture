/**
 * @file
 * Day 71 – Functional Tools.
 *
 * Scenario: the *pricing rules engine* of an online shop. Discounts, taxes and rounding are small
 * functions that marketing can combine per campaign: rules are lambdas that capture their
 * settings, pipelines are built by composing functions, member pointers are called through
 * std::invoke, regional tax uses std::bind_front, and an expensive shipping-rate lookup is
 * memoised.
 *
 * Deliverables (syllabus):
 * - Lambdas and captures
 * - std::invoke and std::bind_front
 * - Higher-order functions
 * - Memoisation
 */
#pragma once

#include <cmath>
#include <functional>
#include <istream>
#include <map>
#include <ostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

#include "cppm/lesson.hpp"

namespace cppm::day71 {

inline constexpr Deliverable DELIVERABLES[] = {
    {"lambdas that capture their settings by value", "percent_off"},
    {"an init-capture with mutable state", "make_ticket_counter"},
    {"composing functions into a pipeline", "compose"},
    {"summing any member through std::invoke", "total_of"},
    {"pre-binding arguments with std::bind_front", "regional_tax"},
    {"caching the results of an expensive function", "Memoised"},
};

using Rule = std::function<double(double)>;

/// A discount rule. Captures @p percent *by value*: the rule keeps working after the caller's
/// variable is gone or changed.
inline Rule percent_off(double percent) {
    if (percent < 0 || percent > 100) throw std::invalid_argument("percent must be 0-100");
    return [percent](double price) { return price * (1 - percent / 100); };
}

inline Rule minimum_price(double floor) {
    return [floor](double price) { return price < floor ? floor : price; };
}

inline Rule round_to_cents() {
    return [](double price) { return std::round(price * 100) / 100; };
}

/// Each call returns the next ticket number. `next = first` is an init-capture, and `mutable`
/// lets the lambda change its own copy.
using Counter = std::function<int()>;

inline Counter make_ticket_counter(int first) {
    return [next = first]() mutable { return next++; };
}

/// compose(f, g, h)(x) == h(g(f(x))): apply left to right, like a pipeline.
template <typename F>
auto compose(F f) {
    return f;
}
template <typename F, typename... Rest>
auto compose(F f, Rest... rest) {
    return [f, next = compose(rest...)](auto&&... args) {
        return next(std::invoke(f, std::forward<decltype(args)>(args)...));
    };
}

/// Apply a list of rules in order – a higher-order function: it takes functions as data.
inline double apply_rules(double price, const std::vector<Rule>& rules) {
    for (const auto& rule : rules) price = rule(price);
    return price;
}

struct Item {
    std::string sku;
    double price;
    int quantity;
    double line_total() const { return price * quantity; }
};

/// Sum any member or member function over the items: std::invoke calls a data-member pointer,
/// a member-function pointer and an ordinary callable in the same way.
template <typename Projection>
double total_of(const std::vector<Item>& items, Projection projection) {
    double total = 0;
    for (const auto& item : items) total += static_cast<double>(std::invoke(projection, item));
    return total;
}

/// Tax for a region applied to a price; regional_tax binds the table and region in front.
inline double add_tax(const std::map<std::string, double>& rates, const std::string& region, double price) {
    const auto it = rates.find(region);
    if (it == rates.end()) throw std::out_of_range("no tax rate for " + region);
    return price * (1 + it->second);
}

inline Rule regional_tax(const std::map<std::string, double>& rates, const std::string& region) {
    return std::bind_front(add_tax, rates, region);  // copies rates and region into the result
}

/// Wraps a function and caches its results by argument. calls() counts real evaluations.
template <typename Result, typename... Args>
class Memoised {
  public:
    explicit Memoised(std::function<Result(Args...)> function) : function_(std::move(function)) {}
    const Result& operator()(const Args&... args) {
        auto key = std::make_tuple(args...);
        auto it = cache_.find(key);
        if (it == cache_.end()) {
            ++calls_;
            it = cache_.emplace(std::move(key), function_(args...)).first;
        }
        return it->second;
    }
    int calls() const { return calls_; }

  private:
    std::function<Result(Args...)> function_;
    std::map<std::tuple<Args...>, Result> cache_;
    int calls_ = 0;
};

/// The "expensive" shipping-rate lookup: in production a call to the carrier's API.
inline double shipping_rate(const std::string& country, int grams) {
    const double base = country == "PT" ? 3.0 : country == "DE" ? 4.5 : 9.0;
    return base + 0.5 * ((grams + 499) / 500);
}

/// The interactive demo: "price campaign region country grams".
inline int run(std::istream& in, std::ostream& out) {
    out << "Day 71 – Functional Tools\n";
    const std::map<std::string, double> vat{{"PT", 0.23}, {"DE", 0.19}, {"FR", 0.20}};
    const std::map<std::string, std::vector<Rule>> campaigns{
        {"none", {}}, {"spring", {percent_off(15), minimum_price(5)}}, {"vip", {percent_off(25), percent_off(10)}}};
    Memoised<double, std::string, int> shipping(shipping_rate);
    auto ticket = make_ticket_counter(1001);
    while (auto line = prompt_line(in, out, "price campaign region grams> ")) {
        std::istringstream words(*line);
        double price = 0;
        std::string campaign;
        std::string region;
        int grams = 0;
        if (!(words >> price >> campaign >> region >> grams)) break;
        try {
            const auto pipeline = compose([&](double p) { return apply_rules(p, campaigns.at(campaign)); },
                                          regional_tax(vat, region), round_to_cents());
            const double total = pipeline(price) + shipping(region, grams);
            out << "  quote #" << ticket() << ": " << total << '\n';
        } catch (const std::out_of_range& error) {
            out << "  unknown campaign or region\n";
        }
    }
    out << shipping.calls() << " shipping lookup(s)\n";
    return 0;
}

}  // namespace cppm::day71
