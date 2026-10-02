// Tests for Day 94 – Capstone: Validation Library.
#include <sstream>
#include <string>
#include <vector>

#include "cppm/testing.hpp"
#include "day_94_validation/lesson.hpp"

using namespace cppm::day94;

namespace {
std::vector<std::string> paths(const Result& r) {
    std::vector<std::string> out;
    for (const auto& p : r.problems) out.push_back(p.path + ": " + p.message);
    return out;
}
Value attendee(const char* name, const char* mail) { return Value::Object{{"name", name}, {"email", mail}}; }
}  // namespace

TEST_CASE("text trims and checks length") {
    CHECK(validate(text(2, 5), "  Ana ").value == Value("Ana"));
    CHECK(paths(validate(text(2, 5), " A ")) == std::vector<std::string>{"(root): must be at least 2 characters"});
    CHECK(paths(validate(text(1, 3), "   ")) == std::vector<std::string>{"(root): is required"});
    CHECK(paths(validate(text(1, 3), 42)) == std::vector<std::string>{"(root): must be text"});
}

TEST_CASE("integers accept numeric text and enforce ranges") {
    CHECK(validate(integer(0, 10), "7").value == Value(7));
    CHECK(validate(integer(0, 10), 3).value == Value(3));
    CHECK(!validate(integer(0, 10), 2.5).value.has_value());
    CHECK(!validate(integer(0, 10), "7a").value.has_value());
    CHECK(paths(validate(integer(0, 10), 11)) == std::vector<std::string>{"(root): must be between 0 and 10"});
}

TEST_CASE("e-mail validation normalises case") {
    CHECK(validate(email(), " Ana@Example.ORG ").value == Value("ana@example.org"));
    for (const char* bad : {"ana", "@example.org", "a@b@c.org", "ana@example", "ana@example.", "a na@x.org"}) {
        CHECK(!validate(email(), bad).value.has_value());
    }
}

TEST_CASE("all_of pipes normalised values and stops at the first failure") {
    const auto even_small = all_of({integer(0, 100), check([](const Value& v) { return static_cast<int>(*v.get<double>()) % 2 == 0; }, "must be even")});
    CHECK(validate(even_small, "8").value == Value(8));
    CHECK(paths(validate(even_small, "7")) == std::vector<std::string>{"(root): must be even"});
    CHECK_EQ(validate(even_small, "x").problems.size(), 1u);  // the check never ran
}

TEST_CASE("a valid order is normalised, with defaults filled in") {
    const Value order = Value::Object{{"event", " RustConf "}, {"buyer_email", "BUYER@x.org"},
                                      {"attendees", Value::List{attendee("Ana", "ANA@x.org"), Value::Object{{"name", "Ben"}, {"email", "ben@x.org"}, {"ticket", "vip"}}}},
                                      {"donation_eur", "20"}};
    const auto r = validate(order_schema(), order);
    CHECK(r.problems.empty());
    CHECK(*r.value->field("event") == Value("RustConf"));
    const auto& people = *r.value->field("attendees")->get<Value::List>();
    CHECK(*people[0].field("email") == Value("ana@x.org"));
    CHECK(*people[0].field("ticket") == Value("standard"));
    CHECK(*people[1].field("ticket") == Value("vip"));
    CHECK(*r.value->field("donation_eur") == Value(20));
}

TEST_CASE("every problem in a nested payload is reported with its path") {
    const Value order = Value::Object{{"buyer_email", "nope"},
                                      {"attendees", Value::List{attendee("Ana", "ana@x.org"), Value::Object{{"name", "B"}, {"email", "b@x"}, {"ticket", "free"}}}},
                                      {"donation_eur", 13},
                                      {"coupon", "HACK"}};
    CHECK(paths(validate(order_schema(), order)) ==
          std::vector<std::string>{"event: is required", "buyer_email: must be an e-mail address",
                                   "attendees[1].name: must be at least 2 characters", "attendees[1].email: must be an e-mail address",
                                   "attendees[1].ticket: must be one of: standard, student, vip", "donation_eur: must not be 13 (we are superstitious)",
                                   "coupon: is not allowed"});
    CHECK(paths(validate(order_schema(), Value::Object{{"event", "x"}, {"buyer_email", "a@b.co"}, {"attendees", Value::List{}}})) ==
          std::vector<std::string>{"attendees: must have 1 to 10 items"});
    CHECK(paths(validate(order_schema(), "text")) == std::vector<std::string>{"(root): must be an object"});
}

TEST_CASE("run validates orders typed on one line") {
    std::istringstream in("PyCon Buyer@x.org Ana:ANA@x.org:student Ben:ben@x.org\nPyCon nope X:y\nstop\n");
    std::ostringstream out;
    CHECK_EQ(run(in, out), 0);
    const auto text = out.str();
    CHECK(text.find("valid\n  Ana <ana@x.org> student\n  Ben <ben@x.org> standard\n") != std::string::npos);
    CHECK(text.find("buyer_email: must be an e-mail address\nattendees[0].name: must be at least 2 characters") != std::string::npos);
}
