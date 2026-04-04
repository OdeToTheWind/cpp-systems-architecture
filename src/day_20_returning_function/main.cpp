#include <iostream>
#include <functional>
#include <string>

// Returns a function that performs a mathematical operation
std::function<double(double, double)> get_operation(const std::string& op) {
    if (op == "add" || op == "+") {
        return [](double a, double b) { return a + b; };
    } 
    else if (op == "subtract" || op == "-") {
        return [](double a, double b) { return a - b; };
    } 
    else if (op == "multiply" || op == "*") {
        return [](double a, double b) { return a * b; };
    } 
    else if (op == "divide" || op == "/") {
        return [](double a, double b) {
            if (b == 0.0) throw std::runtime_error("Division by zero!");
            return a / b;
        };
    } 
    else {
        // Default: addition
        return [](double a, double b) { return a + b; };
    }
}

// Returns a pricing function based on user choice
std::function<double(double)> get_pricing_strategy(const std::string& type) {
    if (type == "normal") {
        return [](double price) { return price; };
    } 
    else if (type == "discount") {
        return [](double price) { return price * 0.9; };        // 10% discount
    } 
    else if (type == "tax") {
        return [](double price) { return price * 1.18; };       // 18% tax
    } 
    else {
        return [](double price) { return price; };
    }
}

int main() {
    std::cout << "=== Day 20: Returning Functions ===\n\n";

    // ── Example 1: Dynamic math operation ───────────────────────────────────────
    std::string operation;
    std::cout << "Enter operation (add, subtract, multiply, divide): ";
    std::cin >> operation;

    auto math_func = get_operation(operation);

    double x, y;
    std::cout << "Enter two numbers: ";
    std::cin >> x >> y;

    try {
        double result = math_func(x, y);
        std::cout << "Result of " << operation << ": " << result << "\n\n";
    } catch (const std::runtime_error& e) {
        std::cout << "Error: " << e.what() << "\n\n";
    }

    // ── Example 2: Pricing strategy using returned function ─────────────────────
    std::string strategy;
    std::cout << "Enter pricing strategy (normal, discount, tax): ";
    std::cin >> strategy;

    auto price_func = get_pricing_strategy(strategy);

    double base_price;
    std::cout << "Enter base price: ";
    std::cin >> base_price;

    double final_price = price_func(base_price);
    std::cout << "Final price after " << strategy << " strategy: " 
              << final_price << "\n\n";

    // ── Example 3: Lambda returned and used immediately ─────────────────────────
    std::cout << "=== Bonus: Lambda as returned function ===\n";
    auto create_multiplier = [](int factor) {
        return [factor](int value) { return value * factor; };
    };

    int factor, value;
    std::cout << "Enter multiplier factor: ";
    std::cin >> factor;
    std::cout << "Enter value to multiply: ";
    std::cin >> value;

    auto multiplier = create_multiplier(factor);
    std::cout << value << " * " << factor << " = " << multiplier(value) << "\n";

    std::cout << "\nKey takeaway:\n"
              << "  * `std::function` is the most flexible way to return functions\n"
              << "  * Lambdas + `std::function` enable powerful higher-order programming\n"
              << "  * This pattern is the foundation of callbacks and strategy design\n";

    return 0;
}