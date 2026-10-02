// Module 2 – the ports: interfaces the service needs from the outside world.
// Production code would implement them with a database, SMTP and the system clock;
// the in-memory versions below are what the tests (and the demo) use.
#pragma once

#include <map>
#include <optional>
#include <string>
#include <vector>

#include "library/model.hpp"

namespace cppm::day92 {

class Clock {
  public:
    virtual ~Clock() = default;
    virtual Date today() const = 0;
};

class Repository {
  public:
    virtual ~Repository() = default;
    virtual std::optional<Book> book(const std::string& isbn) const = 0;
    virtual std::optional<Member> member(const std::string& id) const = 0;
    virtual void add_book(const Book& book) = 0;
    virtual void add_member(const Member& member) = 0;
    virtual int add_loan(Loan loan) = 0;  // returns the new loan id
    virtual void update_loan(const Loan& loan) = 0;
    virtual void remove_loan(int id) = 0;
    virtual std::optional<Loan> loan(int id) const = 0;
    virtual std::vector<Loan> loans_of_book(const std::string& isbn) const = 0;
    virtual std::vector<Loan> loans_of_member(const std::string& member) const = 0;
    virtual std::vector<std::string>& reservations(const std::string& isbn) = 0;  // FIFO queue of member ids
};

class Notifier {
  public:
    virtual ~Notifier() = default;
    virtual void notify(const Member& member, const std::string& message) = 0;
};

// ---- Test doubles ----

class FixedClock : public Clock {
  public:
    explicit FixedClock(Date today) : today_(today) {}
    Date today() const override { return today_; }
    void advance(int days) { today_ = today_ + days; }

  private:
    Date today_;
};

class InMemoryRepository : public Repository {
  public:
    std::optional<Book> book(const std::string& isbn) const override { return find(books_, isbn); }
    std::optional<Member> member(const std::string& id) const override { return find(members_, id); }
    void add_book(const Book& book) override { books_[book.isbn] = book; }
    void add_member(const Member& member) override { members_[member.id] = member; }
    int add_loan(Loan loan) override;
    void update_loan(const Loan& loan) override { loans_.at(loan.id) = loan; }
    void remove_loan(int id) override { loans_.erase(id); }
    std::optional<Loan> loan(int id) const override { return find(loans_, id); }
    std::vector<Loan> loans_of_book(const std::string& isbn) const override;
    std::vector<Loan> loans_of_member(const std::string& member) const override;
    std::vector<std::string>& reservations(const std::string& isbn) override { return reservations_[isbn]; }

  private:
    template <typename Map, typename Key>
    static std::optional<typename Map::mapped_type> find(const Map& map, const Key& key) {
        const auto it = map.find(key);
        return it == map.end() ? std::nullopt : std::optional<typename Map::mapped_type>(it->second);
    }
    std::map<std::string, Book> books_;
    std::map<std::string, Member> members_;
    std::map<int, Loan> loans_;
    std::map<std::string, std::vector<std::string>> reservations_;
    int next_loan_ = 1;
};

class RecordingNotifier : public Notifier {
  public:
    struct Message {
        std::string to;
        std::string text;
    };
    void notify(const Member& member, const std::string& message) override { sent.push_back({member.email, message}); }
    std::vector<Message> sent;
};

}  // namespace cppm::day92
