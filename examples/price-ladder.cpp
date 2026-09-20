#include <array>
#include <bit>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <limits>
#include <map>
#include <optional>
#include <stdexcept>
#include <string_view>

using Price = std::int64_t;
using Quantity = std::uint64_t;
constexpr Price max_price = std::numeric_limits<Price>::max();
constexpr Price min_price = std::numeric_limits<Price>::min();
constexpr Quantity max_quantity = std::numeric_limits<Quantity>::max();
constexpr std::size_t maximum_levels = 64;

// Exact grammar: optional '-', one or more integer digits, '.', exactly two digits.
std::optional<Price> parse_fixed_2(std::string_view text) {
    if (text.empty()) return std::nullopt;
    const bool negative = text.front() == '-';
    const std::size_t begin = negative ? 1 : 0;
    if (text.size() < begin + 4) return std::nullopt;
    const std::size_t point = text.size() - 3;
    if (text[point] != '.') return std::nullopt;
    const Quantity negative_limit = Quantity{1} << 63;
    const Quantity limit = negative ? negative_limit : static_cast<Quantity>(max_price);
    Quantity magnitude = 0;
    for (std::size_t i = begin; i < text.size(); ++i) {
        if (i == point) continue;
        if (text[i] < '0' || text[i] > '9') return std::nullopt;
        const auto digit = static_cast<Quantity>(text[i] - '0');
        if (magnitude > (limit - digit) / 10) return std::nullopt;
        magnitude = magnitude * 10 + digit;
    }
    if (negative && magnitude == negative_limit) return min_price;
    const Price value = static_cast<Price>(magnitude);
    return negative ? -value : value;
}

class PriceGrid {
public:
    PriceGrid(Price minimum, Price tick, std::size_t levels)
        : minimum_(minimum), tick_(tick), levels_(levels) {
        if (tick <= 0 || levels == 0 || levels > maximum_levels) {
            throw std::invalid_argument("invalid fixed-tick grid");
        }
        const auto steps = static_cast<Quantity>(levels - 1);
        const auto unsigned_tick = static_cast<Quantity>(tick);
        if (steps != 0 && unsigned_tick > static_cast<Quantity>(max_price) / steps) {
            throw std::overflow_error("grid span exceeds signed price range");
        }
        const Price span = static_cast<Price>(steps * unsigned_tick);
        if (minimum > max_price - span) throw std::overflow_error("last grid price overflows");
        maximum_ = minimum + span;
    }
    std::optional<std::size_t> index(Price price) const {
        if (price < minimum_ || price > maximum_) return std::nullopt;
        // Avoid signed price-minimum overflow. After bounds checks, the mathematical
        // difference is nonnegative and <= max_price; unsigned subtraction preserves it.
        const Quantity distance = static_cast<Quantity>(price) - static_cast<Quantity>(minimum_);
        const auto tick = static_cast<Quantity>(tick_);
        if (distance % tick != 0) return std::nullopt;
        return static_cast<std::size_t>(distance / tick);
    }
    Price price(std::size_t index) const {
        if (index >= levels_) throw std::out_of_range("grid index");
        const auto offset = static_cast<Quantity>(index) * static_cast<Quantity>(tick_);
        return minimum_ + static_cast<Price>(offset);
    }
    std::size_t levels() const { return levels_; }
private:
    Price minimum_;
    Price tick_;
    std::size_t levels_;
    Price maximum_ = 0;
};

struct Level {
    Price price;
    Quantity quantity;
    bool operator==(const Level&) const = default;
};
enum class Update { applied, invalid_price, total_overflow };

class PriceLadder {
public:
    explicit PriceLadder(PriceGrid grid) : grid_(grid) {}
    // Absolute aggregate quantity (MBP-like), not an order add/cancel delta.
    Update set(Price price, Quantity quantity) {
        const auto index = grid_.index(price);
        if (!index) return Update::invalid_price;
        const Quantity without_old = total_ - quantities_[*index];
        if (quantity > max_quantity - without_old) return Update::total_overflow;
        quantities_[*index] = quantity;
        total_ = without_old + quantity;
        const Quantity bit = Quantity{1} << *index; // 0 <= index < 64.
        if (quantity != 0) occupied_ |= bit;
        else occupied_ &= ~bit;
        verify();
        return Update::applied;
    }
    std::optional<Quantity> quantity(Price price) const {
        const auto index = grid_.index(price);
        if (!index) return std::nullopt;
        return quantities_[*index]; // Valid, empty level returns zero.
    }
    std::optional<Level> lowest() const {
        if (occupied_ == 0) return std::nullopt;
        return level(static_cast<std::size_t>(std::countr_zero(occupied_)));
    }
    std::optional<Level> highest() const {
        if (occupied_ == 0) return std::nullopt;
        return level(static_cast<std::size_t>(63 - std::countl_zero(occupied_)));
    }
    Quantity total() const { return total_; }
    Quantity mask() const { return occupied_; }
private:
    Level level(std::size_t index) const { return {grid_.price(index), quantities_[index]}; }
    void verify() const {
        Quantity sum = 0;
        for (std::size_t i = 0; i < maximum_levels; ++i) {
            const bool bit = (occupied_ & (Quantity{1} << i)) != 0;
            assert(bit == (quantities_[i] != 0));
            if (i >= grid_.levels()) assert(!bit && quantities_[i] == 0);
            assert(quantities_[i] <= max_quantity - sum);
            sum += quantities_[i];
        }
        assert(sum == total_);
    }
    PriceGrid grid_;
    std::array<Quantity, maximum_levels> quantities_{};
    Quantity occupied_ = 0;
    Quantity total_ = 0;
};

void test_decimal_and_grid() {
    assert(parse_fixed_2("12.35") == 1235);
    assert(parse_fixed_2("-0.05") == -5);
    assert(parse_fixed_2("-0.00") == 0);
    assert(parse_fixed_2("0001.00") == 100);
    assert(parse_fixed_2("92233720368547758.07") == max_price);
    assert(parse_fixed_2("-92233720368547758.08") == min_price);
    for (auto bad : {"", ".12", "1.2", "1.234", "+1.00", " 1.00", "1e2", "nan",
                     "92233720368547758.08", "-92233720368547758.09"}) {
        assert(!parse_fixed_2(bad));
    }
    const PriceGrid grid(-100, 5, 41);
    assert(grid.index(-100) == 0 && grid.index(0) == 20 && grid.index(100) == 40);
    assert(!grid.index(-101) && !grid.index(101) && !grid.index(-98));
    for (std::size_t i = 0; i < grid.levels(); ++i) assert(grid.index(grid.price(i)) == i);
    const PriceGrid low(min_price, 1, 64);
    const PriceGrid high(max_price - 63, 1, 64);
    assert(low.price(63) == min_price + 63 && low.index(min_price + 63) == 63);
    assert(high.price(63) == max_price && high.index(max_price) == 63);
    const PriceGrid single(max_price, max_price, 1);
    assert(single.price(0) == max_price && single.index(max_price) == 0);
    auto throws = [](auto action) {
        bool thrown = false;
        try { action(); } catch (const std::invalid_argument&) { thrown = true; }
        catch (const std::overflow_error&) { thrown = true; }
        catch (const std::out_of_range&) { thrown = true; }
        assert(thrown);
    };
    throws([] { (void)PriceGrid(0, 0, 1); });
    throws([] { (void)PriceGrid(0, -1, 1); });
    throws([] { (void)PriceGrid(0, 1, 0); });
    throws([] { (void)PriceGrid(0, 1, 65); });
    throws([] { (void)PriceGrid(0, max_price, 3); });
    throws([] { (void)PriceGrid(max_price, 1, 2); });
    throws([&] { (void)grid.price(grid.levels()); });
}

void test_independent_map() {
    const PriceGrid grid(10000, 5, 64);
    PriceLadder ladder(grid);
    std::map<Price, Quantity> oracle;
    assert(!ladder.lowest() && !ladder.highest());
    for (std::size_t step = 0; step < 400; ++step) {
        const std::size_t index = (step * 17 + 3) % 64;
        const Price price = 10000 + static_cast<Price>(index * 5);
        const Quantity quantity = static_cast<Quantity>((step * 7) % 13);
        assert(ladder.set(price, quantity) == Update::applied);
        if (quantity == 0) oracle.erase(price);
        else oracle[price] = quantity;
        Quantity total = 0;
        for (const auto& [key, value] : oracle) {
            (void)key;
            total += value;
        }
        assert(ladder.total() == total);
        if (oracle.empty()) {
            assert(!ladder.lowest() && !ladder.highest());
        } else {
            assert((ladder.lowest() == Level{oracle.begin()->first, oracle.begin()->second}));
            assert((ladder.highest() == Level{oracle.rbegin()->first, oracle.rbegin()->second}));
        }
        for (std::size_t i = 0; i < 64; ++i) {
            const Price key = 10000 + static_cast<Price>(i * 5);
            const auto found = oracle.find(key);
            const Quantity expected = found == oracle.end() ? 0 : found->second;
            assert(ladder.quantity(key) == expected);
        }
        const auto before_mask = ladder.mask();
        assert(ladder.set(10001, 99) == Update::invalid_price);
        assert(ladder.mask() == before_mask && ladder.total() == total);
    }
    std::cout << "map_oracle_updates=400 level_checks=25600 passed\n";
}

void test_bitmap_and_totals() {
    PriceLadder ladder(PriceGrid(0, 1, 64));
    assert(ladder.set(63, 7) == Update::applied);
    assert(ladder.mask() == (Quantity{1} << 63));
    assert((ladder.lowest() == Level{63, 7}) && (ladder.highest() == Level{63, 7}));
    assert(ladder.set(0, 3) == Update::applied);
    assert(ladder.total() == 10 && ladder.lowest()->price == 0);
    assert(ladder.set(63, 0) == Update::applied && ladder.highest()->price == 0);
    assert(ladder.set(0, 0) == Update::applied && !ladder.highest());
    assert(ladder.quantity(0) == 0 && !ladder.quantity(64));
    assert(ladder.set(1, max_quantity) == Update::applied);
    const auto mask = ladder.mask();
    assert(ladder.set(2, 1) == Update::total_overflow);
    assert(ladder.total() == max_quantity && ladder.mask() == mask && ladder.quantity(2) == 0);
    assert(ladder.set(1, max_quantity) == Update::applied); // Replacement, not addition.
    assert(ladder.set(1, max_quantity - 1) == Update::applied);
    assert(ladder.set(2, 1) == Update::applied && ladder.total() == max_quantity);
    assert(ladder.set(1, 0) == Update::applied && ladder.total() == 1);
    PriceLadder one(PriceGrid(-5, 5, 1));
    assert(one.set(-5, 2) == Update::applied);
    assert((one.lowest() == Level{-5, 2}) && one.lowest() == one.highest());
    assert(one.set(0, 1) == Update::invalid_price);
    std::cout << "bit0/bit63, empty/single-level and total overflow passed\n";
}

int main() {
    test_decimal_and_grid();
    test_independent_map();
    test_bitmap_and_totals();
    std::cout << "fixed tick, bounded price domain, absolute level quantities only\n";
}
