/**
 * @file
 * Day 15 – While and Do-While Loops.
 *
 * Scenario: a *vending-machine controller*. It accepts coins until the price is covered
 * (or the customer types `cancel`), pays change with as few coins as possible, locks the
 * service panel after three wrong PINs, and shows its menu at least once per session.
 *
 * Deliverables (syllabus):
 * - Pre-test and post-test loops
 * - Sentinel loops
 * - Menu loops
 * - Termination on end of input
 */
#pragma once

#include <array>
#include <istream>
#include <optional>
#include <ostream>
#include <sstream>
#include <string>
#include <vector>

#include "cppm/lesson.hpp"

namespace cppm::day15 {

inline constexpr Deliverable DELIVERABLES[] = {
    {"pre-test while loop that may run zero times", "make_change"},
    {"while loop with an unknown number of steps", "collatz_steps"},
    {"sentinel loop that also stops at end of input", "collect_coins"},
    {"bounded retries with a while loop", "unlock_panel"},
    {"do-while menu shown at least once", "run"},
};

inline constexpr std::array<int, 6> coins_cents{200, 100, 50, 20, 10, 5};

/// Greedy change: while there is still money to return, use the largest coin that fits.
/// For a zero amount the loop body never runs – the defining property of a pre-test loop.
inline std::vector<int> make_change(int amount_cents) {
    std::vector<int> change;
    std::size_t coin = 0;
    while (amount_cents > 0 && coin < coins_cents.size()) {
        if (coins_cents[coin] <= amount_cents) {
            change.push_back(coins_cents[coin]);
            amount_cents -= coins_cents[coin];
        } else {
            ++coin;
        }
    }
    return change;
}

/// Steps until the Collatz sequence reaches 1: nobody knows the count in advance.
inline int collatz_steps(long long n) {
    if (n < 1) {
        return -1;
    }
    int steps = 0;
    while (n != 1) {
        n = (n % 2 == 0) ? n / 2 : 3 * n + 1;
        ++steps;
    }
    return steps;
}

/// True if @p cents is a coin the machine accepts.
inline bool accepted_coin(int cents) {
    for (int coin : coins_cents) {
        if (coin == cents) {
            return true;
        }
    }
    return false;
}

/// Read coins until @p price is covered. The sentinel "cancel" or end of input stops early
/// and returns std::nullopt (the inserted coins are refunded by the caller).
inline std::optional<int> collect_coins(std::istream& in, std::ostream& out, int price, int& inserted) {
    inserted = 0;
    while (inserted < price) {
        auto line = prompt_line(in, out, "coin (cents) or 'cancel': ");
        if (!line || *line == "cancel") {  // end of input is also a reason to stop
            return std::nullopt;
        }
        std::istringstream number(*line);
        int coin{};
        if (number >> coin && accepted_coin(coin)) {
            inserted += coin;
            out << "  credit " << inserted << " / " << price << '\n';
        } else {
            out << "  coin rejected\n";
        }
    }
    return inserted - price;
}

/// Ask for the service PIN at most @p max_attempts times; true when unlocked.
inline bool unlock_panel(std::istream& in, std::ostream& out, const std::string& pin, int max_attempts = 3) {
    int attempts = 0;
    while (attempts < max_attempts) {
        auto line = prompt_line(in, out, "service PIN: ");
        if (!line) {
            return false;
        }
        ++attempts;
        if (*line == pin) {
            return true;
        }
        out << "  wrong PIN (" << max_attempts - attempts << " left)\n";
    }
    out << "  panel locked\n";
    return false;
}

/// The interactive demo: a do-while menu that is always shown at least once.
inline int run(std::istream& in, std::ostream& out) {
    out << "Day 15 – While and Do-While Loops\n";
    std::optional<std::string> choice;
    do {
        out << "\n[1] buy a drink (1.30)  [2] service panel  [3] collatz  [q] quit\n";
        choice = prompt_line(in, out, "choice: ");
        if (!choice || *choice == "q") {
            break;
        }
        if (*choice == "1") {
            int inserted = 0;
            if (auto change = collect_coins(in, out, 130, inserted)) {
                out << "  enjoy your drink! change:";
                for (int coin : make_change(*change)) {
                    out << ' ' << coin;
                }
                out << '\n';
            } else {
                out << "  cancelled, refunding " << inserted << " cents\n";
            }
        } else if (*choice == "2") {
            out << (unlock_panel(in, out, "4711") ? "  panel unlocked\n" : "  access denied\n");
        } else if (*choice == "3") {
            auto line = prompt_line(in, out, "start number: ");
            long long n = 0;
            if (line && (std::istringstream(*line) >> n)) {
                out << "  " << collatz_steps(n) << " steps\n";
            }
        } else {
            out << "  unknown choice\n";
        }
    } while (true);
    out << "Goodbye.\n";
    return 0;
}

}  // namespace cppm::day15
