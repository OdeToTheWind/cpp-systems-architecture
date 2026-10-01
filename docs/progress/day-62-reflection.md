# Day 62 – Web Scraping Reflection

**Date:** 2026-05-14 · **Level:** Advanced · **C++:** 20 · **Status:** Covered
**Code:** [`src/day_62_web_scraping/lesson.hpp`](../../src/day_62_web_scraping/lesson.hpp) · **Tests:** [`tests/test_day_62.cpp`](../../tests/test_day_62.cpp) (7 tests)

## Scenario
A *second-hand bookshop price watcher*. Catalogue pages are tokenised into tags and text, book cards are extracted by tag and class, robots.txt is honoured, and a polite crawler follows pagination links on one host with a delay between requests. Pages come from an injected fetch function, so the whole crawl runs offline against saved HTML.

## Syllabus deliverables
> Tokenising HTML, extracting elements and attributes, robots.txt rules, polite crawling

| Deliverable | Implemented in |
|---|---|
| ✅ splitting HTML into tags and text with entities decoded | `tokenize` |
| ✅ selecting elements by tag and class | `select` |
| ✅ reading Allow and Disallow rules from robots.txt | `RobotsRules::parse` |
| ✅ the longest-match robots.txt decision | `RobotsRules::allowed` |
| ✅ a single-host crawler with a delay and a page limit | `crawl` |

## Key learnings
- A tokenizer turns HTML into open tags, close tags and text; entities like &amp; are decoded only in text and attribute values.
- Selecting by tag and whole class name, with depth tracking for nested elements, is enough for most catalogue pages.
- robots.txt groups rules by user agent; the longest matching rule decides, and Allow wins a tie.
- A polite crawler stays on one host, waits the crawl delay between requests, remembers visited pages and stops at a page limit.

## Pitfalls I hit (and how I fixed them)
- A `<` inside a script body was read as a tag until script and style contents were skipped.
- Matching class "car" also matched "card" with a substring search; classes are now compared as whole words.
- Two pages linking to each other as "next" looped forever until visited pages were remembered.

## Run it
```bash
./procpp.sh 62                          # study mode: explanation, code map, notes, tests and quiz
./build/bin/day_62_web_scraping       # the interactive demo
ctest --test-dir build -R test_day_62 --output-on-failure
```

## Next step
- Day 63 automates a browser for pages that need JavaScript.
