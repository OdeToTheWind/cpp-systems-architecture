// Tests for Day 27 – Object-Oriented Programming Basics.
#include <memory>
#include <sstream>
#include <stdexcept>
#include <type_traits>
#include <vector>

#include "cppm/testing.hpp"
#include "day_27_oop_basics/lesson.hpp"

using namespace cppm::day27;

TEST_CASE("the interface is abstract and has a virtual destructor") {
    static_assert(std::is_abstract_v<PaymentMethod>);
    static_assert(std::has_virtual_destructor_v<PaymentMethod>);
    static_assert(std::is_base_of_v<PaymentMethod, CardPayment>);
    CHECK(!std::is_abstract_v<WalletPayment>);
}

TEST_CASE("luhn_valid accepts real test numbers and rejects typos") {
    CHECK(luhn_valid("4111111111111111"));
    CHECK(luhn_valid("5555555555554444"));
    CHECK(!luhn_valid("4111111111111112"));
    CHECK(!luhn_valid("4111-1111"));
    CHECK(!luhn_valid("123"));
}

TEST_CASE("a card hides its number and enforces its limit") {
    CardPayment card("4111 1111 1111 1111", 10'000);
    CHECK_EQ(card.describe(), "card **** 1111");
    CHECK(card.charge(6'000).approved);
    const Receipt declined = card.charge(6'000);
    CHECK(!declined.approved);
    CHECK_EQ(declined.reason, "over the card limit");
    CHECK_THROWS_AS(CardPayment("4111 1111 1111 1112", 1), std::invalid_argument);
}

TEST_CASE("a wallet's balance changes only through its methods") {
    WalletPayment wallet("Ada");
    CHECK(!wallet.charge(1).approved);
    wallet.top_up(500);
    CHECK(wallet.charge(300).approved);
    CHECK_EQ(wallet.balance_cents(), 200);
    CHECK_THROWS_AS(wallet.top_up(0), std::invalid_argument);
}

TEST_CASE("a voucher can be used once") {
    VoucherPayment voucher("GIFT", 1'000);
    CHECK_EQ(voucher.charge(2'000).reason, "voucher value too low");
    CHECK(voucher.charge(800).approved);
    CHECK_EQ(voucher.charge(100).reason, "voucher already used");
}

TEST_CASE("the gateway charges any method polymorphically") {
    std::vector<std::unique_ptr<PaymentMethod>> methods;
    methods.push_back(std::make_unique<CardPayment>("5555555555554444", 5'000));
    auto wallet = std::make_unique<WalletPayment>("Bo");
    wallet->top_up(1'000);
    methods.push_back(std::move(wallet));
    methods.push_back(std::make_unique<VoucherPayment>("V1", 2'000));
    Gateway gateway;
    for (auto& method : methods) {
        gateway.process(*method, 1'500);
    }
    REQUIRE_EQ(gateway.history().size(), 3u);
    CHECK(gateway.history()[0].approved);
    CHECK(!gateway.history()[1].approved);
    CHECK(gateway.history()[2].approved);
    CHECK_EQ(gateway.approved_total(), 3'000);
    CHECK_THROWS_AS(gateway.process(*methods[0], 0), std::invalid_argument);
}

TEST_CASE("run processes payments and never prints a full card number") {
    std::istringstream in("card 1200\nwallet 1500\nwallet 600\nvoucher 1500\nvoucher 10\ncash 5\nwallet\n\n");
    std::ostringstream out;
    CHECK_EQ(run(in, out), 0);
    const auto text = out.str();
    CHECK(text.find("card **** 1111: approved") != std::string::npos);
    CHECK(text.find("wallet of Ada: approved") != std::string::npos);
    CHECK(text.find("declined – insufficient balance") != std::string::npos);
    CHECK(text.find("voucher already used") != std::string::npos);
    CHECK(text.find("unknown method") != std::string::npos);
    CHECK(text.find("4111 1111") == std::string::npos);
    CHECK(text.find("Approved today: 4200 cents in 5 attempt(s)") != std::string::npos);
}
