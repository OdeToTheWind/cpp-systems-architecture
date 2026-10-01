/**
 * @file
 * Day 31 – Member Functions.
 *
 * Scenario: a *coffee-roastery batch tracker*. Each roast batch has methods that change it
 * (and return `*this` so they chain), const methods that only read it, and static members
 * that belong to the roastery as a whole: the batch-number counter and the list of beans it
 * buys.
 *
 * Deliverables (syllabus):
 * - Const member functions
 * - Static member functions and static data
 * - this
 * - Method chaining
 */
#pragma once

#include <algorithm>
#include <array>
#include <istream>
#include <optional>
#include <ostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

#include "cppm/lesson.hpp"

namespace cppm::day31 {

inline constexpr Deliverable DELIVERABLES[] = {
    {"static data shared by every object", "RoastBatch::next_number_"},
    {"a static member function that needs no object", "RoastBatch::is_known_bean"},
    {"a static factory function", "RoastBatch::parse"},
    {"chaining: mutators return *this", "RoastBatch::add_beans"},
    {"const member functions that only read", "RoastBatch::weight_loss_percent"},
    {"using this to compare with another object", "RoastBatch::same_origin_as"},
};

enum class Roast { light, medium, dark };

class RoastBatch {
public:
    /// Static member function: a question about the roastery, not about one batch.
    static bool is_known_bean(std::string_view origin) {
        return std::find(known_beans.begin(), known_beans.end(), origin) != known_beans.end();
    }

    /// Static factory: builds a batch from "origin grams roast", or std::nullopt.
    static std::optional<RoastBatch> parse(std::string_view line) {
        std::istringstream words{std::string(line)};
        std::string origin;
        int grams = 0;
        std::string roast;
        if (!(words >> origin >> grams >> roast) || !is_known_bean(origin) || grams <= 0) {
            return std::nullopt;
        }
        RoastBatch batch(origin);
        batch.add_beans(grams).set_roast(roast == "light" ? Roast::light : roast == "dark" ? Roast::dark : Roast::medium);
        return batch;
    }

    static int batches_created() { return next_number_ - 1; }

    explicit RoastBatch(std::string origin) : number_(next_number_++), origin_(std::move(origin)) {
        if (!is_known_bean(origin_)) {
            throw std::invalid_argument("unknown bean origin");
        }
    }

    // ── mutators: change the batch and return *this so calls can be chained ──
    RoastBatch& add_beans(int grams) {
        if (grams <= 0) {
            throw std::invalid_argument("grams must be positive");
        }
        green_grams_ += grams;
        return *this;
    }
    RoastBatch& set_roast(Roast roast) {
        roast_ = roast;
        return *this;
    }
    RoastBatch& record_output(int roasted_grams) {
        if (roasted_grams <= 0 || roasted_grams > green_grams_) {
            throw std::invalid_argument("roasted weight must be between 1 g and the green weight");
        }
        roasted_grams_ = roasted_grams;
        return *this;
    }

    // ── const member functions: callable on const batches, cannot modify members ──
    int number() const { return number_; }
    const std::string& origin() const { return origin_; }
    int green_grams() const { return green_grams_; }
    Roast roast() const { return roast_; }
    double weight_loss_percent() const {
        if (roasted_grams_ == 0) {
            return 0.0;
        }
        return 100.0 * (green_grams_ - roasted_grams_) / green_grams_;
    }
    /// `this` points at the object the function was called on.
    bool same_origin_as(const RoastBatch& other) const { return this != &other && origin_ == other.origin_; }

private:
    static constexpr std::array<std::string_view, 4> known_beans{"ethiopia", "colombia", "brazil", "kenya"};
    static inline int next_number_ = 1;  // C++17 inline static: one counter for the whole program

    int number_;
    std::string origin_;
    int green_grams_{0};
    int roasted_grams_{0};
    Roast roast_{Roast::medium};
};

/// The interactive demo: "origin grams roast [roasted]" lines.
inline int run(std::istream& in, std::ostream& out) {
    out << "Day 31 – Member Functions\nBeans: ethiopia colombia brazil kenya\n";
    std::vector<RoastBatch> batches;
    while (auto line = prompt_line(in, out, "origin grams light|medium|dark [roasted]> ")) {
        if (line->empty()) {
            break;
        }
        auto batch = RoastBatch::parse(*line);
        if (!batch) {
            out << "  could not parse that batch\n";
            continue;
        }
        std::istringstream words(*line);
        std::string skip;
        int roasted = 0;
        words >> skip >> skip >> skip >> roasted;
        try {
            if (roasted > 0) {
                batch->record_output(roasted);
            }
            out << "  batch #" << batch->number() << " " << batch->origin() << ", loss " << batch->weight_loss_percent()
                << "%\n";
            for (const auto& earlier : batches) {
                if (batch->same_origin_as(earlier)) {
                    out << "  same origin as batch #" << earlier.number() << '\n';
                }
            }
            batches.push_back(*batch);
        } catch (const std::invalid_argument& error) {
            out << "  " << error.what() << '\n';
        }
    }
    out << RoastBatch::batches_created() << " batch number(s) issued so far\n";
    return 0;
}

}  // namespace cppm::day31
