#include <iostream>
#include <vector>
#include <map>
#include <string>
#include <iomanip>

int main() {
    std::cout << "=== Day 17: Vectors and Maps (Lists & Dictionaries) ===\n\n";

    // ── Part 1: std::map - Student Score Management ────────────────────────────
    std::map<std::string, double> student_scores;
    std::string name;
    double score;
    char choice;

    std::cout << "=== Student Score Manager ===\n";

    do {
        std::cout << "\n1. Add/Update student score\n";
        std::cout << "2. View all scores\n";
        std::cout << "3. Search student\n";
        std::cout << "4. Exit\n";
        std::cout << "Choice: ";
        std::cin >> choice;
        std::cin.ignore();

        if (choice == '1') {
            std::cout << "Enter student name: ";
            std::getline(std::cin, name);
            std::cout << "Enter score (0-100): ";
            std::cin >> score;

            student_scores[name] = score;
            std::cout << "Score saved for " << name << "\n";
        } 
        else if (choice == '2') {
            if (student_scores.empty()) {
                std::cout << "No students added yet.\n";
            } else {
                std::cout << "\nStudent Scores:\n";
                std::cout << std::left << std::setw(20) << "Name" 
                          << "Score\n";
                std::cout << std::string(30, '-') << "\n";
                for (const auto& pair : student_scores) {
                    std::cout << std::left << std::setw(20) << pair.first 
                              << pair.second << "\n";
                }
            }
        } 
        else if (choice == '3') {
            std::cout << "Enter student name to search: ";
            std::getline(std::cin, name);

            auto it = student_scores.find(name);
            if (it != student_scores.end()) {
                std::cout << name << "'s score: " << it->second << "\n";
            } else {
                std::cout << "Student not found.\n";
            }
        }

    } while (choice != '4');

    std::cout << "\n";

    // ── Part 2: std::vector - Shopping List ─────────────────────────────────────
    std::vector<std::string> shopping_list;
    std::string item;

    std::cout << "=== Shopping List Manager ===\n";
    std::cin.ignore(); // clear any leftover newline

    while (true) {
        std::cout << "\nEnter item to add (or type 'done' to finish): ";
        std::getline(std::cin, item);

        if (item == "done" || item == "Done") {
            break;
        }

        if (!item.empty()) {
            shopping_list.push_back(item);
            std::cout << "Added: " << item << "\n";
        }
    }

    std::cout << "\nYour Shopping List (" << shopping_list.size() << " items):\n";
    for (size_t i = 0; i < shopping_list.size(); ++i) {
        std::cout << (i + 1) << ". " << shopping_list[i] << "\n";
    }

    std::cout << "\nKey takeaway:\n"
              << "  * Use std::vector when order matters and you need fast indexing\n"
              << "  * Use std::map when you need fast lookup by meaningful key\n"
              << "  * Both support range-based for loops\n"
              << "  * These two containers cover most real-world data needs\n";

    return 0;
}