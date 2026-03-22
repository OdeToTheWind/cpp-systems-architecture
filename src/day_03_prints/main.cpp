#include <iostream>
#include <string>
#include <iomanip>    // setw, setprecision, left, fixed
#include <limits>     // numeric_limits
#include <cctype>     // tolower

int main() {
    std::cout << "=== Day 03: Input & Output with cin / cout ===\n\n";

    // ── Example 1: Simple numeric input ────────────────────────────────────────
    int age;
    std::cout << "Enter your age: ";
    std::cin >> age;

    if (std::cin.fail() || age <= 0) {
        std::cout << "Invalid age! Must be a positive integer.\n";
        return 1;
    }

    std::cout << "You are " << age << " years old.\n\n";

    // ── Fix leftover newline before getline ────────────────────────────────────
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

    // ── Example 2: Full name with spaces ───────────────────────────────────────
    std::string fullName;
    std::cout << "Enter your full name: ";
    std::getline(std::cin, fullName);

    std::cout << "Hello, " << fullName << "!\n\n";

    // ── New: Gender input ──────────────────────────────────────────────────────
    char genderInput;
    std::string gender;
    std::cout << "Enter your gender (M/F/O for Other): ";
    std::cin >> genderInput;

    // Safely convert to lowercase (cast input to unsigned char first)
    genderInput = static_cast<char>(std::tolower(static_cast<unsigned char>(genderInput)));

    if (genderInput == 'm')      gender = "Male";
    else if (genderInput == 'f') gender = "Female";
    else if (genderInput == 'o') gender = "Other";
    else {
        std::cout << "Invalid gender input. Using 'Other'.\n";
        gender = "Other";
    }

    std::cout << "Gender recognized: " << gender << "\n\n";

    // ── Clean up after cin >> ──────────────────────────────────────────────────
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

    // ── Example 3: Height & Weight + BMI calculation ───────────────────────────
    double height_cm;
    double weight_kg;

    std::cout << "Enter your height (cm): ";
    std::cin >> height_cm;

    std::cout << "Enter your weight (kg): ";
    std::cin >> weight_kg;

    if (height_cm <= 0 || weight_kg <= 0) {
        std::cout << "Invalid height or weight (must be positive).\n";
        return 1;
    }

    double bmi = weight_kg / ((height_cm / 100.0) * (height_cm / 100.0));

    // ── Formatted output ───────────────────────────────────────────────────────
    std::cout << "\n--- Your Profile ---\n"
              << std::left << std::setw(15) << "Name:" << fullName << "\n"
              << std::setw(15) << "Gender:" << gender << "\n"
              << std::setw(15) << "Age:" << age << " years\n"
              << std::setw(15) << "Height:" << std::fixed << std::setprecision(1)
              << height_cm << " cm\n"
              << std::setw(15) << "Weight:" << weight_kg << " kg\n"
              << std::setw(15) << "BMI:" << std::setprecision(2) << bmi << "\n";

    // BMI category (using commonly referenced adult ranges)
    // Note: WHO uses same ranges for both sexes, but some sources adjust slightly for females
    std::cout << "Category: ";
    if (bmi < 18.5) {
        std::cout << "Underweight\n";
    } else if (bmi < 25.0) {
        std::cout << "Normal weight\n";
    } else if (bmi < 30.0) {
        std::cout << "Overweight\n";
    } else {
        std::cout << "Obese\n";
    }

    // Optional: gender-specific comment (example only – not medical advice)
    if (gender == "Female" && (bmi >= 25.0 && bmi < 30.0)) {
        std::cout << "(Note: Some health guidelines consider BMI 25–27 acceptable for women)\n";
    }

    return 0;
}