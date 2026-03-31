#include <iostream>
#include <string>

// 1. Basic function with positional arguments
double calculate_rectangle_area(double length, double width) {
    return length * width;
}

// 2. Function with default arguments (simulates optional parameters)
double calculate_rectangle_perimeter(double length, double width = 0.0) {
    if (width == 0.0) width = length;  // square if only length given
    return 2 * (length + width);
}

// 3. Function overloading (different ways to call)
void print_person_info(const std::string& name, int age) {
    std::cout << "Name: " << name << ", Age: " << age << " years\n";
}

void print_person_info(const std::string& name, int age, const std::string& city) {
    std::cout << "Name: " << name << ", Age: " << age 
              << ", City: " << city << "\n";
}

// 4. Named Parameter Idiom using struct (best for many optional params)
struct PersonConfig {
    std::string name = "Unknown";
    int age = 0;
    std::string city = "Not specified";
    double height_cm = 0.0;
};

void create_person(const PersonConfig& config) {
    std::cout << "Created person:\n";
    std::cout << "  Name   : " << config.name << "\n";
    std::cout << "  Age    : " << config.age << "\n";
    std::cout << "  City   : " << config.city << "\n";
    std::cout << "  Height : " << config.height_cm << " cm\n\n";
}

int main() {
    std::cout << "=== Day 18: Positional vs Keyword Arguments in C++ ===\n\n";

    // Positional arguments
    double length, width;
    std::cout << "Enter rectangle length and width: ";
    std::cin >> length >> width;
    std::cout << "Area (positional): " << calculate_rectangle_area(length, width) << "\n\n";

    // Default arguments
    double side;
    std::cout << "Enter side length for square: ";
    std::cin >> side;
    std::cout << "Perimeter (with default): " << calculate_rectangle_perimeter(side) << "\n";
    std::cout << "Perimeter (with width)  : " << calculate_rectangle_perimeter(side, side * 1.5) << "\n\n";

    // Function overloading
    std::string name;
    int age;
    std::string city;

    std::cout << "Enter name and age: ";
    std::cin.ignore();
    std::getline(std::cin, name);
    std::cin >> age;
    std::cin.ignore();

    print_person_info(name, age);

    std::cout << "Enter city (or press Enter to skip): ";
    std::getline(std::cin, city);
    if (!city.empty()) {
        print_person_info(name, age, city);
    }
    std::cout << "\n";

    // Named Parameter Idiom (closest to keyword arguments)
    std::cout << "=== Named Parameter Idiom (Best C++ alternative) ===\n";
    PersonConfig config;

    std::cout << "Enter name: ";
    std::getline(std::cin, config.name);

    std::cout << "Enter age: ";
    std::cin >> config.age;

    std::cin.ignore();
    std::cout << "Enter city (optional): ";
    std::getline(std::cin, config.city);

    std::cout << "Enter height in cm (optional): ";
    std::cin >> config.height_cm;

    create_person(config);

    std::cout << "Key takeaway:\n"
              << "  * C++ uses positional arguments by default\n"
              << "  * Default arguments and overloading simulate optional parameters\n"
              << "  * Named Parameter Idiom (struct) is the cleanest way for many options\n"
              << "  * Clear function signatures prevent confusion\n";

    return 0;
}