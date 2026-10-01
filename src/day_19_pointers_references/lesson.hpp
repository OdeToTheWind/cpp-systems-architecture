/**
 * @file
 * Day 19 – Pointers and References.
 *
 * Scenario: a *hospital ward bed board*. Each bed is a slot in a fixed array; the board
 * hands out pointers to free beds (or nullptr when the ward is full), moves patients between
 * beds through references, and only ever reads through const pointers when it reports.
 *
 * Deliverables (syllabus):
 * - Address-of and dereference
 * - References
 * - Pass by pointer vs reference
 * - nullptr checks
 * - Const correctness
 */
#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <istream>
#include <ostream>
#include <sstream>
#include <string>
#include <vector>

#include "cppm/lesson.hpp"

namespace cppm::day19 {

inline constexpr Deliverable DELIVERABLES[] = {
    {"& takes an address, * dereferences it, a reference is an alias", "address_basics"},
    {"returning a pointer that may be nullptr", "Ward::find_free_bed"},
    {"a nullable pointer parameter checked against nullptr", "admit"},
    {"reference parameters for objects that must exist", "transfer"},
    {"const Bed* for read-only access", "Ward::find_patient"},
    {"walking an array with pointer arithmetic", "count_occupied"},
    {"why pointers into a vector can dangle after push_back", "vector_storage_moved"},
};

/// What address_basics observed.
struct AddressFacts {
    int value_after_write_through_pointer;
    int value_after_write_through_reference;
    bool pointer_holds_address_of_variable;
};

/// `int* p = &x;` stores x's address; `*p = 5` writes x; `int& r = x;` is another name for x.
inline AddressFacts address_basics() {
    int beds_in_use = 1;
    int* pointer = &beds_in_use;  // address-of
    *pointer = 5;                 // dereference and write
    const int after_pointer = beds_in_use;
    int& alias = beds_in_use;  // a reference must be bound at once and can never be re-seated
    alias += 2;
    return {after_pointer, beds_in_use, pointer == &beds_in_use};
}

struct Bed {
    int number{0};
    std::string patient;  // empty = free
    bool occupied() const { return !patient.empty(); }
};

/// A ward of eight beds, stored in a std::array so their addresses never change.
class Ward {
public:
    Ward() {
        for (std::size_t i = 0; i < beds_.size(); ++i) {
            beds_[i].number = static_cast<int>(i) + 1;
        }
    }

    /// The first free bed, or nullptr when the ward is full – a pointer can say "nothing".
    Bed* find_free_bed() {
        for (Bed& bed : beds_) {
            if (!bed.occupied()) {
                return &bed;
            }
        }
        return nullptr;
    }

    /// Read-only lookup: the const member function can only hand out pointers to const.
    const Bed* find_patient(const std::string& name) const {
        for (const Bed& bed : beds_) {
            if (bed.patient == name) {
                return &bed;
            }
        }
        return nullptr;
    }

    Bed* bed_number(int number) {
        return (number >= 1 && number <= static_cast<int>(beds_.size())) ? &beds_[static_cast<std::size_t>(number) - 1]
                                                                          : nullptr;
    }

    const Bed* begin() const { return beds_.data(); }
    const Bed* end() const { return beds_.data() + beds_.size(); }

private:
    std::array<Bed, 8> beds_{};
};

/// Admit @p patient into @p bed. The pointer may be null (ward full), so it is checked first.
inline bool admit(Bed* bed, const std::string& patient) {
    if (bed == nullptr || bed->occupied() || patient.empty()) {
        return false;
    }
    bed->patient = patient;
    return true;
}

/// Move the patient from @p from to @p to. References: both beds must exist, no null checks needed.
inline bool transfer(Bed& from, Bed& to) {
    if (!from.occupied() || to.occupied() || &from == &to) {
        return false;
    }
    to.patient = std::move(from.patient);
    from.patient.clear();
    return true;
}

/// Count occupied beds in [first, last) by advancing a pointer to const.
inline int count_occupied(const Bed* first, const Bed* last) {
    int count = 0;
    for (const Bed* p = first; p != last; ++p) {  // ++p moves to the next Bed in the array
        if (p->occupied()) {
            ++count;
        }
    }
    return count;
}

/// True if push_back moved the vector's storage – every pointer into the old storage then dangles.
inline bool vector_storage_moved() {
    std::vector<Bed> overflow_beds(1);
    overflow_beds.shrink_to_fit();
    const auto before = reinterpret_cast<std::uintptr_t>(overflow_beds.data());
    overflow_beds.push_back(Bed{});  // capacity was 1, so the elements must be relocated
    const auto after = reinterpret_cast<std::uintptr_t>(overflow_beds.data());
    return before != after;
}

/// The interactive demo: admit <name> | move <from> <to> | where <name> | board.
inline int run(std::istream& in, std::ostream& out) {
    out << "Day 19 – Pointers and References\n";
    const auto facts = address_basics();
    out << "  write through pointer -> " << facts.value_after_write_through_pointer << ", through reference -> "
        << facts.value_after_write_through_reference << '\n';
    Ward ward;
    while (auto line = prompt_line(in, out, "admit <name> | move <from> <to> | where <name> | board> ")) {
        std::istringstream words(*line);
        std::string command;
        if (!(words >> command)) {
            break;
        }
        if (command == "admit") {
            std::string name;
            words >> name;
            Bed* bed = ward.find_free_bed();
            if (admit(bed, name)) {
                out << "  " << name << " -> bed " << bed->number << '\n';
            } else {
                out << (bed == nullptr ? "  ward is full\n" : "  name required\n");
            }
        } else if (command == "move") {
            int from = 0;
            int to = 0;
            words >> from >> to;
            Bed* source = ward.bed_number(from);
            Bed* target = ward.bed_number(to);
            const bool moved = source != nullptr && target != nullptr && transfer(*source, *target);
            out << (moved ? "  moved\n" : "  cannot move\n");
        } else if (command == "where") {
            std::string name;
            words >> name;
            const Bed* bed = ward.find_patient(name);
            out << "  " << name << (bed ? " is in bed " + std::to_string(bed->number) : " is not on this ward") << '\n';
        } else if (command == "board") {
            out << "  " << count_occupied(ward.begin(), ward.end()) << " of 8 beds occupied\n";
        } else {
            out << "  unknown command\n";
        }
    }
    return 0;
}

}  // namespace cppm::day19
