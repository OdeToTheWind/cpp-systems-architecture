// Definitions for lesson.hpp. The *what* is documented in the header; comments here explain *why*.
#include "lesson.hpp"

#include <algorithm>
#include <array>
#include <cctype>
#include <iterator>
#include <regex>

namespace cppm::day22 {
namespace {

bool is_mass(Unit unit) {
    return unit == Unit::gram || unit == Unit::kilogram || unit == Unit::ounce || unit == Unit::pound;
}

// Each unit's size in the base unit of its dimension: grams for mass, millilitres for volume.
double in_base_units(Unit unit) {
    switch (unit) {
        case Unit::gram:
            return 1.0;
        case Unit::kilogram:
            return 1000.0;
        case Unit::ounce:
            return 28.349523125;  // avoirdupois ounce, exact by definition
        case Unit::pound:
            return 453.59237;
        case Unit::millilitre:
            return 1.0;
        case Unit::litre:
            return 1000.0;
        case Unit::cup:
            return cups_to_millilitres(1.0);
        case Unit::tablespoon:
            return 14.786765;
        case Unit::teaspoon:
            return 4.928922;
    }
    return 1.0;
}

std::string lowercase_words(std::string_view text) {
    std::string out;
    for (const char c : text) {
        const auto u = static_cast<unsigned char>(c);
        if (std::isalnum(u)) {
            out += static_cast<char>(std::tolower(u));
        } else if (!out.empty() && out.back() != ' ') {
            out += ' ';
        }
    }
    while (!out.empty() && out.back() == ' ') {
        out.pop_back();
    }
    return out;
}

}  // namespace

double convert(double amount, Unit from, Unit to) {
    if (!std::isfinite(amount) || amount < 0) {
        throw std::invalid_argument("amount must be a finite, non-negative number");
    }
    if (is_mass(from) != is_mass(to)) {
        throw std::domain_error("cannot convert between mass and volume without a density");
    }
    return amount * in_base_units(from) / in_base_units(to);
}

std::vector<double> scale_recipe(const std::vector<double>& amounts, double factor) {
    if (!(factor > 0)) {
        throw std::invalid_argument("scale factor must be positive");
    }
    std::vector<double> scaled;
    scaled.reserve(amounts.size());
    std::transform(amounts.begin(), amounts.end(), std::back_inserter(scaled),
                   [factor](double amount) { return amount * factor; });
    return scaled;
}

std::vector<std::string> find_undocumented(std::string_view header_text) {
    // A deliberately small heuristic: a line that declares a function at namespace level,
    // and the closest non-blank line above it. Real projects use doxygen's WARN_IF_UNDOCUMENTED.
    static const std::regex declaration(R"(^(?:inline\s+)?[\w:<>,\s*&]+?\s+[*&]?(\w+)\s*\([^;{]*\)\s*(?:const)?\s*[;{])");
    std::vector<std::string> missing;
    std::istringstream lines{std::string(header_text)};
    std::string line;
    std::string previous;
    while (std::getline(lines, line)) {
        std::smatch match;
        if (std::regex_search(line, match, declaration)) {
            const auto first = previous.find_first_not_of(" \t");
            const std::string above = first == std::string::npos ? "" : previous.substr(first);
            const bool documented = above.rfind("///", 0) == 0 || above.rfind("*/", 0) == 0 ||
                                    (above.size() >= 2 && above.substr(above.size() - 2) == "*/");
            if (!documented) {
                missing.push_back(match[1]);
            }
        }
        if (line.find_first_not_of(" \t") != std::string::npos) {
            previous = line;
        }
    }
    return missing;
}

bool is_what_comment(std::string_view code, std::string_view comment) {
    // If every word of the comment already appears in the code, the comment adds nothing.
    const std::string code_words = " " + lowercase_words(code) + " ";
    std::istringstream words(lowercase_words(comment));
    std::string word;
    int checked = 0;
    static const std::array<std::string_view, 8> filler{"the", "a", "an", "by", "to", "of", "and", "one"};
    while (words >> word) {
        if (std::find(filler.begin(), filler.end(), word) != filler.end()) {
            continue;
        }
        ++checked;
        const bool synonym = (word == "increment" && code_words.find(" i ") != std::string::npos);
        if (!synonym && code_words.find(" " + word + " ") == std::string::npos) {
            return false;
        }
    }
    return checked > 0;
}

std::optional<Unit> parse_unit(std::string_view name) {
    if (name == "g" || name == "gram") return Unit::gram;
    if (name == "kg" || name == "kilogram") return Unit::kilogram;
    if (name == "oz" || name == "ounce") return Unit::ounce;
    if (name == "lb" || name == "pound") return Unit::pound;
    if (name == "ml" || name == "millilitre") return Unit::millilitre;
    if (name == "l" || name == "litre") return Unit::litre;
    if (name == "cup") return Unit::cup;
    if (name == "tbsp" || name == "tablespoon") return Unit::tablespoon;
    if (name == "tsp" || name == "teaspoon") return Unit::teaspoon;
    return std::nullopt;
}

int run(std::istream& in, std::ostream& out) {
    out << "Day 22 – Documentation vs Comments\nConvert with 'amount from to', e.g. 2 cup ml\n";
    while (auto line = prompt_line(in, out, "convert> ")) {
        if (line->empty()) {
            break;
        }
        std::istringstream words(*line);
        double amount = 0;
        std::string from;
        std::string to;
        if (!(words >> amount >> from >> to)) {
            out << "  e.g. 8 oz g\n";
            continue;
        }
        const auto from_unit = parse_unit(from);
        const auto to_unit = parse_unit(to);
        if (!from_unit || !to_unit) {
            out << "  units: g kg oz lb ml l cup tbsp tsp\n";
            continue;
        }
        try {
            out << "  " << amount << ' ' << from << " = " << convert(amount, *from_unit, *to_unit) << ' ' << to << '\n';
        } catch (const std::exception& error) {
            out << "  " << error.what() << '\n';
        }
    }
    return 0;
}

}  // namespace cppm::day22
