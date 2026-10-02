/**
 * @file
 * Day 38 – Game Development with OOP.
 *
 * Scenario: *Dungeon Duel*, a turn-based battle. A hero fights a sequence of monsters; both
 * sides attack, the hero can drink a limited number of potions, and the game can be won or
 * lost. Randomness is injected through the engine, so every battle can be replayed in tests.
 *
 * Deliverables (syllabus):
 * - Game entities as classes
 * - Turn-based game loop
 * - Injected randomness
 * - Win and lose conditions
 */
#pragma once

#include <algorithm>
#include <cstdint>
#include <istream>
#include <memory>
#include <ostream>
#include <random>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

#include "cppm/lesson.hpp"

namespace cppm::day38 {

inline constexpr Deliverable DELIVERABLES[] = {
    {"a base class for every combatant", "Combatant"},
    {"the player's character with extra abilities", "Hero"},
    {"enemies that specialise the base class", "Monster"},
    {"randomness injected through a seeded engine", "Battle::Battle"},
    {"one turn of the game loop", "Battle::play_turn"},
    {"win and lose conditions", "Battle::outcome"},
};

/// Anything with health that can attack and be attacked.
class Combatant {
  public:
    Combatant(std::string name, int health, int min_damage, int max_damage)
        : name_(std::move(name)),
          health_(health),
          max_health_(health),
          min_damage_(min_damage),
          max_damage_(max_damage) {
        if (health <= 0 || min_damage < 0 || max_damage < min_damage) {
            throw std::invalid_argument("invalid combatant stats");
        }
    }
    virtual ~Combatant() = default;

    const std::string& name() const { return name_; }
    int health() const { return health_; }
    int max_health() const { return max_health_; }
    bool alive() const { return health_ > 0; }

    /// Roll damage with the battle's engine; derived classes may change how much they deal.
    virtual int roll_damage(std::mt19937& engine) const {
        std::uniform_int_distribution<int> damage(min_damage_, max_damage_);
        return damage(engine);
    }
    void take_damage(int amount) { health_ = std::max(0, health_ - std::max(0, amount)); }

  protected:
    void restore(int amount) { health_ = std::min(max_health_, health_ + amount); }

  private:
    std::string name_;
    int health_;
    int max_health_;
    int min_damage_;
    int max_damage_;
};

class Hero final : public Combatant {
  public:
    Hero(std::string name, int potions) : Combatant(std::move(name), 40, 4, 9), potions_(potions) {}
    int potions() const { return potions_; }
    /// Drink a potion (+15 health); false when none are left or health is already full.
    bool drink_potion() {
        if (potions_ == 0 || health() == max_health()) {
            return false;
        }
        --potions_;
        restore(15);
        return true;
    }

  private:
    int potions_;
};

class Monster : public Combatant {
  public:
    using Combatant::Combatant;
};

/// Ogres hit hard but often miss.
class Ogre final : public Monster {
  public:
    Ogre() : Monster("Ogre", 30, 6, 12) {}
    int roll_damage(std::mt19937& engine) const override {
        std::bernoulli_distribution miss(0.3);
        return miss(engine) ? 0 : Monster::roll_damage(engine);
    }
};

enum class Action { attack, potion, flee };
enum class Outcome { ongoing, victory, defeat, fled };

class Battle {
  public:
    /// The engine is passed in: a fixed seed replays exactly the same battle.
    Battle(Hero hero, std::vector<std::unique_ptr<Monster>> monsters, std::uint32_t seed)
        : hero_(std::move(hero)), monsters_(std::move(monsters)), engine_(seed) {
        if (monsters_.empty()) {
            throw std::invalid_argument("a battle needs at least one monster");
        }
    }

    /// The hero acts, then the current monster strikes back if it is still alive.
    std::vector<std::string> play_turn(Action action) {
        if (outcome() != Outcome::ongoing) {
            throw std::logic_error("the battle is over");
        }
        std::vector<std::string> log;
        Monster& enemy = *monsters_[current_];
        switch (action) {
            case Action::attack: {
                const int damage = hero_.roll_damage(engine_);
                enemy.take_damage(damage);
                log.push_back(hero_.name() + " hits " + enemy.name() + " for " + std::to_string(damage));
                break;
            }
            case Action::potion:
                log.push_back(hero_.drink_potion() ? hero_.name() + " drinks a potion" : "no potion to drink");
                break;
            case Action::flee:
                fled_ = true;
                log.push_back(hero_.name() + " runs away");
                return log;
        }
        if (!enemy.alive()) {
            log.push_back(enemy.name() + " is defeated");
            ++current_;
            return log;
        }
        const int damage = enemy.roll_damage(engine_);
        hero_.take_damage(damage);
        log.push_back(damage == 0 ? enemy.name() + " misses"
                                  : enemy.name() + " hits back for " + std::to_string(damage));
        return log;
    }

    Outcome outcome() const {
        if (fled_) return Outcome::fled;
        if (!hero_.alive()) return Outcome::defeat;
        if (current_ >= monsters_.size()) return Outcome::victory;
        return Outcome::ongoing;
    }

    const Hero& hero() const { return hero_; }
    const Monster* current_monster() const { return current_ < monsters_.size() ? monsters_[current_].get() : nullptr; }

  private:
    Hero hero_;
    std::vector<std::unique_ptr<Monster>> monsters_;
    std::mt19937 engine_;
    std::size_t current_{0};
    bool fled_{false};
};

/// The standard dungeon: a goblin, then an ogre.
inline std::vector<std::unique_ptr<Monster>> standard_dungeon() {
    std::vector<std::unique_ptr<Monster>> monsters;
    monsters.push_back(std::make_unique<Monster>("Goblin", 18, 2, 5));
    monsters.push_back(std::make_unique<Ogre>());
    return monsters;
}

/// The interactive demo: a seeded duel driven by a/p/f commands.
inline int run(std::istream& in, std::ostream& out) {
    out << "Day 38 – Game Development with OOP\n";
    Battle battle(Hero("Aria", 2), standard_dungeon(), 2026);
    while (battle.outcome() == Outcome::ongoing) {
        const Monster* enemy = battle.current_monster();
        out << "[" << battle.hero().health() << " hp, " << battle.hero().potions() << " potions] vs " << enemy->name()
            << " (" << enemy->health() << " hp)\n";
        auto line = prompt_line(in, out, "a)ttack p)otion f)lee: ");
        if (!line) {
            break;
        }
        const Action action = *line == "p" ? Action::potion : *line == "f" ? Action::flee : Action::attack;
        for (const auto& entry : battle.play_turn(action)) {
            out << "  " << entry << '\n';
        }
    }
    const Outcome result = battle.outcome();
    out << (result == Outcome::victory  ? "Victory!\n"
            : result == Outcome::defeat ? "Defeat...\n"
            : result == Outcome::fled   ? "You escaped.\n"
                                        : "Game paused.\n");
    return 0;
}

}  // namespace cppm::day38
