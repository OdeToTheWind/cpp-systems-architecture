// Tests for Day 67 – Smart Pointers & Ownership. Live-instance counters prove what was freed.
#include <memory>
#include <sstream>
#include <stdexcept>
#include <string>
#include <type_traits>

#include "cppm/testing.hpp"
#include "day_67_smart_pointers/lesson.hpp"

using namespace cppm::day67;

TEST_CASE("unique_ptr ownership: slides die with their deck") {
    const int before = Slide::live;
    {
        Deck deck;
        deck.add_slide("Intro");
        deck.add_slide("Results").title += "!";
        CHECK_EQ(Slide::live, before + 2);
        CHECK(deck.find("Results!") != nullptr);
        CHECK(deck.find("Missing") == nullptr);
    }
    CHECK_EQ(Slide::live, before);
    static_assert(!std::is_copy_constructible_v<Deck> && std::is_move_constructible_v<Deck>);
}

TEST_CASE("ownership moves out of and back into the deck") {
    Deck deck;
    deck.add_slide("A");
    deck.add_slide("B");
    std::unique_ptr<Slide> taken = deck.take_slide(0);
    CHECK_EQ(taken->title, "A");
    CHECK_EQ(deck.size(), 1u);
    deck.insert_slide(0, std::move(taken));
    CHECK(taken == nullptr);  // moved-from unique_ptr is empty
    CHECK_EQ(deck.size(), 2u);
    CHECK_THROWS_AS(deck.take_slide(5), std::out_of_range);
    CHECK_THROWS_AS(deck.insert_slide(0, nullptr), std::invalid_argument);
}

TEST_CASE("shared images live while any slide uses them") {
    AssetCache cache;
    const int before = Image::live;
    auto logo = cache.load("logo.png");
    {
        auto again = cache.load("logo.png");
        CHECK(again == logo);
        CHECK_EQ(logo.use_count(), 2);
    }
    CHECK_EQ(cache.loads(), 1);
    CHECK_EQ(cache.hits(), 1);
    logo.reset();
    CHECK_EQ(Image::live, before);  // the weak cache did not keep it alive
    cache.load("logo.png");
    CHECK_EQ(cache.loads(), 2);  // so it had to be loaded again
}

TEST_CASE("weak parent links give paths without keeping parents alive") {
    const int before = Shape::live;
    std::shared_ptr<Shape> leaf;
    {
        auto root = std::make_shared<Shape>("deck");
        auto group = root->add_child("chart");
        leaf = group->add_child("legend");
        CHECK_EQ(leaf->path(), "deck/chart/legend");
        CHECK_EQ(root->path(), "deck");
        CHECK_EQ(root->child_count(), 1u);
    }
    CHECK_EQ(Shape::live, before + 1);  // root and group were freed despite the child's link
    CHECK_EQ(leaf->path(), "?/legend");
}

TEST_CASE("a strong cycle outlives its scope until broken") {
    const int before = StrongNode::live;
    CHECK(cycle_survives_scope());
    CHECK_EQ(StrongNode::live, before);  // broken by hand, so nothing leaked
}

TEST_CASE("counted copies are tracked too") {
    const int before = Image::live;
    {
        Image a("a.png");
        Image b = a;
        CHECK_EQ(Image::live, before + 2);
    }
    CHECK_EQ(Image::live, before);
}

TEST_CASE("run shares images and frees everything on close") {
    std::istringstream in("add Intro logo\nadd Plan logo\nadd Data chart\ntake 1\ntake 9\nundo\nstop\n");
    std::ostringstream out;
    CHECK_EQ(run(in, out), 0);
    const auto text = out.str();
    CHECK(text.find("removed 'Plan' (kept for undo)") != std::string::npos);
    CHECK(text.find("no slide 9") != std::string::npos);
    CHECK(text.find("assets: 2 load(s), 1 cache hit(s)") != std::string::npos);
    CHECK(text.find("after closing: 0 slide(s), 0 image(s) alive") != std::string::npos);
}
