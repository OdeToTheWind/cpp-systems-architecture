/**
 * @file
 * Day 62 – Web Scraping.
 *
 * Scenario: a *second-hand bookshop price watcher*. Catalogue pages are tokenised into tags and
 * text, book cards are extracted by tag and class, robots.txt is honoured, and a polite crawler
 * follows pagination links on one host with a delay between requests. Pages come from an
 * injected fetch function, so the whole crawl runs offline against saved HTML.
 *
 * Deliverables (syllabus):
 * - Tokenising HTML
 * - Extracting elements and attributes
 * - robots.txt rules
 * - Polite crawling
 */
#pragma once

#include <algorithm>
#include <cctype>
#include <chrono>
#include <functional>
#include <istream>
#include <map>
#include <optional>
#include <ostream>
#include <set>
#include <sstream>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "cppm/lesson.hpp"

namespace cppm::day62 {

inline constexpr Deliverable DELIVERABLES[] = {
    {"splitting HTML into tags and text with entities decoded", "tokenize"},
    {"selecting elements by tag and class", "select"},
    {"reading Allow and Disallow rules from robots.txt", "RobotsRules::parse"},
    {"the longest-match robots.txt decision", "RobotsRules::allowed"},
    {"a single-host crawler with a delay and a page limit", "crawl"},
};

struct Token {
    enum class Kind { open, close, text } kind;
    std::string name;  // tag name (lower case) or text
    std::map<std::string, std::string> attributes;
    bool self_closing = false;
};

inline std::string lower(std::string text) {
    std::transform(text.begin(), text.end(), text.begin(), [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    return text;
}

inline std::string decode_entities(std::string_view text) {
    static const std::map<std::string, std::string, std::less<>> named{
        {"amp", "&"}, {"lt", "<"}, {"gt", ">"}, {"quot", "\""}, {"apos", "'"}, {"nbsp", " "}, {"pound", "\xC2\xA3"}};
    std::string out;
    for (std::size_t i = 0; i < text.size(); ++i) {
        const auto semi = text.find(';', i);
        if (text[i] == '&' && semi != std::string_view::npos && semi - i <= 8) {
            const auto entity = text.substr(i + 1, semi - i - 1);
            if (const auto it = named.find(entity); it != named.end()) {
                out += it->second;
                i = semi;
                continue;
            }
            if (entity.size() > 1 && entity[0] == '#' && std::all_of(entity.begin() + 1, entity.end(), [](unsigned char c) { return std::isdigit(c) != 0; })) {
                const int code = std::stoi(std::string(entity.substr(1)));
                if (code > 0 && code < 128) {
                    out += static_cast<char>(code);
                    i = semi;
                    continue;
                }
            }
        }
        out += text[i];
    }
    return out;
}

/// A forgiving tokenizer for well-behaved pages: comments and <script>/<style> bodies are
/// skipped, attribute values may use either quote or none, and whitespace-only text is dropped.
inline std::vector<Token> tokenize(std::string_view html) {
    std::vector<Token> tokens;
    std::size_t i = 0;
    while (i < html.size()) {
        if (html.substr(i, 4) == "<!--") {
            const auto end = html.find("-->", i);
            i = end == std::string_view::npos ? html.size() : end + 3;
            continue;
        }
        if (html[i] != '<') {
            const auto end = std::min(html.find('<', i), html.size());
            const std::string text = decode_entities(html.substr(i, end - i));
            if (text.find_first_not_of(" \t\r\n") != std::string::npos) tokens.push_back({Token::Kind::text, text, {}});
            i = end;
            continue;
        }
        const auto end = html.find('>', i);
        if (end == std::string_view::npos) break;  // truncated tag: stop
        std::string_view inside = html.substr(i + 1, end - i - 1);
        i = end + 1;
        if (!inside.empty() && inside[0] == '!') continue;  // <!DOCTYPE>
        Token token{Token::Kind::open, "", {}};
        if (!inside.empty() && inside[0] == '/') {
            token.kind = Token::Kind::close;
            inside.remove_prefix(1);
        }
        if (!inside.empty() && inside.back() == '/') {
            token.self_closing = true;
            inside.remove_suffix(1);
        }
        std::size_t p = 0;
        auto skip_space = [&] {
            while (p < inside.size() && std::isspace(static_cast<unsigned char>(inside[p]))) ++p;
        };
        auto read_word = [&] {
            const auto start = p;
            while (p < inside.size() && !std::isspace(static_cast<unsigned char>(inside[p])) && inside[p] != '=') ++p;
            return lower(std::string(inside.substr(start, p - start)));
        };
        token.name = read_word();
        for (skip_space(); p < inside.size(); skip_space()) {
            const std::string key = read_word();
            std::string value;
            skip_space();
            if (p < inside.size() && inside[p] == '=') {
                ++p;
                skip_space();
                if (p < inside.size() && (inside[p] == '"' || inside[p] == '\'')) {
                    const char quote = inside[p++];
                    const auto close = std::min(inside.find(quote, p), inside.size());
                    value = inside.substr(p, close - p);
                    p = std::min(close + 1, inside.size());
                } else {
                    const auto start = p;
                    while (p < inside.size() && !std::isspace(static_cast<unsigned char>(inside[p]))) ++p;
                    value = inside.substr(start, p - start);
                }
            }
            if (!key.empty()) token.attributes[key] = decode_entities(value);
        }
        tokens.push_back(token);
        if (token.kind == Token::Kind::open && (token.name == "script" || token.name == "style")) {
            const auto close = html.find("</" + token.name, i);
            i = close == std::string_view::npos ? html.size() : close;
        }
    }
    return tokens;
}

struct Element {
    std::string tag;
    std::map<std::string, std::string> attributes;
    std::string text;  // all text inside, joined with single spaces
    std::string attr(const std::string& name) const {
        const auto it = attributes.find(name);
        return it == attributes.end() ? "" : it->second;
    }
};

inline bool has_class(const Token& token, std::string_view wanted) {
    const auto it = token.attributes.find("class");
    if (it == token.attributes.end()) return false;
    std::istringstream classes(it->second);
    for (std::string c; classes >> c;) {
        if (c == wanted) return true;
    }
    return false;
}

/// Elements named @p tag (and carrying class @p css_class if given) with their inner text.
inline std::vector<Element> select(const std::vector<Token>& tokens, const std::string& tag, std::string_view css_class = {}) {
    static const std::set<std::string> void_tags{"br", "img", "input", "meta", "link", "hr"};
    std::vector<Element> found;
    for (std::size_t i = 0; i < tokens.size(); ++i) {
        const Token& token = tokens[i];
        if (token.kind != Token::Kind::open || token.name != tag) continue;
        if (!css_class.empty() && !has_class(token, css_class)) continue;
        Element element{token.name, token.attributes, ""};
        if (!token.self_closing && !void_tags.contains(tag)) {
            int depth = 1;
            for (std::size_t j = i + 1; j < tokens.size() && depth > 0; ++j) {
                const Token& inner = tokens[j];
                if (inner.kind == Token::Kind::text) {
                    std::istringstream words(inner.name);
                    for (std::string w; words >> w;) element.text += (element.text.empty() ? "" : " ") + w;
                } else if (inner.name == tag && !inner.self_closing) {
                    depth += inner.kind == Token::Kind::open ? 1 : -1;
                }
            }
        }
        found.push_back(element);
    }
    return found;
}

/// The group of robots.txt rules for one user agent (or '*').
class RobotsRules {
public:
    static RobotsRules parse(std::string_view text, const std::string& agent) {
        RobotsRules specific;
        RobotsRules any;
        RobotsRules* current = nullptr;
        bool in_agents = false;
        std::istringstream lines{std::string(text)};
        for (std::string line; std::getline(lines, line);) {
            line = line.substr(0, line.find('#'));
            const auto colon = line.find(':');
            if (colon == std::string::npos) continue;
            const std::string key = lower(trim(line.substr(0, colon)));
            const std::string value = trim(line.substr(colon + 1));
            if (key == "user-agent") {
                if (!in_agents) current = nullptr;
                in_agents = true;
                if (value == "*") current = &any;
                else if (lower(value) == lower(agent)) current = &specific;
                if (current) current->matched_ = true;
                continue;
            }
            in_agents = false;
            if (current == nullptr) continue;
            if (key == "allow" || key == "disallow") {
                if (!value.empty()) current->rules_.emplace_back(value, key == "allow");
            } else if (key == "crawl-delay") {
                current->delay_ = std::chrono::seconds{std::stoi(value)};
            }
        }
        return specific.matched_ ? specific : any;
    }

    /// The longest matching rule wins; on a tie Allow wins; no match means allowed.
    bool allowed(std::string_view path) const {
        std::size_t best = 0;
        bool verdict = true;
        for (const auto& [prefix, allow] : rules_) {
            if (path.starts_with(prefix) && (prefix.size() > best || (prefix.size() == best && allow))) {
                best = prefix.size();
                verdict = allow;
            }
        }
        return verdict;
    }
    std::chrono::seconds crawl_delay() const { return delay_; }

private:
    static std::string trim(const std::string& s) {
        const auto first = s.find_first_not_of(" \t\r");
        return first == std::string::npos ? "" : s.substr(first, s.find_last_not_of(" \t\r") - first + 1);
    }
    std::vector<std::pair<std::string, bool>> rules_;
    std::chrono::seconds delay_{1};
    bool matched_ = false;
};

struct Book {
    std::string title;
    std::string price;
    std::string url;
};

struct CrawlResult {
    std::vector<Book> books;
    std::vector<std::string> visited;
    std::vector<std::string> skipped;  // blocked by robots.txt
    std::chrono::seconds waited{0};
};

using Fetch = std::function<std::optional<std::string>(const std::string& path)>;
using Sleep = std::function<void(std::chrono::seconds)>;

/// Follow rel="next" links from @p start on one host: honour robots.txt, wait the crawl delay
/// between requests, never visit a page twice and stop after @p max_pages.
inline CrawlResult crawl(const Fetch& fetch, const Sleep& sleep, const std::string& start, int max_pages = 10) {
    const RobotsRules robots = RobotsRules::parse(fetch("/robots.txt").value_or(""), "pricewatch");
    CrawlResult result;
    std::set<std::string> seen;
    std::optional<std::string> next = start;
    while (next && static_cast<int>(result.visited.size()) < max_pages) {
        const std::string path = *next;
        next.reset();
        if (!seen.insert(path).second) break;  // pagination loop
        if (path.find("://") != std::string::npos) break;  // off-host link
        if (!robots.allowed(path)) {
            result.skipped.push_back(path);
            break;
        }
        if (!result.visited.empty()) {
            sleep(robots.crawl_delay());
            result.waited += robots.crawl_delay();
        }
        const auto page = fetch(path);
        result.visited.push_back(path);
        if (!page) continue;
        const auto tokens = tokenize(*page);
        const auto titles = select(tokens, "h3", "title");
        const auto prices = select(tokens, "p", "price");
        const auto links = select(tokens, "a", "title-link");
        for (std::size_t i = 0; i < titles.size() && i < prices.size(); ++i) {
            result.books.push_back({titles[i].text, prices[i].text, i < links.size() ? links[i].attr("href") : ""});
        }
        for (const auto& link : select(tokens, "a")) {
            if (link.attr("rel") == "next") next = link.attr("href");
        }
    }
    return result;
}

/// A tiny saved copy of the shop, used by the demo.
inline std::optional<std::string> demo_site(const std::string& path) {
    static const std::map<std::string, std::string> pages{
        {"/robots.txt", "User-agent: *\nDisallow: /admin\nDisallow: /catalogue/page-3\nCrawl-delay: 2\n"},
        {"/catalogue/page-1",
         R"(<html><body><!-- featured --><article class="book"><h3 class="title">Dune</h3>
<a class="title-link" href="/book/dune">more</a><p class="price">&pound;4.50</p></article>
<article class="book"><h3 class="title">Emma &amp; Persuasion</h3><a class="title-link" href="/book/emma">more</a>
<p class="price">&pound;3.00</p></article><a rel="next" href="/catalogue/page-2">next</a></body></html>)"},
        {"/catalogue/page-2",
         R"(<article class="book"><h3 class="title">Ulysses</h3><a class="title-link" href=/book/ulysses>more</a>
<p class="price">&pound;6.25</p></article><a rel=next href="/catalogue/page-3">next</a>)"},
        {"/catalogue/page-3", "<p>should never be fetched</p>"}};
    const auto it = pages.find(path);
    return it == pages.end() ? std::nullopt : std::optional<std::string>(it->second);
}

/// The interactive demo: enter a start path (default /catalogue/page-1) to crawl the saved shop.
inline int run(std::istream& in, std::ostream& out) {
    out << "Day 62 – Web Scraping\n";
    auto line = prompt_line(in, out, "start path [/catalogue/page-1]> ");
    const std::string start = line && !line->empty() ? *line : "/catalogue/page-1";
    const auto result = crawl(demo_site, [](std::chrono::seconds) {}, start);
    for (const auto& book : result.books) out << "  " << book.title << " – " << book.price << " (" << book.url << ")\n";
    for (const auto& path : result.skipped) out << "  robots.txt blocks " << path << '\n';
    out << result.visited.size() << " page(s), " << result.books.size() << " book(s), waited " << result.waited.count()
        << " s\n";
    return 0;
}

}  // namespace cppm::day62
