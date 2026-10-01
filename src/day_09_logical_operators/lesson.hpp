/**
 * @file
 * Day 09 – Logical Operators.
 *
 * Scenario: a *data-centre door controller* that decides whether a badge may open a door,
 * explains every refusal, and proves with an evaluation trace exactly when C++ stops
 * evaluating a condition.
 *
 * Deliverables (syllabus):
 * - &&, || and !
 * - Short-circuit evaluation
 * - De Morgan's laws
 * - Named boolean conditions
 */
#pragma once

#include <array>
#include <istream>
#include <ostream>
#include <sstream>
#include <string>
#include <string_view>
#include <vector>

#include "cppm/lesson.hpp"

namespace cppm::day09 {

inline constexpr Deliverable DELIVERABLES[] = {
    {"named boolean conditions combined with && and ||", "can_enter"},
    {"! to list what is missing", "denial_reasons"},
    {"short-circuit guards a null pointer", "badge_is_active"},
    {"tracing which operands are evaluated", "EvaluationTrace"},
    {"De Morgan's laws checked exhaustively", "de_morgan_holds"},
};

struct Badge {
    std::string holder;
    int clearance{0};  // 1 = visitor … 4 = administrator
    bool active{true};
    int expires_on_day{365};
};

struct Door {
    std::string name;
    int required_clearance{1};
    bool open_after_hours{false};
};

struct Context {
    int day{1};
    int hour{9};
    bool lockdown{false};
    bool escorted{false};  // an administrator walks the visitor in
};

/// `b != nullptr && b->active`: the right side runs only when the left is true, so no null dereference.
inline bool badge_is_active(const Badge* badge) { return badge != nullptr && badge->active; }

/// Each rule gets a name; the final expression then reads like the security policy.
inline bool can_enter(const Badge* badge, const Door& door, const Context& ctx) {
    const bool valid_badge = badge_is_active(badge) && ctx.day <= badge->expires_on_day;
    const bool office_hours = ctx.hour >= 7 && ctx.hour < 19;
    const bool cleared = valid_badge && (badge->clearance >= door.required_clearance || ctx.escorted);
    const bool time_ok = office_hours || door.open_after_hours;
    return !ctx.lockdown && cleared && time_ok;
}

/// Every unmet condition, phrased with ! so the caller learns all reasons at once.
inline std::vector<std::string> denial_reasons(const Badge* badge, const Door& door, const Context& ctx) {
    std::vector<std::string> reasons;
    if (ctx.lockdown) {
        reasons.emplace_back("building is in lockdown");
    }
    if (!badge_is_active(badge)) {
        reasons.emplace_back("no active badge");
    } else {
        if (!(ctx.day <= badge->expires_on_day)) {
            reasons.emplace_back("badge expired");
        }
        if (!(badge->clearance >= door.required_clearance || ctx.escorted)) {
            reasons.emplace_back("clearance too low and no escort");
        }
    }
    if (!(ctx.hour >= 7 && ctx.hour < 19) && !door.open_after_hours) {
        reasons.emplace_back("door closed after hours");
    }
    return reasons;
}

/// Records which named operands were actually evaluated.
class EvaluationTrace {
public:
    /// Returns @p value unchanged and remembers that this operand ran.
    bool check(std::string_view name, bool value) {
        evaluated_.emplace_back(name);
        return value;
    }
    const std::vector<std::string>& evaluated() const { return evaluated_; }
    std::string joined() const {
        std::string text;
        for (const auto& name : evaluated_) {
            text += (text.empty() ? "" : ",") + name;
        }
        return text;
    }

private:
    std::vector<std::string> evaluated_;
};

/// !(a && b) == (!a || !b) and !(a || b) == (!a && !b) for every combination of a and b.
inline bool de_morgan_holds() {
    for (bool a : {false, true}) {
        for (bool b : {false, true}) {
            const bool not_both = !(a && b);
            const bool either_missing = !a || !b;
            const bool neither = !(a || b);
            const bool both_missing = !a && !b;
            if (not_both != either_missing || neither != both_missing) {
                return false;
            }
        }
    }
    return true;
}

/// Rows "a b | a&&b a||b !a" for printing.
inline std::array<std::string, 4> truth_table() {
    std::array<std::string, 4> rows;
    std::size_t i = 0;
    for (bool a : {false, true}) {
        for (bool b : {false, true}) {
            rows[i++] = std::string(a ? "T" : "F") + ' ' + (b ? "T" : "F") + " | " + ((a && b) ? "T" : "F") + ' ' +
                        ((a || b) ? "T" : "F") + ' ' + (!a ? "T" : "F");
        }
    }
    return rows;
}

/// The interactive demo: "clearance hour lockdown(0/1) escorted(0/1)" against the server room door.
inline int run(std::istream& in, std::ostream& out) {
    out << "Day 09 – Logical Operators\n a b | && || !a\n";
    for (const auto& row : truth_table()) {
        out << ' ' << row << '\n';
    }
    out << "De Morgan's laws hold: " << (de_morgan_holds() ? "yes" : "no") << '\n';
    EvaluationTrace trace;
    const bool result = trace.check("lockdown_clear", false) && trace.check("badge_valid", true);
    out << "Short-circuit demo: result " << result << ", evaluated [" << trace.joined() << "]\n";

    const Door server_room{"server room", 3, false};
    while (auto line = prompt_line(in, out, "clearance hour lockdown escorted> ")) {
        if (line->empty()) {
            break;
        }
        std::istringstream values(*line);
        Badge badge{"visitor", 0, true, 365};
        Context ctx;
        if (!(values >> badge.clearance >> ctx.hour >> ctx.lockdown >> ctx.escorted)) {
            out << "  please type four numbers, e.g. 3 10 0 0\n";
            continue;
        }
        if (can_enter(&badge, server_room, ctx)) {
            out << "  door opens\n";
        } else {
            for (const auto& reason : denial_reasons(&badge, server_room, ctx)) {
                out << "  denied: " << reason << '\n';
            }
        }
    }
    return 0;
}

}  // namespace cppm::day09
