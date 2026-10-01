/**
 * @file
 * Day 63 – Browser Automation.
 *
 * Scenario: a *checkout smoke test* for an online shop, run before every release. The test logs
 * in, adds a book to the cart and checks the order confirmation. A real run would drive a browser
 * through the W3C WebDriver protocol; here a fake browser with delayed elements stands in for it,
 * so locators, explicit waits and page objects can be practised – and tested – offline.
 *
 * Deliverables (syllabus):
 * - WebDriver commands
 * - Locator strategies
 * - Explicit waits
 * - The page-object pattern
 */
#pragma once

#include <cctype>
#include <chrono>
#include <functional>
#include <istream>
#include <optional>
#include <ostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <variant>
#include <vector>

#include "cppm/lesson.hpp"

namespace cppm::day63 {

inline constexpr Deliverable DELIVERABLES[] = {
    {"locators by id, CSS class and link text", "Locator"},
    {"WebDriver commands as W3C HTTP requests", "to_wire"},
    {"a fake browser that implements the driver interface", "FakeShop"},
    {"an explicit wait with a timeout and poll interval", "wait_until"},
    {"page objects that hide locators from the test", "LoginPage::login"},
};

using namespace std::chrono_literals;
using Millis = std::chrono::milliseconds;

struct Locator {
    enum class By { id, css_class, link_text } by;
    std::string value;
    static Locator id(std::string v) { return {By::id, std::move(v)}; }
    static Locator css(std::string v) { return {By::css_class, std::move(v)}; }
    static Locator link(std::string v) { return {By::link_text, std::move(v)}; }
    /// The W3C "using" strategy name.
    std::string strategy() const { return by == By::link_text ? "link text" : "css selector"; }
    std::string selector() const { return by == By::id ? "#" + value : by == By::css_class ? "." + value : value; }
};

// The commands a test sends, and their wire form.
struct Navigate { std::string url; };
struct FindElement { Locator locator; };
struct Click { std::string element; };
struct SendKeys { std::string element; std::string text; };
using Command = std::variant<Navigate, FindElement, Click, SendKeys>;

struct WireRequest {
    std::string method;
    std::string path;
    std::string body;
};

/// What a WebDriver client would POST to chromedriver/geckodriver for each command.
inline WireRequest to_wire(const std::string& session, const Command& command) {
    const std::string base = "/session/" + session;
    struct Visitor {
        const std::string& base;
        WireRequest operator()(const Navigate& c) const { return {"POST", base + "/url", R"({"url":")" + c.url + "\"}"}; }
        WireRequest operator()(const FindElement& c) const {
            return {"POST", base + "/element",
                    R"({"using":")" + c.locator.strategy() + R"(","value":")" + c.locator.selector() + "\"}"};
        }
        WireRequest operator()(const Click& c) const { return {"POST", base + "/element/" + c.element + "/click", "{}"}; }
        WireRequest operator()(const SendKeys& c) const {
            return {"POST", base + "/element/" + c.element + "/value", R"({"text":")" + c.text + "\"}"};
        }
    };
    return std::visit(Visitor{base}, command);
}

class NoSuchElement : public std::runtime_error {
    using std::runtime_error::runtime_error;
};
class TimeoutError : public std::runtime_error {
    using std::runtime_error::runtime_error;
};

/// The part of WebDriver the tests need. Element handles are opaque strings, as in the protocol.
class Driver {
public:
    virtual ~Driver() = default;
    virtual void navigate(const std::string& url) = 0;
    virtual std::optional<std::string> find(const Locator& locator) = 0;  // nullopt: not (yet) present
    virtual void click(const std::string& element) = 0;
    virtual void type(const std::string& element, const std::string& text) = 0;
    virtual std::string text(const std::string& element) = 0;
    virtual std::string url() const = 0;
    virtual Millis now() const = 0;
    virtual void sleep(Millis duration) = 0;
};

/// Poll @p condition until it holds or @p timeout passes – never a fixed sleep.
template <typename Condition>
auto wait_until(Driver& driver, Condition condition, Millis timeout, Millis poll = 100ms, const std::string& what = "condition") {
    const Millis deadline = driver.now() + timeout;
    while (true) {
        if (auto result = condition()) return *result;
        if (driver.now() >= deadline) throw TimeoutError("timed out after " + std::to_string(timeout.count()) + " ms waiting for " + what);
        driver.sleep(poll);
    }
}

/// A fake browser running a tiny shop: /login -> /shop -> /done. Elements can appear late,
/// like content loaded by JavaScript, and every command is logged in its wire form.
class FakeShop : public Driver {
public:
    explicit FakeShop(Millis login_delay = 1500ms, Millis confirm_delay = 800ms)
        : login_delay_(login_delay), confirm_delay_(confirm_delay) {}

    void navigate(const std::string& url) override {
        log(Navigate{url});
        go(url);
    }
    std::optional<std::string> find(const Locator& locator) override {
        log(FindElement{locator});
        for (std::size_t i = 0; i < elements_.size(); ++i) {
            const auto& e = elements_[i];
            if (e.visible_at > clock_) continue;
            const bool match = (locator.by == Locator::By::id && e.id == locator.value) ||
                               (locator.by == Locator::By::css_class && e.css_class == locator.value) ||
                               (locator.by == Locator::By::link_text && e.link && e.text == locator.value);
            if (match) return handle(i);
        }
        return std::nullopt;
    }
    void click(const std::string& element) override {
        log(Click{element});
        // Copy the handler: a click that navigates replaces elements_, destroying the original.
        const auto action = at(element).on_click;
        if (action) action();
    }
    void type(const std::string& element, const std::string& text) override {
        log(SendKeys{element, text});
        at(element).value += text;
    }
    std::string text(const std::string& element) override { return at(element).text; }
    std::string url() const override { return url_; }
    Millis now() const override { return clock_; }
    void sleep(Millis duration) override { clock_ += duration; }
    const std::vector<WireRequest>& wire_log() const { return wire_; }

private:
    struct Element {
        std::string id;
        std::string css_class;
        std::string text;
        bool link = false;
        Millis visible_at{0};
        std::function<void()> on_click;
        std::string value;
    };

    std::string handle(std::size_t index) const { return "e" + std::to_string(generation_) + "-" + std::to_string(index); }
    Element& at(const std::string& handle_text) {
        const std::string prefix = "e" + std::to_string(generation_) + "-";
        if (!handle_text.starts_with(prefix)) throw NoSuchElement("stale element reference " + handle_text);
        const auto index = std::stoul(handle_text.substr(prefix.size()));
        if (index >= elements_.size()) throw NoSuchElement("no element " + handle_text);
        return elements_[index];
    }
    void log(const Command& command) { wire_.push_back(to_wire("s1", command)); }
    std::string value_of(const std::string& id) const {
        for (const auto& e : elements_) {
            if (e.id == id) return e.value;
        }
        return "";
    }
    void go(const std::string& url) {
        url_ = url;
        ++generation_;  // old handles become stale, as after a real page load
        elements_.clear();
        if (url == "/login") {
            elements_ = {{"user", "", "", false, 0ms, {}, ""},
                         {"pass", "", "", false, 0ms, {}, ""},
                         {"login", "button", "Log in", false, 0ms, [this] { submit_login(); }, ""}};
        } else if (url == "/shop") {
            elements_ = {{"add-dune", "add-to-cart", "Add Dune", false, clock_ + login_delay_, [this] { add("Dune"); }, ""},
                         {"add-emma", "add-to-cart", "Add Emma", false, clock_ + login_delay_, [this] { add("Emma"); }, ""},
                         {"cart-count", "", std::to_string(cart_.size()), false, clock_ + login_delay_, {}, ""},
                         {"", "", "Checkout", true, clock_ + login_delay_, [this] { checkout(); }, ""}};
        } else if (url == "/done") {
            elements_ = {{"confirmation", "", "Order #1001 confirmed: " + std::to_string(cart_.size()) + " item(s)", false,
                          clock_ + confirm_delay_, {}, ""}};
        } else if (url == "/login?error") {
            elements_ = {{"error", "alert", "Wrong user name or password", false, 0ms, {}, ""}};
        }
    }
    void submit_login() { go(value_of("user") == "ada" && value_of("pass") == "lovelace" ? "/shop" : "/login?error"); }
    void add(const std::string& title) {
        cart_.push_back(title);
        for (auto& e : elements_) {
            if (e.id == "cart-count") e.text = std::to_string(cart_.size());
        }
    }
    void checkout() { go("/done"); }

    Millis login_delay_;
    Millis confirm_delay_;
    Millis clock_{0};
    std::string url_ = "about:blank";
    int generation_{0};
    std::vector<Element> elements_;
    std::vector<std::string> cart_;
    std::vector<WireRequest> wire_;
};

/// Wait until @p locator finds an element and return its handle.
inline std::string wait_for(Driver& driver, const Locator& locator, Millis timeout = 3000ms) {
    return wait_until(driver, [&] { return driver.find(locator); }, timeout, 100ms, locator.selector());
}

// Page objects: tests talk about "log in" and "add to cart", never about selectors.
class ShopPage {
public:
    explicit ShopPage(Driver& driver) : driver_(driver) { wait_for(driver_, Locator::id("cart-count")); }
    /// Each book has an "add-<slug>" button, e.g. add-dune.
    ShopPage& add(const std::string& title) {
        std::string slug;
        for (const char c : title) slug += static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
        driver_.click(wait_for(driver_, Locator::id("add-" + slug)));
        return *this;
    }
    int cart_count() { return std::stoi(driver_.text(wait_for(driver_, Locator::id("cart-count")))); }
    /// Go to checkout and return the confirmation text once it appears.
    std::string checkout(Millis timeout = 3000ms) {
        driver_.click(wait_for(driver_, Locator::link("Checkout")));
        return driver_.text(wait_for(driver_, Locator::id("confirmation"), timeout));
    }

private:
    Driver& driver_;
};

class LoginPage {
public:
    explicit LoginPage(Driver& driver) : driver_(driver) { driver_.navigate("/login"); }
    /// Fill in the form and wait for the shop. A rejected login throws with the page's message.
    ShopPage login(const std::string& user, const std::string& password) {
        driver_.type(wait_for(driver_, Locator::id("user")), user);
        driver_.type(wait_for(driver_, Locator::id("pass")), password);
        driver_.click(wait_for(driver_, Locator::css("button")));
        if (const auto error = driver_.find(Locator::css("alert"))) throw std::runtime_error(driver_.text(*error));
        return ShopPage(driver_);
    }

private:
    Driver& driver_;
};

/// The interactive demo: "user password items" runs the smoke test and prints the wire log size.
inline int run(std::istream& in, std::ostream& out) {
    out << "Day 63 – Browser Automation\n";
    while (auto line = prompt_line(in, out, "user password items> ")) {
        std::istringstream words(*line);
        std::string user;
        std::string password;
        int items = 0;
        if (!(words >> user >> password >> items)) break;
        FakeShop browser;
        try {
            ShopPage shop = LoginPage(browser).login(user, password);
            for (int i = 0; i < items; ++i) shop.add(i % 2 == 0 ? "Dune" : "Emma");
            out << "  cart: " << shop.cart_count() << '\n';
            out << "  " << shop.checkout() << " at " << browser.now().count() << " ms\n";
        } catch (const std::exception& error) {
            out << "  FAILED: " << error.what() << '\n';
        }
        out << "  " << browser.wire_log().size() << " WebDriver command(s), first: " << browser.wire_log().front().method
            << ' ' << browser.wire_log().front().path << '\n';
    }
    return 0;
}

}  // namespace cppm::day63
