#include <iostream>
#include <string>
#include <limits>

// ── Good naming examples ────────────────────────────────────────────────────────

// Clear, descriptive, snake_case (Google/LLVM style preferred in this project)
int user_age_in_years = 0;
double body_mass_index = 0.0;
std::string full_name_of_person;
bool is_adult = false;
int number_of_attempts_remaining = 3;

// Constants in UPPER_SNAKE_CASE
constexpr double MAXIMUM_ALLOWED_BMI = 30.0;
constexpr int MINIMUM_AGE_TO_VOTE = 18;

// ── Acceptable short names in limited scope ────────────────────────────────────
void print_summary(int age, double bmi) {
    // i is OK in very short loops / counters
    for (int i = 0; i < 3; ++i) {
        std::cout << ".";
    }
    std::cout << "\n";

    // Short but contextually clear
    std::cout << "Age: " << age << " | BMI: " << bmi << "\n";
}

// ── Bad naming examples (commented out – do NOT use) ────────────────────────────
// int x;                          // meaningless
// double tempVar123;              // cryptic
// bool flag;                      // doesn't say what it flags
// int userAgeInYearsOld;          // redundant type info
// std::string strFullNameOfTheUser; // Hungarian + too long
// int _private_counter;           // leading underscore in global scope = undefined behavior risk

int main() {
    std::cout << "=== Day 04: Variable Naming Conventions ===\n\n";

    std::cout << "Enter your age: ";
    std::cin >> user_age_in_years;

    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

    std::cout << "Enter your full name: ";
    std::getline(std::cin, full_name_of_person);

    std::cout << "Enter your weight (kg): ";
    double weight_in_kg;
    std::cin >> weight_in_kg;

    std::cout << "Enter your height (cm): ";
    double height_in_cm;
    std::cin >> height_in_cm;

    // Calculate BMI with intention-revealing names
    double height_in_meters = height_in_cm / 100.0;
    body_mass_index = weight_in_kg / (height_in_meters * height_in_meters);

    is_adult = (user_age_in_years >= MINIMUM_AGE_TO_VOTE);

    std::cout << "\n--- Summary ---\n";
    print_summary(user_age_in_years, body_mass_index);

    std::cout << "Full name       : " << full_name_of_person << "\n";
    std::cout << "Adult?          : " << (is_adult ? "Yes" : "No") << "\n";
    std::cout << "BMI status      : "
              << (body_mass_index > MAXIMUM_ALLOWED_BMI ? "Above limit" : "OK") << "\n";

    std::cout << "\nGood naming makes code self-documenting.\n"
              << "Bad naming forces readers to decode intent.\n";

    return 0;
}