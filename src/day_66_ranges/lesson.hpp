/**
 * @file
 * Day 66 – Ranges & Views.
 *
 * Scenario: *order analytics for a small online shop*. Questions such as "what did paid orders
 * over €50 earn?", "who are the top three customers?" or "which tags appear most?" are answered
 * with range algorithms and lazy view pipelines instead of hand-written loops and temporary
 * vectors.
 *
 * Deliverables (syllabus):
 * - Range algorithms
 * - Lazy views
 * - Composing pipelines
 * - Projections
 */
#pragma once

#include <algorithm>
#include <istream>
#include <map>
#include <numeric>
#include <ostream>
#include <ranges>
#include <sstream>
#include <string>
#include <string_view>
#include <vector>

#include "cppm/lesson.hpp"

namespace cppm::day66 {

inline constexpr Deliverable DELIVERABLES[] = {
    {"a filter-and-transform pipeline over orders", "paid_amounts"},
    {"sorting with a projection instead of a comparator", "sort_by_total"},
    {"taking the first n results lazily", "top_customers"},
    {"proving that views are lazy", "count_evaluations"},
    {"splitting text with a view", "split_tags"},
};

struct Order {
    int id;
    std::string customer;
    long cents;
    bool paid;
    std::string tags;  // comma-separated, e.g. "gift,express"
};

/// Amounts of paid orders of at least @p min_cents – a lazy pipeline, materialised once.
inline std::vector<long> paid_amounts(const std::vector<Order>& orders, long min_cents = 0) {
    auto pipeline = orders | std::views::filter([](const Order& o) { return o.paid; }) |
                    std::views::transform(&Order::cents) |
                    std::views::filter([min_cents](long cents) { return cents >= min_cents; });
    return {pipeline.begin(), pipeline.end()};
}

/// Sort largest first. The projection &Order::cents says *what* to compare; std::greater says how.
inline void sort_by_total(std::vector<Order>& orders) {
    std::ranges::stable_sort(orders, std::ranges::greater{}, &Order::cents);
}

/// Customers ranked by paid total, highest first; ties alphabetical. Only the first @p n are taken.
inline std::vector<std::pair<std::string, long>> top_customers(const std::vector<Order>& orders, std::size_t n) {
    std::map<std::string, long> totals;
    for (const Order& o : orders | std::views::filter(&Order::paid)) totals[o.customer] += o.cents;
    std::vector<std::pair<std::string, long>> ranked(totals.begin(), totals.end());  // already alphabetical
    std::ranges::stable_sort(ranked, std::ranges::greater{}, &std::pair<std::string, long>::second);
    auto first = ranked | std::views::take(n);
    return {first.begin(), first.end()};
}

/// How many times the transform runs when the first @p wanted squares are taken from an
/// unbounded sequence. Views do work only on demand, so an infinite input is fine: exactly
/// @p wanted calls happen. (Caution: a filter placed *after* a transform calls the transform
/// twice per accepted element – once to test it and once when it is read.)
inline int count_evaluations(int wanted) {
    int calls = 0;
    auto squares = std::views::iota(1) | std::views::transform([&calls](int i) {
                       ++calls;
                       return i * i;
                   }) |
                   std::views::take(wanted);
    for (const int value : squares) static_cast<void>(value);
    return calls;
}

/// "gift, express,,gift" -> {"gift", "express", "gift"}: split, trim, drop empties.
inline std::vector<std::string> split_tags(std::string_view tags) {
    std::vector<std::string> out;
    for (const auto part : tags | std::views::split(',')) {
        std::string_view piece(part.begin(), part.end());
        while (!piece.empty() && piece.front() == ' ') piece.remove_prefix(1);
        while (!piece.empty() && piece.back() == ' ') piece.remove_suffix(1);
        if (!piece.empty()) out.emplace_back(piece);
    }
    return out;
}

/// Tag counts across all orders, most frequent first.
inline std::vector<std::pair<std::string, int>> tag_counts(const std::vector<Order>& orders) {
    std::map<std::string, int> counts;
    for (const auto& tag : orders | std::views::transform(&Order::tags) | std::views::transform(split_tags) | std::views::join) {
        ++counts[tag];
    }
    std::vector<std::pair<std::string, int>> ranked(counts.begin(), counts.end());
    std::ranges::stable_sort(ranked, std::ranges::greater{}, &std::pair<std::string, int>::second);
    return ranked;
}

/// The largest paid order, found with a projection; nullptr when nothing is paid.
inline const Order* largest_paid(const std::vector<Order>& orders) {
    auto paid = orders | std::views::filter(&Order::paid);
    const auto it = std::ranges::max_element(paid, {}, &Order::cents);
    return it == paid.end() ? nullptr : &*it;
}

inline std::string euros(long cents) {
    return "€" + std::to_string(cents / 100) + "." + (cents % 100 < 10 ? "0" : "") + std::to_string(cents % 100);
}

/// The interactive demo: "id customer cents paid(0/1) tags" lines, then a report.
inline int run(std::istream& in, std::ostream& out) {
    out << "Day 66 – Ranges & Views\n";
    std::vector<Order> orders;
    while (auto line = prompt_line(in, out, "id customer cents paid tags> ")) {
        std::istringstream words(*line);
        Order o{};
        int paid = 0;
        if (!(words >> o.id >> o.customer >> o.cents >> paid)) break;
        o.paid = paid != 0;
        words >> o.tags;
        orders.push_back(o);
    }
    const auto amounts = paid_amounts(orders, 5000);
    out << amounts.size() << " paid order(s) of at least €50 worth " << euros(std::accumulate(amounts.begin(), amounts.end(), 0L))
        << '\n';
    for (const auto& [customer, cents] : top_customers(orders, 3)) out << "  top: " << customer << ' ' << euros(cents) << '\n';
    for (const auto& [tag, count] : tag_counts(orders)) out << "  tag " << tag << " x" << count << '\n';
    if (const Order* best = largest_paid(orders)) out << "largest paid order: #" << best->id << '\n';
    return 0;
}

}  // namespace cppm::day66
