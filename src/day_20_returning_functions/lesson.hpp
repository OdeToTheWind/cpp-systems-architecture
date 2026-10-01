/**
 * @file
 * Day 20 – Returning Functions.
 *
 * Scenario: a *shipping-rate engine for an online shop*. Carrier rates are plain functions
 * looked up through function pointers, promotions are lambdas built at run time and composed
 * into one pricing rule, quotes come back as structs, and the checkout page receives each
 * quote through a callback.
 *
 * Deliverables (syllabus):
 * - Returning values and structs
 * - Function pointers
 * - std::function
 * - Lambdas returned from functions
 * - Callbacks
 */
#pragma once

#include <functional>
#include <istream>
#include <optional>
#include <ostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

#include "cppm/lesson.hpp"

namespace cppm::day20 {

inline constexpr Deliverable DELIVERABLES[] = {
    {"returning a struct with several values", "Quote"},
    {"a function pointer type and a lookup returning one", "rate_function_for"},
    {"a lambda returned from a function, capturing by value", "make_discount"},
    {"std::function composing two pricing rules", "compose"},
    {"returning std::optional when there may be no answer", "cheapest_quote"},
    {"a callback invoked for every result", "quote_all"},
};

/// Everything the shop needs to show one shipping option.
struct Quote {
    std::string carrier;
    long long cents;
    int days;
};

/// Carrier tariffs: ordinary functions with the same signature.
inline long long post_rate(int grams) { return 450 + 3LL * grams / 10; }       // 4.50 + 0.03 per 10 g
inline long long courier_rate(int grams) { return 900 + grams / 100 * 30LL; }  // 9.00 + 0.30 per 100 g
inline long long freight_rate(int grams) { return grams < 5'000 ? 4'000 : 4'000 + (grams - 5'000) / 1'000 * 150LL; }

/// A pointer to any function taking grams and returning cents.
using RateFunction = long long (*)(int);

/// Look up a carrier's tariff; nullptr for an unknown carrier.
inline RateFunction rate_function_for(std::string_view carrier) {
    if (carrier == "post") return &post_rate;
    if (carrier == "courier") return &courier_rate;
    if (carrier == "freight") return freight_rate;  // a function name decays to a pointer, & is optional
    return nullptr;
}

inline int delivery_days(std::string_view carrier) { return carrier == "courier" ? 1 : carrier == "post" ? 3 : 5; }

/// A pricing rule maps cents to cents.
using PriceRule = std::function<long long(long long)>;

/// Returns a lambda that remembers @p percent. Capture by value: the lambda outlives this call.
inline PriceRule make_discount(int percent) {
    if (percent < 0 || percent > 100) {
        throw std::invalid_argument("discount must be 0-100 %");
    }
    return [percent](long long cents) { return cents - cents * percent / 100; };
}

/// Returns a lambda that adds a flat fee (for example remote-area delivery).
inline PriceRule make_surcharge(long long cents_to_add) {
    return [cents_to_add](long long cents) { return cents + cents_to_add; };
}

/// first, then second: `compose(f, g)(x) == g(f(x))`.
inline PriceRule compose(PriceRule first, PriceRule second) {
    return [first = std::move(first), second = std::move(second)](long long cents) { return second(first(cents)); };
}

/// A full quote for one carrier, with an optional pricing rule applied.
inline Quote quote(std::string_view carrier, int grams, const PriceRule& rule = {}) {
    const RateFunction rate = rate_function_for(carrier);
    if (rate == nullptr) {
        throw std::invalid_argument("unknown carrier");
    }
    if (grams <= 0) {
        throw std::invalid_argument("weight must be positive");
    }
    long long cents = rate(grams);  // calling through the function pointer
    if (rule) {                     // an empty std::function converts to false
        cents = rule(cents);
    }
    return {std::string(carrier), cents, delivery_days(carrier)};
}

/// The cheapest of @p carriers, or std::nullopt if none of them is known.
inline std::optional<Quote> cheapest_quote(int grams, const std::vector<std::string>& carriers,
                                           const PriceRule& rule = {}) {
    std::optional<Quote> best;
    for (const auto& carrier : carriers) {
        if (rate_function_for(carrier) == nullptr) {
            continue;
        }
        Quote q = quote(carrier, grams, rule);
        if (!best || q.cents < best->cents) {
            best = q;
        }
    }
    return best;
}

/// Quote every parcel and hand each result to @p on_quote – the caller decides what to do with it.
inline int quote_all(const std::vector<int>& parcel_grams, std::string_view carrier,
                     const std::function<void(const Quote&)>& on_quote) {
    int delivered = 0;
    for (int grams : parcel_grams) {
        on_quote(quote(carrier, grams));
        ++delivered;
    }
    return delivered;
}

/// The interactive demo: "grams [discount%] [surcharge-cents]" → the cheapest option.
inline int run(std::istream& in, std::ostream& out) {
    out << "Day 20 – Returning Functions\n";
    const std::vector<std::string> carriers{"post", "courier", "freight"};
    while (auto line = prompt_line(in, out, "grams [discount%] [surcharge]> ")) {
        if (line->empty()) {
            break;
        }
        std::istringstream words(*line);
        int grams = 0;
        int discount = 0;
        long long surcharge = 0;
        if (!(words >> grams)) {
            out << "  e.g. 2500 10 300\n";
            continue;
        }
        words >> discount >> surcharge;
        try {
            const PriceRule rule = compose(make_discount(discount), make_surcharge(surcharge));
            if (auto best = cheapest_quote(grams, carriers, rule)) {
                out << "  cheapest: " << best->carrier << ' ' << best->cents << " cents, " << best->days << " day(s)\n";
            }
        } catch (const std::invalid_argument& error) {
            out << "  " << error.what() << '\n';
        }
    }
    long long total = 0;
    quote_all({500, 1'500}, "post", [&](const Quote& q) {
        out << "  callback: post " << q.cents << " cents\n";
        total += q.cents;
    });
    out << "Batch total: " << total << " cents\n";
    return 0;
}

}  // namespace cppm::day20
