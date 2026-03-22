#include <iostream>
#include <iomanip>     // setprecision, fixed
#include <cmath>       // sqrt, pow, hypot, round, abs
#include <limits>      // numeric_limits

int main() {
    std::cout << "=== Day 05: Mathematical Operations in C++ ===\n\n";

    // ── Interactive basic arithmetic ────────────────────────────────────────────
    int a, b;
    std::cout << "Enter two integers (a b): ";
    while (!(std::cin >> a >> b)) {
        std::cout << "Invalid input. Please enter two integers separated by space: ";
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    }

    std::cout << "a = " << a << ", b = " << b << "\n";
    std::cout << "a + b  = " << (a + b) << "\n";
    std::cout << "a - b  = " << (a - b) << "\n";
    std::cout << "a * b  = " << (a * b) << "\n";

    if (b != 0) {
        std::cout << "a / b  = " << (a / b) << " (integer division)\n";
        std::cout << "a % b  = " << (a % b) << " (remainder)\n";
    } else {
        std::cout << "Division by zero avoided.\n";
    }
    std::cout << "\n";

    // ── Interactive compound assignment demo ────────────────────────────────────
    double price;
    std::cout << "Enter original price: ";
    while (!(std::cin >> price)) {
        std::cout << "Invalid input. Please enter a number: ";
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    }

    double tax_rate_percent, discount_amount;
    std::cout << "Enter tax rate (in %): ";
    while (!(std::cin >> tax_rate_percent)) {
        std::cout << "Invalid input. Please enter a number: ";
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    }
    std::cout << "Enter discount amount: ";
    while (!(std::cin >> discount_amount)) {
        std::cout << "Invalid input. Please enter a number: ";
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    }

    price += discount_amount;                // wait — usually discount first
    price *= (1.0 + tax_rate_percent / 100.0);  // then tax

    std::cout << "Final price after discount & tax: " 
              << std::fixed << std::setprecision(2) << price << "\n\n";

    // ── Interactive increment / decrement ───────────────────────────────────────
    int counter;
    std::cout << "Enter starting counter value: ";
    while (!(std::cin >> counter)) {
        std::cout << "Invalid input. Please enter an integer: ";
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    }

    std::cout << "Original counter: " << counter << "\n";
    std::cout << "++counter (pre):  " << ++counter << "\n";
    std::cout << "counter now:      " << counter << "\n";
    std::cout << "counter++ (post): " << counter++ << "\n";
    std::cout << "counter now:      " << counter << "\n\n";

    // ── Interactive <cmath> functions ───────────────────────────────────────────
    double x;
    std::cout << "Enter a number for math functions: ";
    while (!(std::cin >> x)) {
        std::cout << "Invalid input. Please enter a number: ";
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    }

    std::cout << "sqrt(" << x << ")     = " << std::sqrt(x) << "\n";
    std::cout << "pow(" << x << ", 2)   = " << std::pow(x, 2) << "\n";
    std::cout << "round(" << x << ")    = " << std::round(x) << "\n";
    std::cout << "abs(" << x << ")      = " << std::abs(x) << "\n\n";

    // ── Interactive circle calculator ───────────────────────────────────────────
    double radius;
    std::cout << "Enter circle radius (cm): ";
    while (!(std::cin >> radius)) {
        std::cout << "Invalid input. Please enter a positive number: ";
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    }

    if (radius <= 0) {
        std::cout << "Radius must be positive!\n";
        return 1;
    }

    const double PI = 3.141592653589793;
    double area = PI * std::pow(radius, 2);
    double circumference = 2 * PI * radius;
    double diameter = 2 * radius;

    std::cout << std::fixed << std::setprecision(2);
    std::cout << "\nCircle stats:\n"
              << "Radius        : " << radius << " cm\n"
              << "Diameter      : " << diameter << " cm\n"
              << "Area          : " << area << " cm²\n"
              << "Circumference : " << circumference << " cm\n";

    return 0;
}