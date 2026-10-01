// Tests for Day 15 – While and Do-While Loops.
#include <numeric>
#include <sstream>

#include "cppm/testing.hpp"
#include "day_15_while_loops/lesson.hpp"

using namespace cppm::day15;

TEST_CASE("make_change uses the fewest coins and runs zero times for zero") {
    CHECK(make_change(0).empty());
    CHECK(make_change(370) == std::vector<int>{200, 100, 50, 20});
    CHECK(make_change(15) == std::vector<int>{10, 5});
    for (int amount = 0; amount <= 500; amount += 5) {
        const auto change = make_change(amount);
        CHECK_EQ(std::accumulate(change.begin(), change.end(), 0), amount);
    }
}

TEST_CASE("make_change stops even when the amount cannot be paid exactly") {
    const auto change = make_change(3);
    CHECK(change.empty());
}

TEST_CASE("collatz_steps counts an unknown number of iterations") {
    CHECK_EQ(collatz_steps(1), 0);
    CHECK_EQ(collatz_steps(6), 8);
    CHECK_EQ(collatz_steps(27), 111);
    CHECK_EQ(collatz_steps(0), -1);
}

TEST_CASE("collect_coins stops once the price is covered and returns the change") {
    std::istringstream in("100\n3\n50\n");
    std::ostringstream out;
    int inserted = 0;
    CHECK_EQ(collect_coins(in, out, 130, inserted).value_or(-1), 20);
    CHECK_EQ(inserted, 150);
    CHECK(out.str().find("coin rejected") != std::string::npos);
}

TEST_CASE("collect_coins honours the sentinel and end of input") {
    std::istringstream cancel("50\ncancel\n");
    std::ostringstream out;
    int inserted = 0;
    CHECK(!collect_coins(cancel, out, 130, inserted).has_value());
    CHECK_EQ(inserted, 50);
    std::istringstream eof("20\n");
    CHECK(!collect_coins(eof, out, 130, inserted).has_value());
}

TEST_CASE("unlock_panel allows three attempts then locks") {
    std::istringstream good("1111\n4711\n");
    std::ostringstream out;
    CHECK(unlock_panel(good, out, "4711"));
    std::istringstream bad("1\n2\n3\n4711\n");
    std::ostringstream out2;
    CHECK(!unlock_panel(bad, out2, "4711"));
    CHECK(out2.str().find("panel locked") != std::string::npos);
    std::istringstream empty("");
    CHECK(!unlock_panel(empty, out2, "4711"));
}

TEST_CASE("run shows the menu at least once and handles every option") {
    std::istringstream in("1\n200\n2\n4711\n3\n6\nx\nq\n");
    std::ostringstream out;
    CHECK_EQ(run(in, out), 0);
    const auto text = out.str();
    CHECK(text.find("change: 50 20") != std::string::npos);
    CHECK(text.find("panel unlocked") != std::string::npos);
    CHECK(text.find("8 steps") != std::string::npos);
    CHECK(text.find("unknown choice") != std::string::npos);
    CHECK(text.find("Goodbye.") != std::string::npos);
}

TEST_CASE("run terminates at end of input even mid-purchase") {
    std::istringstream in("1\n100\n");
    std::ostringstream out;
    CHECK_EQ(run(in, out), 0);
    CHECK(out.str().find("refunding 100 cents") != std::string::npos);
    std::istringstream nothing("");
    std::ostringstream out2;
    run(nothing, out2);
    CHECK(out2.str().find("[1] buy a drink") != std::string::npos);
}
