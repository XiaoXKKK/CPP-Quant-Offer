#include <cassert>
#include <cstdint>
#include <iostream>
#include <iterator>
#include <limits>
#include <list>
#include <map>
#include <optional>
#include <unordered_map>

enum class Side { Bid, Ask };
using Price = std::int64_t; // positive integer ticks
using Quantity = std::int64_t;
struct Order { std::uint64_t id; Quantity qty; };
struct Level { std::list<Order> fifo; Quantity total = 0; };
class Book {
    using Levels = std::map<Price, Level>;
    struct Location { Side side; Price price; std::list<Order>::iterator order; };
    Levels bids_, asks_;
    std::unordered_map<std::uint64_t, Location> ids_;
    Levels& levels(Side side) { return side == Side::Bid ? bids_ : asks_; }
public:
    // Locations contain iterators: a default copy would be invalid.
    Book() = default;
    Book(const Book&) = delete;
    Book& operator=(const Book&) = delete;
    bool add(std::uint64_t id, Side side, Price price, Quantity qty) {
        if (price <= 0 || qty <= 0 || ids_.contains(id)) return false;
        auto& tree = levels(side);
        auto [level_it, inserted] = tree.try_emplace(price);
        auto& level = level_it->second;
        if (qty > std::numeric_limits<Quantity>::max() - level.total) return false;
        try { level.fifo.push_back({id, qty}); }
        catch (...) { if (inserted) tree.erase(level_it); throw; }
        auto order = std::prev(level.fifo.end());
        try { ids_.emplace(id, Location{side, price, order}); }
        catch (...) { level.fifo.erase(order); if (inserted) tree.erase(level_it); throw; }
        level.total += qty;
        return true;
    }
    bool reduce(std::uint64_t id, Quantity qty) {
        const auto found = ids_.find(id);
        if (found == ids_.end() || qty <= 0 || qty > found->second.order->qty) return false;
        auto& loc = found->second;
        auto& tree = levels(loc.side);
        const auto level_it = tree.find(loc.price);
        auto& level = level_it->second;
        loc.order->qty -= qty;
        level.total -= qty;
        if (loc.order->qty == 0) {
            level.fifo.erase(loc.order);
            ids_.erase(found);
            if (level.fifo.empty()) tree.erase(level_it);
        }
        return true;
    }
    bool cancel(std::uint64_t id) {
        const auto it = ids_.find(id);
        return it != ids_.end() && reduce(id, it->second.order->qty);
    }
    std::optional<Price> best(Side side) const {
        const auto& tree = side == Side::Bid ? bids_ : asks_;
        if (tree.empty()) return std::nullopt;
        return side == Side::Bid ? tree.rbegin()->first : tree.begin()->first;
    }
    Quantity volume(Side side, Price price) const {
        const auto& tree = side == Side::Bid ? bids_ : asks_;
        const auto it = tree.find(price);
        return it == tree.end() ? 0 : it->second.total;
    }
    std::optional<std::uint64_t> first(Side side, Price price) const {
        const auto& tree = side == Side::Bid ? bids_ : asks_;
        const auto it = tree.find(price);
        if (it == tree.end()) return std::nullopt;
        return it->second.fifo.front().id;
    }
};
int main() {
    Book book;
    assert(!book.best(Side::Bid));
    assert(book.add(1, Side::Bid, 10000, 10));
    assert(book.add(2, Side::Bid, 10000, 20));
    assert(book.add(3, Side::Ask, 10002, 5));
    assert(!book.add(1, Side::Ask, 10003, 1));
    assert(!book.add(9, Side::Bid, -1, 1));
    assert(book.first(Side::Bid, 10000) == 1);
    assert(book.reduce(1, 4));
    assert(book.volume(Side::Bid, 10000) == 26);
    assert(!book.reduce(1, 7));
    assert(!book.reduce(999, 1));
    assert(book.cancel(1));
    assert(book.first(Side::Bid, 10000) == 2);
    assert(book.cancel(2));
    assert(!book.best(Side::Bid));
    assert(book.best(Side::Ask) == 10002);
    assert(book.add(4, Side::Bid, 9000, std::numeric_limits<Quantity>::max()));
    assert(!book.add(5, Side::Bid, 9000, 1));
    assert(book.cancel(4));
    assert(book.cancel(3));
    assert(!book.best(Side::Ask));
    std::cout << "MBO book: add/reduce/cancel/FIFO/overflow/empty checks passed\n";
}
