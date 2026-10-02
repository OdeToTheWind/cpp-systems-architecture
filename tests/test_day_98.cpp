// Tests for Day 98 – Capstone: Scientific Simulation. Numerical results are checked against
// invariants, theory and convergence rates rather than copied from a run.
#include <cmath>
#include <random>
#include <sstream>
#include <stdexcept>
#include <string>

#include "cppm/testing.hpp"
#include "day_98_simulation/lesson.hpp"

using namespace cppm::day98;

namespace {
const State city{99'990, 10, 0};
constexpr double gamma_ = 1.0 / 5;
constexpr double beta_ = 2.5 * gamma_;  // R0 = 2.5
}  // namespace

TEST_CASE("the derivative conserves the population") {
    const auto d = derivative(city, beta_, gamma_);
    CHECK_NEAR(d.s + d.i + d.r, 0.0, 1e-9);
    CHECK(d.s < 0 && d.r > 0);
    const auto later = simulate(city, beta_, gamma_, 200, 0.1);
    for (const auto& x : later) CHECK_NEAR(total(x), 100'000.0, 1e-6);
}

TEST_CASE("an epidemic rises, peaks and ends near the theoretical final size") {
    const auto series = simulate(city, beta_, gamma_, 400, 0.1);
    const auto p = peak(series);
    CHECK(p.day > 30 && p.day < 90);
    CHECK(series.back().i < 1.0);
    CHECK_NEAR(series.back().r / 100'000.0, final_size_fraction(2.5), 0.002);
    CHECK_NEAR(final_size_fraction(2.5), 0.8926, 1e-3);
    CHECK_EQ(final_size_fraction(0.9), 0.0);
}

TEST_CASE("RK4 converges much faster than Euler") {
    const auto reference = simulate(city, beta_, gamma_, 60, 0.01).back().i;
    const auto euler_err = std::abs(simulate(city, beta_, gamma_, 60, 0.5, Method::euler).back().i - reference);
    const auto rk4_err = std::abs(simulate(city, beta_, gamma_, 60, 0.5, Method::rk4).back().i - reference);
    CHECK(rk4_err * 100 < euler_err);
    const auto rk4_half = std::abs(simulate(city, beta_, gamma_, 60, 0.25).back().i - reference);
    CHECK(rk4_err / rk4_half > 10);  // halving dt cuts the error ~16x for a 4th-order method
    CHECK_THROWS_AS(simulate(city, beta_, gamma_, 1, 0.3), std::invalid_argument);
}

TEST_CASE("seeded randomness is reproducible and portable") {
    std::mt19937_64 a(42);
    std::mt19937_64 b(42);
    for (int k = 0; k < 1000; ++k) CHECK(uniform01(a) == uniform01(b));
    std::mt19937_64 c(42);
    CHECK_EQ(c(), 13930160852258120406ULL);  // the standard fixes mt19937_64's output sequence
    for (int k = 0; k < 1000; ++k) {
        const double u = uniform01(c);
        CHECK(u >= 0.0 && u < 1.0);
    }
}

TEST_CASE("Reed-Frost outbreaks stay within the population") {
    std::mt19937_64 rng(7);
    for (int k = 0; k < 50; ++k) {
        const auto o = reed_frost(200, 3, 0.01, rng);
        CHECK(o.total_infected >= 3 && o.total_infected <= 200);
    }
    std::mt19937_64 none(1);
    CHECK_EQ(reed_frost(100, 2, 0.0, none).total_infected, 2);
    std::mt19937_64 all(1);
    CHECK_EQ(reed_frost(100, 2, 1.0, all).total_infected, 100);
}

TEST_CASE("Monte Carlo estimates agree with theory and repeat exactly for a seed") {
    const double r0 = 2.0;
    const auto mc = monte_carlo(2000, 500, 1, 1 - std::exp(-r0 / 500), 98);
    // Branching-process theory: with Poisson-like offspring (Reed-Frost), the chance that one case
    // starts a major outbreak solves the same equation as the final size: z = 1 - exp(-R0 z), ~0.80.
    const double theory = final_size_fraction(r0);
    CHECK(mc.ci95_low < theory + 0.03 && mc.ci95_high > theory - 0.03);
    CHECK(mc.mean_attack_rate > 0.5 && mc.mean_attack_rate < theory);
    const auto again = monte_carlo(2000, 500, 1, 1 - std::exp(-r0 / 500), 98);
    CHECK(mc.major_outbreak_probability == again.major_outbreak_probability);
    CHECK(mc.mean_attack_rate == again.mean_attack_rate);
    CHECK_THROWS_AS(monte_carlo(0, 10, 1, 0.1, 1), std::invalid_argument);
}

TEST_CASE("run reports the peak and the outbreak probability") {
    std::istringstream in("100000 2.5 5 10\nstop\n");
    std::ostringstream out;
    CHECK_EQ(run(in, out), 0);
    CHECK(out.str().find("infected in total (theory 89.3%)") != std::string::npos);
    CHECK(out.str().find("P(major outbreak) = ") != std::string::npos);
}
