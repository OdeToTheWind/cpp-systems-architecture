#include <iostream>
#include <string>
#include <limits>
#include <random>

int main() {
    std::cout << "=== Day 15: While and Do-While Loops ===\n\n";

    // ── Example 1: Number Guessing Game with while loop ────────────────────────
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<int> dist(1, 100);

    int secret = dist(gen);
    int guess;
    int attempts = 0;
    const int max_attempts = 10;

    std::cout << "Guess the secret number between 1 and 100!\n";
    std::cout << "(You have " << max_attempts << " attempts)\n\n";

    while (attempts < max_attempts) {
        std::cout << "Attempt " << (attempts + 1) << ": Your guess: ";
        if (!(std::cin >> guess)) {
            std::cout << "Invalid input. Try again.\n";
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            continue;
        }

        attempts++;

        if (guess == secret) {
            std::cout << "Congratulations! You guessed it in " << attempts << " attempts!\n";
            break;
        } 
        else if (guess < secret) {
            std::cout << "Too low!\n";
        } 
        else {
            std::cout << "Too high!\n";
        }
    }

    if (attempts == max_attempts && guess != secret) {
        std::cout << "Game over! The secret number was " << secret << ".\n";
    }
    std::cout << "\n";

    // ── Example 2: Interactive Menu using do-while ─────────────────────────────
    int choice;
    bool running = true;

    std::cout << "=== Simple Calculator Menu ===\n";

    do {
        std::cout << "\n1. Add two numbers\n";
        std::cout << "2. Multiply two numbers\n";
        std::cout << "3. Exit\n";
        std::cout << "Enter your choice: ";
        std::cin >> choice;

        if (choice == 3) {
            std::cout << "Goodbye!\n";
            running = false;
            continue;
        }

        double a, b;
        std::cout << "Enter two numbers: ";
        std::cin >> a >> b;

        if (choice == 1) {
            std::cout << "Result: " << (a + b) << "\n";
        } 
        else if (choice == 2) {
            std::cout << "Result: " << (a * b) << "\n";
        } 
        else {
            std::cout << "Invalid choice! Please try again.\n";
        }

    } while (running);

    std::cout << "\nKey takeaway:\n"
              << "  * Use while when you may skip the loop entirely\n"
              << "  * Use do-while when the loop must run at least once (e.g., menus)\n"
              << "  * Always update the loop condition inside the body\n"
              << "  * Combine loops with if/else for powerful interactive programs\n";

    return 0;
}