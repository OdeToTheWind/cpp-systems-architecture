/**
 * @file
 * Day 97 – Capstone: Automation Bot.
 *
 * Scenario: a *price-drop bot*. Users put products on a wishlist with a target price (served by a
 * small JSON API); the bot checks each product page on a schedule, scrapes the current price,
 * retries flaky fetches with back-off, and notifies users when a price falls to their target –
 * exactly once per price, however often the bot runs. Network, clock and notifications are all
 * injected, so a whole day of bot activity is a fast, deterministic test.
 *
 * Deliverables (syllabus):
 * - Scraping and APIs
 * - Scheduling
 * - Notifications with de-duplication
 * - Retries with back-off
 */
#pragma once

#include <chrono>
#include <cstdint>
#include <functional>
#include <istream>
#include <map>
#include <optional>
#include <ostream>
#include <regex>
#include <set>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

#include "cppm/lesson.hpp"

namespace cppm::day97 {

inline constexpr Deliverable DELIVERABLES[] = {
    {"scraping a price from a product page", "extract_price"},
    {"reading wishlist subscriptions from a JSON API", "parse_wishlist"},
    {"retrying transient failures with exponential back-off", "with_retries"},
    {"notifying once per user, product and price", "Deduplicator::first_time"},
    {"a scheduled check of every product", "PriceBot::tick"},
};

/// "€1,299.00" or "€12.99" inside <span class="price">…</span> -> cents.
inline std::optional<std::int64_t> extract_price(const std::string& html) {
    static const std::regex price(R"re(<span class="price">\s*(?:€|&euro;|EUR\s*)?([0-9][0-9,]*)\.([0-9]{2})\s*</span>)re");
    std::smatch m;
    if (!std::regex_search(html, m, price)) return std::nullopt;
    std::string whole = m[1].str();
    std::erase(whole, ',');
    if (whole.size() > 9) return std::nullopt;
    return std::stoll(whole) * 100 + std::stoll(m[2].str());
}

struct Subscription {
    std::string user;
    std::string url;
    std::int64_t target_cents;
};

/// Parse `[{"user":"ana","url":"/p/1","target_cents":1500}, …]` – a flat array of flat objects.
inline std::vector<Subscription> parse_wishlist(const std::string& json) {
    static const std::regex object(R"(\{([^{}]*)\})");
    static const std::regex field(R"re("(\w+)"\s*:\s*(?:"([^"]*)"|(-?\d+)))re");
    std::vector<Subscription> subs;
    for (std::sregex_iterator it(json.begin(), json.end(), object), end; it != end; ++it) {
        std::map<std::string, std::string> f;
        const std::string body = (*it)[1].str();
        for (std::sregex_iterator fi(body.begin(), body.end(), field); fi != end; ++fi) {
            f[(*fi)[1].str()] = (*fi)[2].matched ? (*fi)[2].str() : (*fi)[3].str();
        }
        if (!f.contains("user") || !f.contains("url") || !f.contains("target_cents")) throw std::runtime_error("wishlist entry is missing a field");
        subs.push_back({f["user"], f["url"], std::stoll(f["target_cents"])});
    }
    return subs;
}

/// A failure worth retrying (timeouts, 503s). Anything else propagates at once.
struct TransientError : std::runtime_error {
    using std::runtime_error::runtime_error;
};

using Sleep = std::function<void(std::chrono::milliseconds)>;

/// Call @p fn up to @p attempts times, sleeping first_delay, 2x, 4x … between transient failures.
template <typename Fn>
auto with_retries(Fn fn, int attempts, std::chrono::milliseconds first_delay, const Sleep& sleep) -> decltype(fn()) {
    if (attempts < 1) throw std::invalid_argument("attempts must be at least 1");
    auto delay = first_delay;
    for (int attempt = 1;; ++attempt) {
        try {
            return fn();
        } catch (const TransientError&) {
            if (attempt == attempts) throw;
            sleep(delay);
            delay *= 2;
        }
    }
}

/// Remembers which notifications were sent. A price that drops, rises and drops to the same value
/// again is not re-announced; a new lower price is.
class Deduplicator {
public:
    bool first_time(const std::string& user, const std::string& url, std::int64_t cents) {
        return sent_.insert(user + "|" + url + "|" + std::to_string(cents)).second;
    }
    std::size_t size() const { return sent_.size(); }

private:
    std::set<std::string> sent_;
};

using Fetch = std::function<std::string(const std::string& path)>;  // throws TransientError for flaky responses
using Notify = std::function<void(const std::string& user, const std::string& message)>;

struct BotStats {
    int checks = 0;
    int failures = 0;
    int notifications = 0;
    int duplicates_suppressed = 0;
};

class PriceBot {
public:
    PriceBot(Fetch fetch, Notify notify, Sleep sleep, std::chrono::minutes interval)
        : fetch_(std::move(fetch)), notify_(std::move(notify)), sleep_(std::move(sleep)), interval_(interval) {}

    /// Refresh subscriptions from the wishlist API (GET /api/wishlist).
    void sync_wishlist() { subscriptions_ = parse_wishlist(with_retries([&] { return fetch_("/api/wishlist"); }, 3, std::chrono::milliseconds(500), sleep_)); }

    /// Check every product that is due at @p now (minutes since start).
    void tick(std::chrono::minutes now) {
        std::set<std::string> urls;
        for (const auto& s : subscriptions_) urls.insert(s.url);
        for (const auto& url : urls) {
            auto& next = next_check_[url];
            if (now < next) continue;
            next = now + interval_;
            ++stats_.checks;
            std::optional<std::int64_t> price;
            try {
                price = extract_price(with_retries([&] { return fetch_(url); }, 3, std::chrono::milliseconds(500), sleep_));
            } catch (const std::exception&) {
                ++stats_.failures;  // give up on this product until its next scheduled check
                continue;
            }
            if (!price) {
                ++stats_.failures;
                continue;
            }
            last_price_[url] = *price;
            for (const auto& s : subscriptions_) {
                if (s.url != url || *price > s.target_cents) continue;
                if (!dedup_.first_time(s.user, url, *price)) {
                    ++stats_.duplicates_suppressed;
                    continue;
                }
                notify_(s.user, url + " is now " + format_cents(*price) + " (your target " + format_cents(s.target_cents) + ")");
                ++stats_.notifications;
            }
        }
    }
    const BotStats& stats() const { return stats_; }
    std::optional<std::int64_t> last_price(const std::string& url) const {
        const auto it = last_price_.find(url);
        return it == last_price_.end() ? std::nullopt : std::optional<std::int64_t>(it->second);
    }
    static std::string format_cents(std::int64_t c) { return "€" + std::to_string(c / 100) + "." + (c % 100 < 10 ? "0" : "") + std::to_string(c % 100); }

private:
    Fetch fetch_;
    Notify notify_;
    Sleep sleep_;
    std::chrono::minutes interval_;
    std::vector<Subscription> subscriptions_;
    std::map<std::string, std::chrono::minutes> next_check_;
    std::map<std::string, std::int64_t> last_price_;
    Deduplicator dedup_;
    BotStats stats_;
};

/// A tiny simulated shop whose prices and failures the demo (and tests) can change.
struct FakeShop {
    std::map<std::string, std::int64_t> prices{{"/p/kettle", 4999}, {"/p/headphones", 12900}};
    std::map<std::string, int> fail_next;  // url -> number of transient failures to produce
    std::string wishlist = R"([{"user":"ana","url":"/p/kettle","target_cents":3999},)"
                           R"( {"user":"ben","url":"/p/headphones","target_cents":9900},)"
                           R"( {"user":"cy","url":"/p/kettle","target_cents":4500}])";
    std::string fetch(const std::string& path) {
        if (path == "/api/wishlist") return wishlist;
        if (fail_next[path] > 0) {
            --fail_next[path];
            throw TransientError("503 from " + path);
        }
        const auto it = prices.find(path);
        if (it == prices.end()) throw std::runtime_error("404 " + path);
        return "<html><h1>Product</h1><span class=\"price\">€" + std::to_string(it->second / 100) + "." +
               (it->second % 100 < 10 ? "0" : "") + std::to_string(it->second % 100) + "</span></html>";
    }
};

/// The interactive demo: "price <path> <cents>", "fail <path> <n>", "run <minutes>" (bot runs each minute).
inline int run(std::istream& in, std::ostream& out) {
    out << "Day 97 – Capstone: Automation Bot\n";
    FakeShop shop;
    PriceBot bot([&shop](const std::string& p) { return shop.fetch(p); },
                 [&out](const std::string& user, const std::string& message) { out << "  notify " << user << ": " << message << '\n'; },
                 [](std::chrono::milliseconds) {}, std::chrono::minutes(30));
    bot.sync_wishlist();
    std::chrono::minutes now{0};
    while (auto line = prompt_line(in, out, "price <path> <cents> | fail <path> <n> | run <minutes>> ")) {
        std::istringstream words(*line);
        std::string command;
        if (!(words >> command)) break;
        if (command == "price") {
            std::string path;
            std::int64_t cents = 0;
            words >> path >> cents;
            shop.prices[path] = cents;
        } else if (command == "fail") {
            std::string path;
            int n = 0;
            words >> path >> n;
            shop.fail_next[path] = n;
        } else if (command == "run") {
            int minutes = 0;
            words >> minutes;
            for (const auto end = now + std::chrono::minutes(minutes); now < end; ++now) bot.tick(now);
            const auto& s = bot.stats();
            out << "  checks " << s.checks << ", failures " << s.failures << ", notifications " << s.notifications << ", suppressed "
                << s.duplicates_suppressed << '\n';
        }
    }
    return 0;
}

}  // namespace cppm::day97
