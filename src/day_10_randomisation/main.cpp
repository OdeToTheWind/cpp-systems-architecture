#include <iostream>
#include <random>
#include <string>
#include <limits>

int main() {
    std::cout << "=== Day 10: Randomisation using <random> ===\n\n";

    // Setup modern random engine
    std::random_device rd;
    std::mt19937 gen(rd());   // Mersenne Twister - high quality

    // ── Example 1: Random integer in user-defined range ────────────────────────
    int min_val, max_val, count;
    std::cout << "Enter minimum value: ";
    std::cin >> min_val;
    std::cout << "Enter maximum value: ";
    std::cin >> max_val;
    std::cout << "How many random numbers to generate? ";
    std::cin >> count;

    std::uniform_int_distribution<int> int_dist(min_val, max_val);

    std::cout << "\n" << count << " random integers between " 
              << min_val << " and " << max_val << ":\n";
    for (int i = 0; i < count; ++i) {
        std::cout << int_dist(gen) << " ";
    }
    std::cout << "\n\n";

    // ── Example 2: Random floating-point numbers ───────────────────────────────
    double min_d, max_d;
    std::cout << "Enter min and max for floating-point range: ";
    std::cin >> min_d >> max_d;

    std::uniform_real_distribution<double> real_dist(min_d, max_d);

    std::cout << "5 random doubles:\n";
    for (int i = 0; i < 5; ++i) {
        std::cout << real_dist(gen) << " ";
    }
    std::cout << "\n\n";

    // ── Example 3: Coin flip simulator (bernoulli) ─────────────────────────────
    double probability_heads;
    std::cout << "Enter probability of heads (0.0 - 1.0): ";
    std::cin >> probability_heads;

    std::bernoulli_distribution coin(probability_heads);

    int flips;
    std::cout << "How many coin flips? ";
    std::cin >> flips;

    int heads = 0;
    for (int i = 0; i < flips; ++i) {
        if (coin(gen)) heads++;
    }

    std::cout << "Results: Heads = " << heads 
              << ", Tails = " << (flips - heads) 
              << " (" << (static_cast<double>(heads) / flips * 100) << "% heads)\n\n";

    // ── Bonus: Simple number guessing game ─────────────────────────────────────
    std::cout << "--- Number Guessing Game ---\n";
    int low = 1, high = 100;
    std::uniform_int_distribution<int> secret_dist(low, high);
    int secret = secret_dist(gen);

    int guess;
    int attempts = 0;
    std::cout << "Guess a number between " << low << " and " << high << "!\n";

    while (true) {
        std::cout << "Your guess: ";
        if (!(std::cin >> guess)) {
            std::cout << "Invalid input.\n";
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            continue;
        }
        attempts++;

        if (guess == secret) {
            std::cout << "Congratulations! You guessed it in " << attempts << " attempts!\n";
            break;
        } else if (guess < secret) {
            std::cout << "Too low! Try higher.\n";
        } else {
            std::cout << "Too high! Try lower.\n";
        }
    }

    std::cout << "\nKey takeaway:\n"
              << "  * Always use <random> instead of rand()/srand()\n"
              << "  * Separate engine from distribution\n"
              << "  * std::mt19937 + uniform distributions are the standard choice\n";

    return 0;
}