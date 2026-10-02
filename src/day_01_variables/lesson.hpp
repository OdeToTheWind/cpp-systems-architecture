/**
 * @file
 * Day 01 – Variables, Types & Basic I/O.
 *
 * Scenario: a *coding-club sign-up desk* that records each new member's profile card
 * (name, age, height, membership tier, newsletter choice) and offers a mini calculator
 * for splitting the club's membership fees.
 *
 * Deliverables (syllabus):
 * - Fundamental types, brace initialisation, auto and const
 * - Reading values with std::cin and std::getline
 * - A mini calculator
 */
#pragma once

#include <iomanip>
#include <istream>
#include <optional>
#include <ostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include "cppm/lesson.hpp"

namespace cppm::day01 {

inline constexpr Deliverable DELIVERABLES[] = {
    {"fundamental types in one record", "MemberCard"},
    {"brace initialisation, auto and const", "type_tour"},
    {"parsing a whole line into a number", "parse_number"},
    {"reading values with std::getline", "read_member"},
    {"a mini calculator", "calculate"},
};

/// A member's profile card: one field per fundamental type, every field initialised.
struct MemberCard {
    std::string name;        // text of any length
    int age{0};              // whole number
    double height_m{0.0};    // real number
    char tier{'B'};          // one character: 'B'asic or 'P'ro
    bool newsletter{false};  // yes / no
};

/// One `(declaration, value)` row per variable, showing brace initialisation, auto and const.
inline std::vector<std::pair<std::string, std::string>> type_tour() {
    const int members{42};  // const: can never change after initialisation
    auto fee = 12.5;        // auto deduces double from the literal
    const char tier{'P'};
    bool open{true};
    auto club = std::string{"cpp-club"};
    long long budget_cents{1'250'000};  // digit separators make big numbers readable
    // int narrowed{fee};            // would not compile: brace init forbids narrowing double -> int

    std::ostringstream fee_text;
    fee_text << std::fixed << std::setprecision(2) << fee;
    return {
        {"const int members{42}", std::to_string(members)},
        {"auto fee = 12.5   (double)", fee_text.str()},
        {"const char tier{'P'}", std::string(1, tier)},
        {"bool open{true}", open ? "true" : "false"},
        {"auto club = std::string{...}", club},
        {"long long budget_cents{1'250'000}", std::to_string(budget_cents)},
    };
}

/// Parse a whole line as a number of type @p T; std::nullopt if anything but the number is there.
template <typename T>
std::optional<T> parse_number(const std::string& text) {
    std::istringstream stream(text);
    T value{};
    if (!(stream >> value)) {
        return std::nullopt;
    }
    stream >> std::ws;
    if (!stream.eof()) {  // "12abc" or "3 4" must not be accepted as 12 or 3
        return std::nullopt;
    }
    return value;
}

/// Ask until the user types a valid number; std::nullopt only at end of input.
template <typename T>
std::optional<T> read_number(std::istream& in, std::ostream& out, const std::string& prompt) {
    while (auto line = prompt_line(in, out, prompt)) {
        if (auto value = parse_number<T>(*line)) {
            return value;
        }
        out << "  '" << *line << "' is not a valid number, please try again.\n";
    }
    return std::nullopt;
}

/// Read a full member card; names may contain spaces, so they are read with std::getline.
inline std::optional<MemberCard> read_member(std::istream& in, std::ostream& out) {
    MemberCard card;
    auto name = prompt_line(in, out, "Full name: ");
    while (name && name->find_first_not_of(' ') == std::string::npos) {
        out << "  A name is required.\n";
        name = prompt_line(in, out, "Full name: ");
    }
    if (!name) {
        return std::nullopt;
    }
    card.name = *name;
    auto age = read_number<int>(in, out, "Age: ");
    auto height = read_number<double>(in, out, "Height in metres: ");
    auto tier = prompt_line(in, out, "Tier (B = basic, P = pro): ");
    auto newsletter = prompt_line(in, out, "Newsletter? (y/n): ");
    if (!age || !height || !tier || !newsletter) {
        return std::nullopt;
    }
    card.age = *age;
    card.height_m = *height;
    card.tier = (!tier->empty() && (tier->front() == 'P' || tier->front() == 'p')) ? 'P' : 'B';
    card.newsletter = !newsletter->empty() && (newsletter->front() == 'y' || newsletter->front() == 'Y');
    return card;
}

/// Render a card as aligned text.
inline std::string format_card(const MemberCard& card) {
    std::ostringstream out;
    out << std::left << std::setw(12) << "Name" << card.name << '\n'
        << std::setw(12) << "Age" << card.age << '\n'
        << std::setw(12) << "Height" << std::fixed << std::setprecision(2) << card.height_m << " m\n"
        << std::setw(12) << "Tier" << (card.tier == 'P' ? "Pro" : "Basic") << '\n'
        << std::setw(12) << "Newsletter" << (card.newsletter ? "yes" : "no") << '\n';
    return out.str();
}

/// The mini calculator: `a op b` for + - * /. Throws instead of returning a magic value.
inline double calculate(double a, char op, double b) {
    switch (op) {
        case '+': return a + b;
        case '-': return a - b;
        case '*': return a * b;
        case '/':
            if (b == 0.0) {
                throw std::domain_error("division by zero");
            }
            return a / b;
        default: throw std::invalid_argument(std::string("unknown operator '") + op + "'");
    }
}

/// The interactive demo: type tour, sign-up form, then the calculator until a blank line.
inline int run(std::istream& in, std::ostream& out) {
    out << "Day 01 – Variables, Types & Basic I/O\n\n";
    for (const auto& [declaration, value] : type_tour()) {
        out << "  " << std::left << std::setw(36) << declaration << "-> " << value << '\n';
    }

    out << "\nNew member sign-up\n";
    if (auto card = read_member(in, out)) {
        out << '\n' << format_card(*card);
    } else {
        out << "\nSign-up cancelled (end of input).\n";
        return 0;
    }

    out << "\nMini calculator – type 'a op b' (e.g. 120 / 4), blank line to finish\n";
    while (auto line = prompt_line(in, out, "calc> ")) {
        if (line->empty()) {
            break;
        }
        std::istringstream expression(*line);
        double a{};
        double b{};
        char op{};
        if (!(expression >> a >> op >> b)) {
            out << "  please type a number, an operator and a number\n";
            continue;
        }
        try {
            out << "  = " << calculate(a, op, b) << '\n';
        } catch (const std::exception& error) {
            out << "  error: " << error.what() << '\n';
        }
    }
    return 0;
}

}  // namespace cppm::day01
