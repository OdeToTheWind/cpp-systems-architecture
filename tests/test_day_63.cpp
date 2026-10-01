// Tests for Day 63 – Browser Automation. A fake browser replaces chromedriver; waiting advances a fake clock.
#include <sstream>
#include <stdexcept>
#include <string>

#include "cppm/testing.hpp"
#include "day_63_browser_automation/lesson.hpp"

using namespace cppm::day63;

TEST_CASE("locators map to W3C strategies and selectors") {
    CHECK_EQ(Locator::id("user").selector(), "#user");
    CHECK_EQ(Locator::css("button").selector(), ".button");
    CHECK_EQ(Locator::css("button").strategy(), "css selector");
    CHECK_EQ(Locator::link("Checkout").strategy(), "link text");
}

TEST_CASE("commands become WebDriver HTTP requests") {
    const auto find = to_wire("abc", FindElement{Locator::id("user")});
    CHECK_EQ(find.path, "/session/abc/element");
    CHECK_EQ(find.body, R"({"using":"css selector","value":"#user"})");
    CHECK_EQ(to_wire("abc", Click{"e1-2"}).path, "/session/abc/element/e1-2/click");
    CHECK_EQ(to_wire("abc", SendKeys{"e1-0", "ada"}).body, R"({"text":"ada"})");
    CHECK_EQ(to_wire("abc", Navigate{"/login"}).body, R"({"url":"/login"})");
}

TEST_CASE("elements that load late are absent until the clock reaches them") {
    FakeShop browser(1500ms);
    browser.navigate("/login");
    browser.type(*browser.find(Locator::id("user")), "ada");
    browser.type(*browser.find(Locator::id("pass")), "lovelace");
    browser.click(*browser.find(Locator::css("button")));
    CHECK_EQ(browser.url(), "/shop");
    CHECK(!browser.find(Locator::id("cart-count")).has_value());
    browser.sleep(1500ms);
    CHECK(browser.find(Locator::id("cart-count")).has_value());
}

TEST_CASE("explicit waits poll until found and time out otherwise") {
    FakeShop browser(1500ms);
    LoginPage(browser).login("ada", "lovelace");
    CHECK(browser.now() >= 1500ms);
    CHECK(browser.now() < 1700ms);  // polled every 100 ms, not a fixed long sleep
    CHECK_THROWS_AS(wait_for(browser, Locator::id("missing"), 500ms), TimeoutError);
}

TEST_CASE("stale handles from an earlier page are rejected") {
    FakeShop browser;
    browser.navigate("/login");
    const auto user = *browser.find(Locator::id("user"));
    browser.navigate("/login");
    CHECK_THROWS_AS(browser.text(user), NoSuchElement);
}

TEST_CASE("page objects run the whole checkout flow") {
    FakeShop browser;
    ShopPage shop = LoginPage(browser).login("ada", "lovelace");
    shop.add("Dune").add("Emma");
    CHECK_EQ(shop.cart_count(), 2);
    CHECK_EQ(shop.checkout(), "Order #1001 confirmed: 2 item(s)");
    CHECK_EQ(browser.wire_log().front().path, "/session/s1/url");
}

TEST_CASE("failures surface as clear errors") {
    FakeShop browser;
    CHECK_THROWS_AS(LoginPage(browser).login("ada", "wrong"), std::runtime_error);
    FakeShop slow(1500ms, 5000ms);
    ShopPage shop = LoginPage(slow).login("ada", "lovelace");
    CHECK_THROWS_AS(shop.checkout(3000ms), TimeoutError);
    std::istringstream in("ada lovelace 3\nbob x 1\n");
    std::ostringstream out;
    CHECK_EQ(run(in, out), 0);
    CHECK(out.str().find("Order #1001 confirmed: 3 item(s)") != std::string::npos);
    CHECK(out.str().find("FAILED: Wrong user name or password") != std::string::npos);
}
