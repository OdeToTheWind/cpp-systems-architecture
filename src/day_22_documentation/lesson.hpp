/**
 * @file
 * Day 22 – Documentation vs Comments.
 *
 * Scenario: a *kitchen unit-conversion library* that is documented the professional way –
 * Doxygen comments describe what each function promises, ordinary comments explain why –
 * plus a small documentation auditor that checks a header for undocumented functions.
 *
 * Deliverables (syllabus):
 * - Doxygen comments
 * - Why-comments vs what-comments
 * - Documenting preconditions and errors
 */
#pragma once

#include <cmath>
#include <cstddef>
#include <istream>
#include <optional>
#include <ostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

#include "cppm/lesson.hpp"

namespace cppm::day22 {

inline constexpr Deliverable DELIVERABLES[] = {
    {"a fully documented function: @brief, @param, @return, @throws, @pre", "convert"},
    {"a why-comment explaining a non-obvious choice", "cups_to_millilitres"},
    {"documenting a precondition that is checked", "scale_recipe"},
    {"a documentation auditor that finds undocumented functions", "find_undocumented"},
    {"spotting comments that only repeat the code", "is_what_comment"},
};

/// Kitchen units supported by convert().
enum class Unit { gram, kilogram, ounce, pound, millilitre, litre, cup, tablespoon, teaspoon };

/**
 * @brief Convert a quantity between two kitchen units.
 *
 * Mass converts to mass and volume to volume; mixing them needs a density, which this
 * library deliberately does not guess.
 *
 * @param amount  The quantity to convert; must be finite and non-negative.
 * @param from    The unit @p amount is expressed in.
 * @param to      The unit wanted.
 * @return        The quantity in @p to units.
 * @throws std::invalid_argument if @p amount is negative or not finite.
 * @throws std::domain_error if one unit measures mass and the other volume.
 * @pre   `amount >= 0`
 */
double convert(double amount, Unit from, Unit to);

/**
 * @brief Volume of @p cups US cups in millilitres.
 * @param cups Number of cups (may be fractional).
 * @return Millilitres.
 */
inline double cups_to_millilitres(double cups) {
    // Why 236.588 and not 250? US recipes use the US customary cup; the 250 ml "metric cup"
    // would make American baking recipes about 6 % too wet.
    return cups * 236.588;
}

/**
 * @brief Scale every ingredient amount by @p factor.
 * @param amounts Ingredient amounts, in any units.
 * @param factor  Scale factor; must be > 0 (halving is 0.5).
 * @return A new vector; @p amounts is not modified.
 * @throws std::invalid_argument if @p factor <= 0.
 */
std::vector<double> scale_recipe(const std::vector<double>& amounts, double factor);

/**
 * @brief Names of functions in @p header_text that have no doc comment directly above them.
 * @param header_text The full text of a C++ header.
 * @return Function names in order of appearance.
 */
std::vector<std::string> find_undocumented(std::string_view header_text);

/**
 * @brief True if @p comment merely restates @p code (for example `i++; // increment i`).
 * @param code    The statement the comment sits next to.
 * @param comment The comment text without the leading `//`.
 */
bool is_what_comment(std::string_view code, std::string_view comment);

/// Parse a unit name such as "cup" or "g"; std::nullopt for unknown names.
std::optional<Unit> parse_unit(std::string_view name);

/// The interactive demo: "amount from to" conversions.
int run(std::istream& in, std::ostream& out);

}  // namespace cppm::day22
