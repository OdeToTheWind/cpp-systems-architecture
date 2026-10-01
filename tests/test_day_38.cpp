// Tests for Day 38 – Game Development with OOP. Every battle uses a fixed seed.
#include <memory>
#include <sstream>
#include <stdexcept>

#include "cppm/testing.hpp"
#include "day_38_game_development/lesson.hpp"

using namespace cppm::day38;

namespace {
std::vector<std::unique_ptr<Monster>> one(int health, int min_damage, int max_damage) {
    std::vector<std::unique_ptr<Monster>> monsters;
    monsters.push_back(std::make_unique<Monster>("Rat", health, min_damage, max_damage));
    return monsters;
}
}  // namespace

TEST_CASE("combatants validate their stats and never drop below zero health") {
    CHECK_THROWS_AS(Monster("Bad", 0, 1, 2), std::invalid_argument);
    CHECK_THROWS_AS(Monster("Bad", 5, 3, 2), std::invalid_argument);
    Monster rat("Rat", 5, 1, 1);
    rat.take_damage(50);
    CHECK_EQ(rat.health(), 0);
    CHECK(!rat.alive());
    rat.take_damage(-10);
    CHECK_EQ(rat.health(), 0);
}

TEST_CASE("damage rolls stay within each combatant's range") {
    std::mt19937 engine(1);
    const Hero hero("Aria", 0);
    const Ogre ogre;
    for (int i = 0; i < 200; ++i) {
        const int hit = hero.roll_damage(engine);
        CHECK(hit >= 4 && hit <= 9);
        const int smash = ogre.roll_damage(engine);
        CHECK(smash == 0 || (smash >= 6 && smash <= 12));
    }
}

TEST_CASE("potions heal up to the maximum and run out") {
    Hero hero("Aria", 1);
    CHECK(!hero.drink_potion());  // already at full health
    hero.take_damage(30);
    CHECK(hero.drink_potion());
    CHECK_EQ(hero.health(), 25);
    CHECK(!hero.drink_potion());
    CHECK_EQ(hero.potions(), 0);
}

TEST_CASE("the same seed replays the same battle") {
    Battle a(Hero("Aria", 2), standard_dungeon(), 99);
    Battle b(Hero("Aria", 2), standard_dungeon(), 99);
    for (int turn = 0; turn < 5 && a.outcome() == Outcome::ongoing; ++turn) {
        CHECK(a.play_turn(Action::attack) == b.play_turn(Action::attack));
    }
}

TEST_CASE("defeating every monster is a victory") {
    Battle battle(Hero("Aria", 0), one(1, 0, 0), 7);
    const auto log = battle.play_turn(Action::attack);
    CHECK(battle.outcome() == Outcome::victory);
    CHECK_EQ(log.back(), "Rat is defeated");
    CHECK(battle.current_monster() == nullptr);
    CHECK_THROWS_AS(battle.play_turn(Action::attack), std::logic_error);
}

TEST_CASE("losing all health is a defeat; fleeing ends the battle too") {
    Battle doomed(Hero("Aria", 0), one(1'000, 50, 50), 3);
    doomed.play_turn(Action::attack);
    CHECK(doomed.outcome() == Outcome::defeat);
    Battle coward(Hero("Aria", 0), one(10, 1, 1), 3);
    coward.play_turn(Action::flee);
    CHECK(coward.outcome() == Outcome::fled);
    CHECK_THROWS_AS(Battle(Hero("Aria", 0), {}, 1), std::invalid_argument);
}

TEST_CASE("a full game loop always ends") {
    Battle battle(Hero("Aria", 2), standard_dungeon(), 2026);
    int turns = 0;
    while (battle.outcome() == Outcome::ongoing && turns < 100) {
        battle.play_turn(battle.hero().health() < 12 && battle.hero().potions() > 0 ? Action::potion : Action::attack);
        ++turns;
    }
    CHECK(battle.outcome() != Outcome::ongoing);
    CHECK(turns < 100);
}

TEST_CASE("run plays until the battle is decided or input ends") {
    std::string commands;
    for (int i = 0; i < 60; ++i) commands += "a\n";
    std::istringstream attacks(commands);
    std::ostringstream out;
    CHECK_EQ(run(attacks, out), 0);
    const auto text = out.str();
    CHECK(text.find("Aria hits Goblin") != std::string::npos);
    CHECK(text.find("Victory!") != std::string::npos || text.find("Defeat...") != std::string::npos);
    std::istringstream flee("f\n");
    std::ostringstream out2;
    run(flee, out2);
    CHECK(out2.str().find("You escaped.") != std::string::npos);
    std::istringstream nothing("");
    std::ostringstream out3;
    run(nothing, out3);
    CHECK(out3.str().find("Game paused.") != std::string::npos);
}
