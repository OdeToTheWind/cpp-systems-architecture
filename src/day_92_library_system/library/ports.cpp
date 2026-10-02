#include "library/ports.hpp"

namespace cppm::day92 {

int InMemoryRepository::add_loan(Loan loan) {
    loan.id = next_loan_++;
    loans_[loan.id] = loan;
    return loan.id;
}

std::vector<Loan> InMemoryRepository::loans_of_book(const std::string& isbn) const {
    std::vector<Loan> out;
    for (const auto& [id, loan] : loans_) {
        if (loan.isbn == isbn) out.push_back(loan);
    }
    return out;
}

std::vector<Loan> InMemoryRepository::loans_of_member(const std::string& member) const {
    std::vector<Loan> out;
    for (const auto& [id, loan] : loans_) {
        if (loan.member == member) out.push_back(loan);
    }
    return out;
}

}  // namespace cppm::day92
