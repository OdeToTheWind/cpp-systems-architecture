# Day 98 – Scientific Simulation Reflection

**Date:** 2026-06-19 · **Level:** Capstone · **C++:** 20 · **Status:** Covered
**Code:** [`src/day_98_simulation/lesson.hpp`](../../src/day_98_simulation/lesson.hpp) · **Tests:** [`tests/test_day_98.cpp`](../../tests/test_day_98.cpp) (7 tests)

## Scenario
*flu-season planning* for a city's hospitals. A compartmental SIR model (susceptible, infected, recovered) is integrated with Runge-Kutta 4 to find when infections peak and how many beds are needed; the numerical method is checked against an analytic result and a crude Euler integration. A stochastic Monte Carlo version estimates how likely a large outbreak is when it starts from a handful of cases – reproducibly, because every run is seeded.

## Syllabus deliverables
> SIR epidemic model with RK4 integration, stochastic Monte Carlo, reproducible seeds

| Deliverable | Implemented in |
|---|---|
| ✅ the SIR equations | `derivative` |
| ✅ one fourth-order Runge-Kutta step | `rk4_step` |
| ✅ the analytic final epidemic size used as a check | `final_size_fraction` |
| ✅ a seeded chain-binomial outbreak | `reed_frost` |
| ✅ Monte Carlo estimates with a confidence interval | `monte_carlo` |

## Key learnings
- The SIR model is three coupled differential equations; RK4 integrates them accurately with modest step sizes.
- A numerical result is trusted when it passes independent checks: conservation of the population, the analytic final-size equation, and the expected convergence rate.
- Monte Carlo turns a random process into estimates with confidence intervals; more runs narrow the interval like 1 / sqrt(n).
- std::mt19937_64 gives the same sequence everywhere, but standard distributions do not; converting raw bits to [0, 1) by hand keeps seeded results portable.

## Pitfalls I hit (and how I fixed them)
- Euler's method with half-day steps was off by thousands of cases; RK4 at the same step was over 100x more accurate.
- The first test expected P(major outbreak) = 1 - 1/R0, which holds for exponential infectious periods; Reed-Frost generations follow z = 1 - exp(-R0 z) instead.
- Starting the Monte Carlo from ten cases made a major outbreak certain and the estimate useless; it now starts from one imported case.

## Run it
```bash
./procpp.sh 98                          # study mode: explanation, code map, notes, tests and quiz
./build/bin/day_98_simulation       # the interactive demo
ctest --test-dir build -R test_day_98 --output-on-failure
```

## Next step
- Day 99 adds observability for bugs that only happen in production.
