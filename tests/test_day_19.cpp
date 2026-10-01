// Tests for Day 19 – Pointers and References.
#include <sstream>
#include <string>
#include <type_traits>

#include "cppm/testing.hpp"
#include "day_19_pointers_references/lesson.hpp"

using namespace cppm::day19;

TEST_CASE("writes through a pointer and a reference change the original variable") {
    const auto facts = address_basics();
    CHECK_EQ(facts.value_after_write_through_pointer, 5);
    CHECK_EQ(facts.value_after_write_through_reference, 7);
    CHECK(facts.pointer_holds_address_of_variable);
}

TEST_CASE("find_free_bed returns nullptr once the ward is full") {
    Ward ward;
    for (int i = 0; i < 8; ++i) {
        Bed* bed = ward.find_free_bed();
        REQUIRE(bed != nullptr);
        CHECK_EQ(bed->number, i + 1);
        CHECK(admit(bed, "patient" + std::to_string(i)));
    }
    CHECK(ward.find_free_bed() == nullptr);
}

TEST_CASE("admit checks for nullptr, occupied beds and empty names") {
    Ward ward;
    CHECK(!admit(nullptr, "Ada"));
    Bed* bed = ward.find_free_bed();
    CHECK(!admit(bed, ""));
    CHECK(admit(bed, "Ada"));
    CHECK(!admit(bed, "Bob"));
    CHECK_EQ(bed->patient, "Ada");
}

TEST_CASE("transfer moves a patient through references") {
    Ward ward;
    Bed& one = *ward.bed_number(1);
    Bed& two = *ward.bed_number(2);
    admit(&one, "Ada");
    CHECK(transfer(one, two));
    CHECK(!one.occupied());
    CHECK_EQ(two.patient, "Ada");
    CHECK(!transfer(one, two));  // source is empty now
    CHECK(!transfer(two, two));  // same bed
}

TEST_CASE("find_patient gives read-only access") {
    Ward ward;
    admit(ward.find_free_bed(), "Grace");
    const Ward& read_only = ward;
    const Bed* bed = read_only.find_patient("Grace");
    REQUIRE(bed != nullptr);
    CHECK_EQ(bed->number, 1);
    CHECK(read_only.find_patient("Nobody") == nullptr);
    static_assert(std::is_same_v<decltype(read_only.find_patient("x")), const Bed*>);
}

TEST_CASE("pointer arithmetic walks the bed array") {
    Ward ward;
    admit(ward.bed_number(2), "A");
    admit(ward.bed_number(5), "B");
    CHECK_EQ(count_occupied(ward.begin(), ward.end()), 2);
    CHECK_EQ(count_occupied(ward.begin(), ward.begin() + 3), 1);
    CHECK_EQ(ward.end() - ward.begin(), 8);
    CHECK(ward.bed_number(0) == nullptr);
    CHECK(ward.bed_number(9) == nullptr);
}

TEST_CASE("growing a full vector relocates its elements") {
    CHECK(vector_storage_moved());
}

TEST_CASE("run admits, moves and finds patients") {
    std::istringstream in("admit Ada\nadmit Bob\nmove 1 3\nmove 9 1\nwhere Ada\nwhere Zed\nboard\nfly\n\n");
    std::ostringstream out;
    CHECK_EQ(run(in, out), 0);
    const auto text = out.str();
    CHECK(text.find("Ada -> bed 1") != std::string::npos);
    CHECK(text.find("Bob -> bed 2") != std::string::npos);
    CHECK(text.find("moved") != std::string::npos);
    CHECK(text.find("cannot move") != std::string::npos);
    CHECK(text.find("Ada is in bed 3") != std::string::npos);
    CHECK(text.find("Zed is not on this ward") != std::string::npos);
    CHECK(text.find("2 of 8 beds occupied") != std::string::npos);
}
