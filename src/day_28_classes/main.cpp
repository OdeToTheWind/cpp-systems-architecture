#include <iostream>
#include <string>

class BankAccount {
private:
    std::string owner;
    double balance;

public:
    BankAccount(const std::string& name, double initial_balance = 0.0)
        : owner(name), balance(initial_balance) {}

    void deposit(double amount) {
        if (amount > 0) balance += amount;
    }

    bool withdraw(double amount) {
        if (amount > 0 && amount <= balance) {
            balance -= amount;
            return true;
        }
        return false;
    }

    void show_balance() const {
        std::cout << owner << "'s balance: $" << balance << "\n";
    }
};

int main() {
    std::cout << "=== Day 28: Creating Classes ===\n\n";

    std::string name;
    std::cout << "Enter account holder name: ";
    std::getline(std::cin, name);

    BankAccount account(name, 1000.0);
    account.show_balance();

    double amount;
    std::cout << "Enter deposit amount: ";
    std::cin >> amount;
    account.deposit(amount);
    account.show_balance();

    return 0;
}