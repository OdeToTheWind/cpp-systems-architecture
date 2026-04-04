#include <iostream>
#include <string>

/**
 * @brief Calculates the area of a circle.
 * 
 * @param radius Radius of the circle in cm (must be positive)
 * @return Area of the circle in cm²
 * @throws std::invalid_argument if radius is negative or zero
 */
double calculate_circle_area(double radius) {
    if (radius <= 0.0) {
        throw std::invalid_argument("Radius must be positive");
    }
    const double PI = 3.141592653589793;
    return PI * radius * radius;
}

/**
 * @brief Greets the user with a personalized message.
 * 
 * This function demonstrates proper documentation and self-documenting code.
 * 
 * @param name User's name
 * @param age User's age (optional, default 0)
 * @return Personalized greeting string
 */
std::string greet_user(const std::string& name, int age = 0) {
    if (name.empty()) {
        return "Hello, stranger!";
    }
    if (age > 0) {
        return "Hello, " + name + "! You are " + std::to_string(age) + " years old.";
    }
    return "Hello, " + name + "!";
}

int main() {
    std::cout << "=== Day 22: Documentation & Comments ===\n\n";

    std::string name;
    int age;

    std::cout << "Enter your name: ";
    std::getline(std::cin, name);

    std::cout << "Enter your age (0 to skip): ";
    std::cin >> age;

    std::cout << "\n" << greet_user(name, age) << "\n";

    double radius;
    std::cout << "\nEnter circle radius (cm): ";
    std::cin >> radius;

    try {
        double area = calculate_circle_area(radius);
        std::cout << "Circle area: " << area << " cm²\n";
    } catch (const std::invalid_argument& e) {
        std::cout << "Error: " << e.what() << "\n";
    }

    std::cout << "\nKey takeaway:\n"
              << "  * Write code that explains itself\n"
              << "  * Use comments for 'why', not 'what'\n"
              << "  * Doxygen-style comments help generate professional documentation\n";

    return 0;
}