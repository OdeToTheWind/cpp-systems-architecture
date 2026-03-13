#include <iostream>
#include <string>

int main() {
    std::cout << "=== Day 1: Variables & Basic I/O ===\n\n";

    // 1. Basic variable declarations
    int age = 25;
    double height = 175.5;
    char grade = 'A';
    bool isStudent = true;
    std::string name = "SpireLab";

    std::cout << "Name    : " << name << "\n";
    std::cout << "Age     : " << age << " years\n";
    std::cout << "Height  : " << height << " cm\n";
    std::cout << "Grade   : " << grade << "\n";
    std::cout << "Student?: " << (isStudent ? "Yes" : "No") << "\n\n";

    // 2. User input version
    std::string userName;
    int userAge;
    double userSalary;

    std::cout << "Enter your name: ";
    std::getline(std::cin, userName);           // better than cin >> for names with spaces

    std::cout << "Enter your age: ";
    std::cin >> userAge;

    std::cout << "Enter your expected salary (LPA): ";
    std::cin >> userSalary;

    std::cout << "\nHello " << userName << "! "
              << "At " << userAge << " you want " << userSalary << " LPA? "
              << "Keep grinding!\n";

    return 0;
}