// Tests for Day 97 – Capstone: Automation Bot. A fake shop, recorded notifications and a fake clock.
#include <chrono>
#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include "cppm/testing.hpp"
#include "day_97_automation_bot/lesson.hpp"

using namespace cppm::day97;
using namespace std::chrono_literals;

namespace {
struct World {
    FakeShop shop;
    std::vector<std::pair<std::string, std::string>> sent;
    std::vector<std::chrono::milliseconds> sleeps;
    PriceBot bot{[this](const std::string& p) { return shop.fetch(p); },
                 [this](const std::string& u, const std::string& m) { sent.emplace_back(u, m); },
                 [this](std::chrono::milliseconds d) { sleeps.push_back(d); }, 30min};
    World() { bot.sync_wishlist(); }
    void run_minutes(int from, int to) {
        for (int m = from; m < to; ++m) bot.tick(std::chrono::minutes(m));
    }
};
}  // namespace

TEST_CASE("prices are scraped in several notations") {
    CHECK_EQ(extract_price("<span class=\"price\">€12.99</span>").value_or(-1), 1299);
    CHECK_EQ(extract_price("<span class=\"price\"> &euro;1,299.00 </span>").value_or(-1), 129900);
    CHECK_EQ(extract_price("<span class=\"price\">EUR 5.00</span>").value_or(-1), 500);
    CHECK(!extract_price("<span class=\"price\">sold out</span>").has_value());
    CHECK(!extract_price("<p>no price here</p>").has_value());
}

TEST_CASE("the wishlist API response is parsed") {
    const auto subs = parse_wishlist(FakeShop{}.wishlist);
    CHECK_EQ(subs.size(), 3u);
    CHECK_EQ(subs[1].user, "ben");
    CHECK_EQ(subs[1].target_cents, 9900);
    CHECK(parse_wishlist("[]").empty());
    CHECK_THROWS_AS(parse_wishlist(R"([{"user":"x"}])"), std::runtime_error);
}

TEST_CASE("transient failures are retried with doubling delays") {
    int calls = 0;
    std::vector<std::chrono::milliseconds> sleeps;
    const auto result = with_retries([&] {
        if (++calls < 3) throw TransientError("503");
        return 42;
    }, 3, 100ms, [&](auto d) { sleeps.push_back(d); });
    CHECK_EQ(result, 42);
    CHECK(sleeps == std::vector<std::chrono::milliseconds>{100ms, 200ms});
    CHECK_THROWS_AS(with_retries([]() -> int { throw TransientError("down"); }, 2, 1ms, [](auto) {}), TransientError);
    int permanent = 0;
    CHECK_THROWS_AS(with_retries([&]() -> int { ++permanent; throw std::runtime_error("404"); }, 5, 1ms, [](auto) {}), std::runtime_error);
    CHECK_EQ(permanent, 1);  // permanent errors are not retried
}

TEST_CASE("the deduplicator remembers user, product and price") {
    Deduplicator d;
    CHECK(d.first_time("ana", "/p/1", 100));
    CHECK(!d.first_time("ana", "/p/1", 100));
    CHECK(d.first_time("ana", "/p/1", 90));
    CHECK(d.first_time("ben", "/p/1", 100));
}

TEST_CASE("products are checked on schedule and drops are notified once") {
    World w;
    w.run_minutes(0, 60);
    CHECK_EQ(w.bot.stats().checks, 4);  // two products at minutes 0 and 30
    CHECK(w.sent.empty());
    w.shop.prices["/p/kettle"] = 4400;  // below cy's target only
    w.run_minutes(60, 120);
    CHECK_EQ(w.sent.size(), 1u);
    CHECK_EQ(w.sent[0].first, "cy");
    CHECK_EQ(w.sent[0].second, "/p/kettle is now €44.00 (your target €45.00)");
    CHECK_EQ(w.bot.stats().duplicates_suppressed, 1);  // the 90-minute check saw the same price
    w.shop.prices["/p/kettle"] = 3500;  // below both targets
    w.run_minutes(120, 150);
    CHECK_EQ(w.sent.size(), 3u);
}

TEST_CASE("flaky pages are retried and persistent failures are counted") {
    World w;
    w.shop.fail_next["/p/headphones"] = 2;
    w.shop.prices["/p/headphones"] = 9000;
    w.run_minutes(0, 1);
    CHECK_EQ(w.sent.size(), 1u);  // succeeded on the third attempt
    CHECK(w.sleeps == std::vector<std::chrono::milliseconds>{500ms, 1000ms});
    w.shop.fail_next["/p/kettle"] = 10;
    w.run_minutes(30, 31);
    CHECK_EQ(w.bot.stats().failures, 1);
    CHECK_EQ(w.bot.last_price("/p/kettle").value_or(-1), 4999);  // the last good price is kept
}

TEST_CASE("run drives the bot through a scenario") {
    std::istringstream in("run 60\nprice /p/headphones 9500\nfail /p/headphones 1\nrun 60\nstop\n");
    std::ostringstream out;
    CHECK_EQ(run(in, out), 0);
    CHECK(out.str().find("checks 4, failures 0, notifications 0, suppressed 0") != std::string::npos);
    CHECK(out.str().find("notify ben: /p/headphones is now €95.00 (your target €99.00)") != std::string::npos);
    CHECK(out.str().find("checks 8, failures 0, notifications 1, suppressed 1") != std::string::npos);
}
