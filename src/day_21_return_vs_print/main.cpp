#include <iostream>
#include <string>

// Bad design: mixes calculation and printing
void bad_calculator(double a, double b, char op) {
    if (op == '+') {
        std::cout << a << " + " << b << " = " << (a + b) << "\n";
    } else if (op == '-') {
        std::cout << a << " - " << b << " = " << (a - b) << "\n";
    } else {
        std::cout << "Unsupported operator\n";
    }
}

// Good design: pure function that only computes and returns
double good_calculator(double a, double b, char op) {
    if (op == '+') return a + b;
    if (op == '-') return a - b;
    if (op == '*') return a * b;
    if (op == '/' && b != 0.0) return a / b;
    throw std::invalid_argument("Invalid operator or division by zero");
}

int main() {
    std::cout << "=== Day 21: Return vs Print ===\n\n";

    double a, b;
    char op;

    std::cout << "Enter two numbers: ";
    std::cin >> a >> b;
    std::cout << "Enter operator (+, -, *, /): ";
    std::cin >> op;

    std::cout << "\n--- Bad Design (mixed return + print) ---\n";
    bad_calculator(a, b, op);

    std::cout << "\n--- Good Design (pure function + separate output) ---\n";
    try {
        double result = good_calculator(a, b, op);
        std::cout << a << " " << op << " " << b << " = " << result << "\n";
    } catch (const std::invalid_argument& e) {
        std::cout << "Error: " << e.what() << "\n";
    }

    std::cout << "\nKey takeaway:\n"
              << "  * Functions should either compute AND return a value, OR perform I/O — not both\n"
              << "  * Pure functions are easier to test, reuse, and reason about\n"
              << "  * Let the caller decide whether to print, save, or use the result\n";

    return 0;
}