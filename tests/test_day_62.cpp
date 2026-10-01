// Tests for Day 62 – Web Scraping. Pages come from an in-memory site; the crawl delay is recorded, not slept.
#include <chrono>
#include <map>
#include <optional>
#include <sstream>
#include <string>
#include <vector>

#include "cppm/testing.hpp"
#include "day_62_web_scraping/lesson.hpp"

using namespace cppm::day62;
using namespace std::chrono_literals;

TEST_CASE("the tokenizer separates tags, attributes and decoded text") {
    const auto tokens = tokenize(R"(<!DOCTYPE html><P Class='a b' data-x=1>Fish &amp; chips<br/></p><!-- hidden -->)");
    CHECK_EQ(tokens.size(), 4u);
    CHECK(tokens[0].kind == Token::Kind::open);
    CHECK_EQ(tokens[0].name, "p");
    CHECK_EQ(tokens[0].attributes.at("class"), "a b");
    CHECK_EQ(tokens[0].attributes.at("data-x"), "1");
    CHECK_EQ(tokens[1].name, "Fish & chips");
    CHECK(tokens[2].self_closing);
    CHECK(tokens[3].kind == Token::Kind::close);
}

TEST_CASE("script bodies, comments and unknown entities are handled") {
    const auto tokens = tokenize("<script>if (a < b) {}</script><p>&#65;&bogus;</p>");
    CHECK_EQ(tokens.size(), 5u);
    CHECK_EQ(tokens[3].name, "A&bogus;");
    CHECK_EQ(decode_entities("&lt;b&gt; &quot;x&quot;"), "<b> \"x\"");
}

TEST_CASE("select matches tag and class and collects nested text") {
    const auto tokens = tokenize(R"(<div class="card sale"><div>Inner <b>bold</b></div> tail</div><div class="card">B</div>)");
    const auto cards = select(tokens, "div", "card");
    CHECK_EQ(cards.size(), 2u);
    CHECK_EQ(cards[0].text, "Inner bold tail");
    CHECK_EQ(cards[1].text, "B");
    CHECK_EQ(select(tokens, "div", "sale").size(), 1u);
    CHECK_EQ(select(tokens, "div", "car").size(), 0u);  // whole class names only
}

TEST_CASE("robots.txt uses the agent's own group and longest-match rules") {
    const auto text = "User-agent: *\nDisallow: /\n\nUser-agent: PriceWatch\nUser-agent: other\nDisallow: /private\n"
                      "Allow: /private/public\nCrawl-delay: 5 # be nice\n";
    const auto rules = RobotsRules::parse(text, "pricewatch");
    CHECK(rules.allowed("/catalogue"));
    CHECK(!rules.allowed("/private/x"));
    CHECK(rules.allowed("/private/public/page"));
    CHECK(rules.crawl_delay() == 5s);
    CHECK(!RobotsRules::parse(text, "somebot").allowed("/catalogue"));
    CHECK(RobotsRules::parse("", "x").allowed("/anything"));
}

TEST_CASE("the crawler follows pagination politely and respects robots.txt") {
    std::vector<std::chrono::seconds> sleeps;
    const auto result = crawl(demo_site, [&](auto d) { sleeps.push_back(d); }, "/catalogue/page-1");
    CHECK_EQ(result.books.size(), 3u);
    CHECK_EQ(result.books[1].title, "Emma & Persuasion");
    CHECK_EQ(result.books[0].price, "\xC2\xA3" "4.50");
    CHECK_EQ(result.books[2].url, "/book/ulysses");
    CHECK(result.skipped == std::vector<std::string>{"/catalogue/page-3"});
    CHECK(sleeps == std::vector<std::chrono::seconds>{2s});
}

TEST_CASE("pagination loops and page limits end the crawl") {
    const std::map<std::string, std::string> site{{"/a", R"(<a rel="next" href="/b">n</a>)"},
                                                  {"/b", R"(<a rel="next" href="/a">n</a>)"}};
    const Fetch fetch = [&](const std::string& p) -> std::optional<std::string> {
        const auto it = site.find(p);
        return it == site.end() ? std::nullopt : std::optional<std::string>(it->second);
    };
    CHECK_EQ(crawl(fetch, [](auto) {}, "/a").visited.size(), 2u);
    CHECK_EQ(crawl(fetch, [](auto) {}, "/a", 1).visited.size(), 1u);
}

TEST_CASE("run prints the books found") {
    std::istringstream in("\n");
    std::ostringstream out;
    CHECK_EQ(run(in, out), 0);
    CHECK(out.str().find("Dune – \xC2\xA3" "4.50 (/book/dune)") != std::string::npos);
    CHECK(out.str().find("2 page(s), 3 book(s), waited 2 s") != std::string::npos);
}
