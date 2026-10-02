// Definitions for the declarations in lesson.hpp. Only this file knows the price list.
#include "lesson.hpp"

#include <iomanip>
#include <map>
#include <sstream>
#include <stdexcept>

namespace cppm::day12 {
namespace {  // internal linkage: helpers invisible outside this file

const std::map<std::string, long long>& price_list() {
    static const std::map<std::string, long long> prices{
        {"croissant", 250}, {"baguette", 320}, {"muffin", 290}, {"cinnamon roll", 380}, {"coffee", 300}};
    return prices;
}

long long size_factor_percent(Size size) {
    switch (size) {
        case Size::small: return 80;
        case Size::medium: return 100;
        case Size::large: return 130;
    }
    return 100;
}

}  // namespace

long long unit_price(const std::string& item, Size size) {  // no default argument here
    const auto found = price_list().find(item);
    if (found == price_list().end()) {
        throw std::invalid_argument("we do not bake '" + item + "'");
    }
    return (found->second * size_factor_percent(size) + 50) / 100;
}

long long add_item(Order& order, const std::string& item, int quantity, Size size) {
    if (quantity <= 0) {
        throw std::invalid_argument("quantity must be positive");
    }
    const long long line_total = unit_price(item, size) * quantity;
    order.lines.push_back({item, quantity, size, line_total});
    return line_total;
}

long long order_total(const Order& order) {
    long long total = 0;
    for (const auto& line : order.lines) {
        total += line.line_total_cents;
    }
    return total;
}

long long apply_loyalty(long long total, int stamps) {
    if (stamps < 0) {
        throw std::invalid_argument("stamps cannot be negative");
    }
    const long long rewards = stamps / 10;
    for (long long i = 0; i < rewards; ++i) {
        total -= total / 10;  // modifies the local copy only
    }
    return total;
}

std::string format_price(long long cents) {
    std::ostringstream out;
    out << cents / 100 << '.' << std::setw(2) << std::setfill('0') << cents % 100;
    return out.str();
}

std::string format_price(long long cents, const std::string& currency) {
    return format_price(cents) + ' ' + currency;  // reuses overload 1
}

std::string format_price(const Order& order, const std::string& currency) {
    return format_price(order_total(order), currency);
}

Size parse_size(const std::string& word) {
    if (word == "small") return Size::small;
    if (word == "large") return Size::large;
    return Size::medium;
}

int run(std::istream& in, std::ostream& out) {
    out << "Day 12 – Functions\nOrder lines as '<quantity> <item> [small|medium|large]', blank to pay\n";
    Order order;
    while (auto line = prompt_line(in, out, "order> ")) {
        if (line->empty()) {
            break;
        }
        std::istringstream words(*line);
        int quantity{};
        std::string item;
        if (!(words >> quantity >> item)) {
            out << "  e.g. 2 croissant large\n";
            continue;
        }
        std::string rest;
        std::string size_word = "medium";
        while (words >> rest) {  // multi-word items such as "cinnamon roll"
            if (rest == "small" || rest == "medium" || rest == "large") {
                size_word = rest;
            } else {
                item += ' ' + rest;
            }
        }
        try {
            const long long added = add_item(order, item, quantity, parse_size(size_word));
            out << "  + " << quantity << " x " << item << " = " << format_price(added) << '\n';
        } catch (const std::invalid_argument& error) {
            out << "  " << error.what() << '\n';
        }
    }
    out << "Subtotal: " << format_price(order) << '\n';
    auto stamps_line = prompt_line(in, out, "Loyalty stamps: ");
    int stamps = 0;
    if (stamps_line) {
        std::istringstream(*stamps_line) >> stamps;
    }
    out << "To pay: " << format_price(apply_loyalty(order_total(order), stamps < 0 ? 0 : stamps), "EUR") << '\n';
    return 0;
}

}  // namespace cppm::day12
