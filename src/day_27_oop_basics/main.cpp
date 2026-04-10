#include <iostream>
#include <string>

// Simple class demonstrating Encapsulation
class Player {
private:
    std::string name;   // hidden from outside
    int health;         // hidden from outside

public:
    // Constructor
    Player(const std::string& player_name, int starting_health = 100)
        : name(player_name), health(starting_health) {}

    // Public method to modify health safely
    void take_damage(int damage) {
        if (damage > 0) {
            health -= damage;
            if (health < 0) health = 0;
        }
    }

    // Public method to heal
    void heal(int amount) {
        if (amount > 0) {
            health += amount;
        }
    }

    // Public method to display status
    void display_status() const {
        std::cout << "Player Name : " << name << "\n";
        std::cout << "Health      : " << health << "\n\n";
    }
};

int main() {
    std::cout << "=== Day 27: Object-Oriented Programming Basics ===\n\n";

    std::string player_name;
    std::cout << "Enter player name: ";
    std::getline(std::cin, player_name);

    Player hero(player_name, 100);

    std::cout << "Initial Status:\n";
    hero.display_status();

    int damage;
    std::cout << "Enter damage amount: ";
    std::cin >> damage;

    hero.take_damage(damage);

    std::cout << "After taking damage:\n";
    hero.display_status();

    std::cout << "Explanation:\n";
    std::cout << "* Encapsulation means wrapping data (name, health) and methods together.\n";
    std::cout << "* The 'private' section hides internal data from direct access.\n";
    std::cout << "* We can only modify health safely through public methods like take_damage().\n";
    std::cout << "* This prevents invalid states (e.g., negative health without checks).\n";

    return 0;
}