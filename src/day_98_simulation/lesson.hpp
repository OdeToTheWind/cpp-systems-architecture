/**
 * @file
 * Day 98 – Capstone: Scientific Simulation.
 *
 * Scenario: *flu-season planning* for a city's hospitals. A compartmental SIR model (susceptible,
 * infected, recovered) is integrated with Runge-Kutta 4 to find when infections peak and how many
 * beds are needed; the numerical method is checked against an analytic result and a crude Euler
 * integration. A stochastic Monte Carlo version estimates how likely a large outbreak is when it
 * starts from a handful of cases – reproducibly, because every run is seeded.
 *
 * Deliverables (syllabus):
 * - Differential equations with RK4
 * - Checking a numerical method
 * - Monte Carlo simulation
 * - Reproducible seeds
 */
#pragma once

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <istream>
#include <ostream>
#include <random>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

#include "cppm/lesson.hpp"

namespace cppm::day98 {

inline constexpr Deliverable DELIVERABLES[] = {
    {"the SIR equations", "derivative"},
    {"one fourth-order Runge-Kutta step", "rk4_step"},
    {"the analytic final epidemic size used as a check", "final_size_fraction"},
    {"a seeded chain-binomial outbreak", "reed_frost"},
    {"Monte Carlo estimates with a confidence interval", "monte_carlo"},
};

struct State {
    double s;  // susceptible
    double i;  // infected
    double r;  // recovered
};

inline double total(const State& x) {
    return x.s + x.i + x.r;
}

/// beta: infections per infected person per day in a fully susceptible population; gamma: 1 / infectious period.
/// dS/dt = -beta S I / N, dI/dt = beta S I / N - gamma I, dR/dt = gamma I.
inline State derivative(const State& x, double beta, double gamma) {
    const double infections = beta * x.s * x.i / total(x);
    const double recoveries = gamma * x.i;
    return {-infections, infections - recoveries, recoveries};
}

inline State add(const State& a, const State& b, double k) {
    return {a.s + k * b.s, a.i + k * b.i, a.r + k * b.r};
}

inline State euler_step(const State& x, double beta, double gamma, double dt) {
    return add(x, derivative(x, beta, gamma), dt);
}

/// Classic RK4: four slope evaluations per step; error shrinks like dt^4 instead of Euler's dt.
inline State rk4_step(const State& x, double beta, double gamma, double dt) {
    const State k1 = derivative(x, beta, gamma);
    const State k2 = derivative(add(x, k1, dt / 2), beta, gamma);
    const State k3 = derivative(add(x, k2, dt / 2), beta, gamma);
    const State k4 = derivative(add(x, k3, dt), beta, gamma);
    return {x.s + dt / 6 * (k1.s + 2 * k2.s + 2 * k3.s + k4.s), x.i + dt / 6 * (k1.i + 2 * k2.i + 2 * k3.i + k4.i),
            x.r + dt / 6 * (k1.r + 2 * k2.r + 2 * k3.r + k4.r)};
}

enum class Method { euler, rk4 };

/// Integrate for @p days with step @p dt; returns the state at the end of every day.
inline std::vector<State> simulate(State start, double beta, double gamma, int days, double dt,
                                   Method method = Method::rk4) {
    if (dt <= 0 || dt > 1) throw std::invalid_argument("dt must be in (0, 1]");
    const int steps_per_day = static_cast<int>(std::lround(1.0 / dt));
    if (std::abs(steps_per_day * dt - 1.0) > 1e-9) throw std::invalid_argument("1 / dt must be a whole number");
    std::vector<State> daily{start};
    State x = start;
    for (int d = 0; d < days; ++d) {
        for (int k = 0; k < steps_per_day; ++k)
            x = method == Method::rk4 ? rk4_step(x, beta, gamma, dt) : euler_step(x, beta, gamma, dt);
        daily.push_back(x);
    }
    return daily;
}

struct Peak {
    int day;
    double infected;
};

inline Peak peak(const std::vector<State>& series) {
    Peak p{0, series.front().i};
    for (std::size_t d = 1; d < series.size(); ++d) {
        if (series[d].i > p.infected) p = {static_cast<int>(d), series[d].i};
    }
    return p;
}

/// For R0 = beta / gamma > 1 and a tiny initial outbreak, the fraction z eventually infected solves
/// z = 1 - exp(-R0 z). Solved by bisection – an independent check on the integrator.
inline double final_size_fraction(double r0) {
    if (r0 <= 1) return 0;
    double lo = 1e-9;
    double hi = 1;
    for (int it = 0; it < 200; ++it) {
        const double mid = (lo + hi) / 2;
        (mid - (1 - std::exp(-r0 * mid)) < 0 ? lo : hi) = mid;
    }
    return (lo + hi) / 2;
}

/// Uniform in [0, 1) from the engine's raw bits. std::mt19937_64 produces the same numbers on every
/// platform, but std::uniform_real_distribution may not – so seeded results are portable only this way.
inline double uniform01(std::mt19937_64& rng) {
    return static_cast<double>(rng() >> 11) * (1.0 / 9007199254740992.0);
}

struct Outbreak {
    int total_infected;
    int generations;
};

/// Reed-Frost chain binomial: in each generation every susceptible escapes each of the I infected
/// independently with probability (1 - p).
inline Outbreak reed_frost(int population, int initial, double p, std::mt19937_64& rng) {
    int s = population - initial;
    int i = initial;
    Outbreak o{initial, 0};
    while (i > 0) {
        const double infection_chance = 1 - std::pow(1 - p, i);
        int new_cases = 0;
        for (int k = 0; k < s; ++k) new_cases += uniform01(rng) < infection_chance ? 1 : 0;
        s -= new_cases;
        i = new_cases;
        o.total_infected += new_cases;
        ++o.generations;
    }
    return o;
}

struct MonteCarlo {
    int runs;
    double major_outbreak_probability;
    double mean_attack_rate;
    double ci95_low;
    double ci95_high;
};

/// @p runs independent outbreaks from one seed. A "major" outbreak infects more than 10%.
inline MonteCarlo monte_carlo(int runs, int population, int initial, double p, std::uint64_t seed) {
    if (runs <= 0) throw std::invalid_argument("need at least one run");
    std::mt19937_64 rng(seed);
    int major = 0;
    double attack_sum = 0;
    for (int k = 0; k < runs; ++k) {
        const auto o = reed_frost(population, initial, p, rng);
        const double attack = static_cast<double>(o.total_infected) / population;
        attack_sum += attack;
        major += attack > 0.1 ? 1 : 0;
    }
    const double prob = static_cast<double>(major) / runs;
    const double half = 1.96 * std::sqrt(prob * (1 - prob) / runs);  // normal approximation
    return {runs, prob, attack_sum / runs, std::max(0.0, prob - half), std::min(1.0, prob + half)};
}

/// The interactive demo: "<population> <R0> <infectious_days> <initial_cases>".
inline int run(std::istream& in, std::ostream& out) {
    out << "Day 98 – Capstone: Scientific Simulation\n";
    while (auto line = prompt_line(in, out, "population R0 infectious_days initial> ")) {
        std::istringstream words(*line);
        int population = 0;
        double r0 = 0;
        double infectious_days = 0;
        int initial = 0;
        if (!(words >> population >> r0 >> infectious_days >> initial) || population <= initial || initial <= 0 ||
            infectious_days <= 0)
            break;
        const double gamma = 1 / infectious_days;
        const auto series = simulate({double(population - initial), double(initial), 0}, r0 * gamma, gamma, 365, 0.1);
        const auto p = peak(series);
        char text[200];
        std::snprintf(text, sizeof text,
                      "  peak on day %d with %.0f infected; %.1f%% infected in total (theory %.1f%%)\n", p.day,
                      p.infected, 100 * series.back().r / population, 100 * final_size_fraction(r0));
        out << text;
        const auto mc = monte_carlo(400, 300, 1, 1 - std::exp(-r0 / 300), 98);  // one imported case
        std::snprintf(text, sizeof text,
                      "  town of 300, one imported case: P(major outbreak) = %.2f (95%% CI %.2f-%.2f)\n",
                      mc.major_outbreak_probability, mc.ci95_low, mc.ci95_high);
        out << text;
    }
    return 0;
}

}  // namespace cppm::day98
