/**
 * @file
 * A tiny, dependency-free unit-test framework for the 100-day course.
 *
 * Why not GoogleTest or Catch2? Every test in this repository must build and run offline,
 * on every operating system, with nothing but a C++20 compiler. This header gives the
 * handful of features the course needs and nothing more:
 *
 * @code
 * #include "cppm/testing.hpp"
 *
 * TEST_CASE("split_bill shares add up to the total") {
 *     auto shares = split_bill(10'000, 3);
 *     CHECK_EQ(shares.size(), 3u);
 *     CHECK_THROWS_AS(split_bill(100, 0), std::invalid_argument);
 * }
 * @endcode
 *
 * Every test executable links `cppm_test_main`, which provides `main()`:
 * `./test_day_05` runs everything, `./test_day_05 split` runs the tests whose name contains
 * "split", and `./test_day_05 --list` prints the test names.
 */
#pragma once

#include <cmath>
#include <concepts>
#include <cstddef>
#include <functional>
#include <ostream>
#include <sstream>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>
#include <vector>

namespace cppm::testing {

/// One registered test case.
struct TestCase {
    std::string name;
    std::function<void()> body;
    const char* file;
    int line;
};

/// The global list of test cases, filled by TEST_CASE before main() runs.
std::vector<TestCase>& registry();

/// Records the outcome of one check; failures are printed immediately.
void report_check(bool passed, std::string_view expression, std::string_view detail, const char* file, int line);

/// Thrown by REQUIRE to abort the current test case (never escapes the runner).
struct RequireFailed {};

/// Run the registered tests whose name contains @p filter; returns the process exit code.
int run_all(std::string_view filter, std::ostream& out);

/// Registers a test case at static-initialisation time.
struct Registrar {
    Registrar(std::string name, std::function<void()> body, const char* file, int line) {
        registry().push_back({std::move(name), std::move(body), file, line});
    }
};

template <typename T>
concept Streamable = requires(std::ostream& os, const T& value) { os << value; };

/// Best-effort printable form of a value, used in failure messages.
template <typename T>
std::string describe(const T& value) {
    if constexpr (std::is_same_v<T, bool>) {
        return value ? "true" : "false";
    } else if constexpr (std::is_same_v<T, std::nullptr_t>) {
        return "nullptr";
    } else if constexpr (std::is_enum_v<T>) {
        return std::to_string(static_cast<long long>(static_cast<std::underlying_type_t<T>>(value)));
    } else if constexpr (std::is_convertible_v<const T&, std::string_view>) {
        return "\"" + std::string(std::string_view(value)) + "\"";
    } else if constexpr (Streamable<T>) {
        std::ostringstream os;
        os << value;
        return os.str();
    } else {
        return "{unprintable}";
    }
}

/// Integer types that std::cmp_equal accepts (not bool and not the character types).
template <typename T>
concept StandardInteger =
    std::is_integral_v<T> && !std::is_same_v<T, bool> && !std::is_same_v<T, char> && !std::is_same_v<T, wchar_t> &&
    !std::is_same_v<T, char8_t> && !std::is_same_v<T, char16_t> && !std::is_same_v<T, char32_t>;

/// Equality that compares mixed signed/unsigned integers by value instead of by conversion.
template <typename A, typename B>
bool equal(const A& a, const B& b) {
    if constexpr (StandardInteger<A> && StandardInteger<B>) {
        return std::cmp_equal(a, b);
    } else {
        return a == b;
    }
}

template <typename A, typename B>
void check_eq(const A& a, const B& b, std::string_view expression, const char* file, int line, bool require) {
    const bool passed = equal(a, b);
    report_check(passed, expression, passed ? "" : describe(a) + " != " + describe(b), file, line);
    if (!passed && require) {
        throw RequireFailed{};
    }
}

inline void check_near(double a, double b, double tolerance, std::string_view expression, const char* file, int line) {
    const bool passed = std::fabs(a - b) <= tolerance;
    std::ostringstream detail;
    if (!passed) {
        detail.precision(12);
        detail << a << " differs from " << b << " by more than " << tolerance;
    }
    report_check(passed, expression, detail.str(), file, line);
}

}  // namespace cppm::testing

#define CPPM_CONCAT_INNER(a, b) a##b
#define CPPM_CONCAT(a, b) CPPM_CONCAT_INNER(a, b)

/// Define a test case: `TEST_CASE("what it proves") { ... }`.
#define TEST_CASE(name)                                                            \
    static void CPPM_CONCAT(cppm_test_fn_, __LINE__)();                            \
    static const ::cppm::testing::Registrar CPPM_CONCAT(cppm_test_reg_, __LINE__){ \
        name, &CPPM_CONCAT(cppm_test_fn_, __LINE__), __FILE__, __LINE__};          \
    static void CPPM_CONCAT(cppm_test_fn_, __LINE__)()

/// Record a failure but keep running the test.
#define CHECK(...) ::cppm::testing::report_check(static_cast<bool>(__VA_ARGS__), #__VA_ARGS__, "", __FILE__, __LINE__)

/// Stop the current test case if the condition is false.
#define REQUIRE(...)                                                                  \
    do {                                                                              \
        const bool cppm_ok = static_cast<bool>(__VA_ARGS__);                          \
        ::cppm::testing::report_check(cppm_ok, #__VA_ARGS__, "", __FILE__, __LINE__); \
        if (!cppm_ok) throw ::cppm::testing::RequireFailed{};                         \
    } while (false)

#define CHECK_EQ(a, b) ::cppm::testing::check_eq((a), (b), #a " == " #b, __FILE__, __LINE__, false)
#define REQUIRE_EQ(a, b) ::cppm::testing::check_eq((a), (b), #a " == " #b, __FILE__, __LINE__, true)
#define CHECK_NEAR(a, b, tolerance)                                                                                  \
    ::cppm::testing::check_near(static_cast<double>(a), static_cast<double>(b), (tolerance), #a " ~= " #b, __FILE__, \
                                __LINE__)

/// Passes only if evaluating the expression throws an exception of the given type.
#define CHECK_THROWS_AS(expression, exception_type)                                                            \
    do {                                                                                                       \
        bool cppm_thrown = false;                                                                              \
        try {                                                                                                  \
            static_cast<void>(expression);                                                                     \
        } catch (const exception_type&) {                                                                      \
            cppm_thrown = true;                                                                                \
        } catch (...) {                                                                                        \
        }                                                                                                      \
        ::cppm::testing::report_check(cppm_thrown, #expression " throws " #exception_type,                     \
                                      cppm_thrown ? "" : "a different exception or none was thrown", __FILE__, \
                                      __LINE__);                                                               \
    } while (false)

/// Passes only if evaluating the expression throws nothing.
#define CHECK_NOTHROW(expression)                                                                         \
    do {                                                                                                  \
        bool cppm_clean = true;                                                                           \
        try {                                                                                             \
            static_cast<void>(expression);                                                                \
        } catch (...) {                                                                                   \
            cppm_clean = false;                                                                           \
        }                                                                                                 \
        ::cppm::testing::report_check(cppm_clean, #expression " does not throw", "", __FILE__, __LINE__); \
    } while (false)
