#include <iostream>
#include <string>

// Helper function: Power (base^exponent)
double power(double base, int exponent) {
    if (exponent < 0) {
        throw std::invalid_argument("Negative exponent not supported in this simple version");
    }
    double result = 1.0;
    for (int i = 0; i < exponent; ++i) {
        result *= base;
    }
    return result;
}

// Helper function: Calculate BMI
double calculate_bmi(double weight_kg, double height_cm) {
    if (weight_kg <= 0 || height_cm <= 0) {
        throw std::invalid_argument("Weight and height must be positive");
    }
    double height_m = height_cm / 100.0;
    return weight_kg / (height_m * height_m);
}

// Pass-by-value vs Pass-by-reference example
void swap_by_value(int a, int b) {
    int temp = a;
    a = b;
    b = temp;
}

void swap_by_reference(int& a, int& b) {
    int temp = a;
    a = b;
    b = temp;
}

// Function overloading
std::string describe_number(int n) {
    return (n % 2 == 0) ? "Even" : "Odd";
}

std::string describe_number(double n) {
    return (n >= 0) ? "Non-negative" : "Negative";
}

int main() {
    std::cout << "=== Day 12: Functions in C++ ===\n\n";

    // ── Power calculation ───────────────────────────────────────────────────────
    double base;
    int exp;
    std::cout << "Enter base and exponent: ";
    std::cin >> base >> exp;

    try {
        double result = power(base, exp);
        std::cout << base << " ^ " << exp << " = " << result << "\n\n";
    } catch (const std::invalid_argument& e) {
        std::cout << "Error: " << e.what() << "\n\n";
    }

    // ── BMI Calculator using function ───────────────────────────────────────────
    double weight, height;
    std::cout << "Enter weight (kg) and height (cm): ";
    std::cin >> weight >> height;

    try {
        double bmi = calculate_bmi(weight, height);
        std::cout << "Your BMI is: " << bmi << "\n";
        if (bmi < 18.5)      std::cout << "Category: Underweight\n";
        else if (bmi < 25)   std::cout << "Category: Normal\n";
        else if (bmi < 30)   std::cout << "Category: Overweight\n";
        else                 std::cout << "Category: Obese\n";
    } catch (const std::invalid_argument& e) {
        std::cout << "Error: " << e.what() << "\n";
    }

    std::cout << "\n";

    // ── Pass-by-value vs Pass-by-reference ──────────────────────────────────────
    int x = 10, y = 20;
    std::cout << "Before swap: x=" << x << ", y=" << y << "\n";

    swap_by_value(x, y);
    std::cout << "After swap_by_value: x=" << x << ", y=" << y << " (no change)\n";

    swap_by_reference(x, y);
    std::cout << "After swap_by_reference: x=" << x << ", y=" << y << "\n\n";

    // ── Function overloading ────────────────────────────────────────────────────
    int num_int;
    double num_double;
    std::cout << "Enter an integer: ";
    std::cin >> num_int;
    std::cout << "Enter a decimal number: ";
    std::cin >> num_double;

    std::cout << num_int << " is " << describe_number(num_int) << "\n";
    std::cout << num_double << " is " << describe_number(num_double) << "\n";

    std::cout << "\nKey takeaway:\n"
              << "  * Functions make code reusable and readable\n"
              << "  * Prefer const& for large objects you don't modify\n"
              << "  * Use meaningful names and single responsibility\n";

    return 0;
}