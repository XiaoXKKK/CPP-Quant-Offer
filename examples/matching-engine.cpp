#include <algorithm>
#include <array>
#include <cstdint>
#include <iostream>
#include <stdexcept>
#include <type_traits>
#include <vector>

namespace {
constexpr std::size_t capacity = 8;
constexpr unsigned max_id = 64;
constexpr unsigned max_price = 1000;
constexpr unsigned max_quantity = 8;
enum class Side { buy, sell };
enum class Tif { gtc, ioc };
enum class Status { accepted, invalid, duplicate, full };
struct Input {
    unsigned id;
    Side side;
    unsigned price;
    unsigned quantity;
    Tif tif = Tif::gtc;
};
struct Resting {
    unsigned id = 0;
    Side side = Side::buy;
    unsigned price = 0;
    unsigned quantity = 0;
    unsigned priority = 0;
    bool operator==(const Resting&) const = default;
};
struct Trade {
    unsigned maker = 0;
    unsigned taker = 0;
    unsigned price = 0;
    unsigned quantity = 0;
    bool operator==(const Trade&) const = default;
};
struct Result {
    Status status = Status::invalid;
    std::array<Trade, capacity> trades{};
    std::size_t trade_count = 0;
    unsigned executed = 0;
    unsigned resting = 0;
    unsigned canceled = 0;
    bool operator==(const Result&) const = default;
};
struct State {
    std::array<Resting, capacity> book{};
    std::array<bool, max_id + 1> seen{};
    unsigned accepted_count = 0;
    bool operator==(const State&) const = default;
};
static_assert(std::is_nothrow_copy_assignable_v<State>);
static_assert(std::is_nothrow_copy_constructible_v<Result>);
static_assert(std::is_nothrow_move_constructible_v<Result>);
struct InjectedFailure {};

void require(bool condition, const char* message) {
    if (!condition) {
        throw std::runtime_error(message);
    }
}

class Engine {
public:
    Result submit(Input input, bool fail_before_commit = false) {
        if (input.id == 0 || input.id > max_id || input.price == 0 ||
            input.price > max_price || input.quantity == 0 || input.quantity > max_quantity ||
            (input.side != Side::buy && input.side != Side::sell) ||
            (input.tif != Tif::gtc && input.tif != Tif::ioc)) {
            return {};
        }
        if (state_.seen[input.id]) {
            Result rejected;
            rejected.status = Status::duplicate;
            return rejected;
        }
        State candidate = state_;
        Result result;
        result.status = Status::accepted;
        unsigned remaining = input.quantity;
        while (remaining != 0) {
            std::size_t best = capacity;
            for (std::size_t i = 0; i < capacity; ++i) {
                const auto& order = candidate.book[i];
                if (order.quantity == 0 || order.side == input.side) {
                    continue;
                }
                const bool crosses = input.side == Side::buy ? order.price <= input.price
                                                              : order.price >= input.price;
                if (!crosses) {
                    continue;
                }
                if (best == capacity) {
                    best = i;
                    continue;
                }
                const auto& previous = candidate.book[best];
                const bool better_price = input.side == Side::buy ? order.price < previous.price
                                                                  : order.price > previous.price;
                if (better_price || (order.price == previous.price &&
                                     order.priority < previous.priority)) {
                    best = i;
                }
            }
            if (best == capacity) {
                break;
            }
            auto& maker = candidate.book[best];
            const unsigned quantity = std::min(remaining, maker.quantity);
            // Each iteration exhausts a maker or the incoming order: <= capacity trades.
            result.trades[result.trade_count++] = {maker.id, input.id, maker.price, quantity};
            result.executed += quantity;
            remaining -= quantity;
            maker.quantity -= quantity;
            if (maker.quantity == 0) {
                maker = {};
            }
        }
        if (remaining != 0 && input.tif == Tif::gtc) {
            auto slot = std::find_if(candidate.book.begin(), candidate.book.end(),
                                     [](const Resting& order) { return order.quantity == 0; });
            if (slot == candidate.book.end()) {
                Result rejected;
                rejected.status = Status::full;
                return rejected;
            }
            *slot = {input.id, input.side, input.price, remaining, candidate.accepted_count + 1};
            result.resting = remaining;
        } else {
            result.canceled = remaining;
        }
        // Accepted IDs cannot be reused during this bounded engine session.
        candidate.seen[input.id] = true;
        ++candidate.accepted_count; // At most max_id, because accepted IDs are unique.
        if (fail_before_commit) {
            throw InjectedFailure{};
        }
        state_ = candidate; // Fixed storage; no allocation or throwing publication callback.
        return result;     // Result copying is nonthrowing. External delivery is out of scope.
    }

    unsigned cancel(unsigned id) noexcept {
        for (auto& order : state_.book) {
            if (id != 0 && order.id == id) {
                const auto removed = order.quantity;
                order = {};
                return removed;
            }
        }
        return 0; // Unknown, already filled, or already canceled.
    }

    const State& state() const noexcept { return state_; }

private:
    State state_{};
};

// Independent slow model: expand each order into unit-quantity lots, sort all
// eligible lots, then execute one unit at a time. No reuse of Engine's selector.
class UnitOracle {
    struct Unit {
        unsigned id;
        Side side;
        unsigned price;
        unsigned priority;
    };
public:
    Result submit(Input input) {
        if (!(input.id >= 1 && input.id <= 64 && input.price >= 1 && input.price <= 1000 &&
              input.quantity >= 1 && input.quantity <= 8 &&
              (input.side == Side::buy || input.side == Side::sell) &&
              (input.tif == Tif::gtc || input.tif == Tif::ioc))) {
            return {};
        }
        if (seen_[input.id]) {
            Result result;
            result.status = Status::duplicate;
            return result;
        }
        auto next = units_;
        std::vector<Unit> eligible;
        for (const auto unit : units_) {
            if ((input.side == Side::buy && unit.side == Side::sell && unit.price <= input.price) ||
                (input.side == Side::sell && unit.side == Side::buy && unit.price >= input.price)) {
                eligible.push_back(unit);
            }
        }
        std::sort(eligible.begin(), eligible.end(), [input](const Unit& a, const Unit& b) {
            if (a.price != b.price) {
                return input.side == Side::buy ? a.price < b.price : a.price > b.price;
            }
            return a.priority < b.priority;
        });
        Result result;
        result.status = Status::accepted;
        for (const auto unit : eligible) {
            if (result.executed == input.quantity) {
                break;
            }
            const auto position = std::find_if(next.begin(), next.end(),
                                               [unit](const Unit& u) { return u.id == unit.id; });
            require(position != next.end(), "oracle lot missing");
            next.erase(position);
            if (result.trade_count != 0 && result.trades[result.trade_count - 1].maker == unit.id) {
                ++result.trades[result.trade_count - 1].quantity;
            } else {
                result.trades[result.trade_count++] = {unit.id, input.id, unit.price, 1};
            }
            ++result.executed;
        }
        const auto remaining = input.quantity - result.executed;
        if (remaining != 0 && input.tif == Tif::gtc) {
            std::array<bool, max_id + 1> present{};
            for (const auto unit : next) {
                present[unit.id] = true;
            }
            if (std::count(present.begin(), present.end(), true) >=
                static_cast<std::ptrdiff_t>(capacity)) {
                Result rejected;
                rejected.status = Status::full;
                return rejected;
            }
            for (unsigned i = 0; i < remaining; ++i) {
                next.push_back({input.id, input.side, input.price, accepted_count_ + 1});
            }
            result.resting = remaining;
        } else {
            result.canceled = remaining;
        }
        units_.swap(next);
        seen_[input.id] = true;
        ++accepted_count_;
        return result;
    }

    unsigned cancel(unsigned id) {
        const auto old_size = units_.size();
        std::erase_if(units_, [id](const Unit& unit) { return unit.id == id; });
        return static_cast<unsigned>(old_size - units_.size());
    }

    void compare(const Engine& engine) const {
        const auto& state = engine.state();
        require(state.seen == seen_ && state.accepted_count == accepted_count_, "ID history mismatch");
        std::array<Resting, max_id + 1> expected{};
        for (const auto unit : units_) {
            auto& order = expected[unit.id];
            if (order.quantity == 0) {
                order = {unit.id, unit.side, unit.price, 0, unit.priority};
            }
            ++order.quantity;
        }
        std::array<Resting, max_id + 1> actual{};
        unsigned best_bid = 0;
        unsigned best_ask = max_price + 1;
        for (const auto order : state.book) {
            if (order.quantity != 0) {
                require(order.id <= max_id && order.id != 0, "invalid live ID");
                require(actual[order.id].quantity == 0, "duplicate live ID");
                actual[order.id] = order;
                if (order.side == Side::buy) {
                    best_bid = std::max(best_bid, order.price);
                } else {
                    best_ask = std::min(best_ask, order.price);
                }
            }
        }
        require(actual == expected, "order state mismatch");
        require(best_bid < best_ask, "crossed resting book");
    }

private:
    std::vector<Unit> units_;
    std::array<bool, max_id + 1> seen_{};
    unsigned accepted_count_ = 0;
};

struct CheckedEngine {
    Engine engine;
    UnitOracle oracle;
    Result submit(Input input) {
        const State before = engine.state();
        const auto result = engine.submit(input);
        require(result == oracle.submit(input), "trade/result mismatch");
        oracle.compare(engine);
        if (result.status == Status::accepted) {
            require(result.executed + result.resting + result.canceled == input.quantity,
                    "incoming quantity conservation");
        } else {
            require(engine.state() == before && result.trade_count == 0, "rejection mutated state");
        }
        return result;
    }
    unsigned cancel(unsigned id) {
        const auto removed = engine.cancel(id);
        require(removed == oracle.cancel(id), "cancel mismatch");
        oracle.compare(engine);
        return removed;
    }
};

void deterministic_tests() {
    for (const Side maker_side : {Side::sell, Side::buy}) {
        CheckedEngine test;
        const Side taker_side = maker_side == Side::sell ? Side::buy : Side::sell;
        test.submit({1, maker_side, 100, 3});
        test.submit({2, maker_side, 100, 4});
        const unsigned worse_price = maker_side == Side::sell ? 101U : 99U;
        test.submit({3, maker_side, worse_price, 2});
        auto result = test.submit({4, taker_side, worse_price, 5});
        require(result.trade_count == 2 && result.trades[0] == Trade{1, 4, 100, 3} &&
                    result.trades[1] == Trade{2, 4, 100, 2}, "price/time partial match");
        result = test.submit({5, taker_side, worse_price, 7, Tif::ioc});
        require(result.trade_count == 2 && result.trades[0] == Trade{2, 5, 100, 2} &&
                    result.trades[1] == Trade{3, 5, worse_price, 2} && result.canceled == 3,
                "IOC multi-level remainder");
        require(test.submit({1, maker_side, 100, 1}).status == Status::duplicate,
                "filled ID reused");
    }
    CheckedEngine test;
    require(test.cancel(1) == 0, "unknown cancel");
    test.submit({1, Side::sell, 100, 2});
    const auto residual = test.submit({2, Side::buy, 101, 5});
    require(residual.executed == 2 && residual.resting == 3, "GTC remainder");
    require(test.cancel(2) == 3 && test.cancel(2) == 0, "cancel remainder");
    require(test.submit({2, Side::buy, 1, 1}).status == Status::duplicate, "canceled ID reused");
    for (const Input bad : {Input{0, Side::buy, 1, 1}, Input{65, Side::buy, 1, 1},
                           Input{3, Side::buy, 0, 1}, Input{3, Side::buy, 1001, 1},
                           Input{3, Side::buy, 1, 0}, Input{3, Side::buy, 1, 9},
                           Input{3, static_cast<Side>(7), 1, 1},
                           Input{3, Side::buy, 1, 1, static_cast<Tif>(7)}}) {
        require(test.submit(bad).status == Status::invalid, "invalid input accepted");
    }
    require(test.submit({64, Side::buy, 1, 8, Tif::ioc}).canceled == 8, "empty IOC");
    CheckedEngine full;
    for (unsigned id = 1; id <= capacity; ++id) {
        full.submit({id, Side::buy, 1, 1});
    }
    require(full.submit({9, Side::buy, 1, 1}).status == Status::full, "capacity boundary");
    require(full.cancel(1) == 1, "free slot");
    require(full.submit({9, Side::buy, 1000, 8}).status == Status::accepted, "rejected ID retry");
    CheckedEngine sweep;
    for (unsigned id = 1; id <= capacity; ++id) {
        sweep.submit({id, Side::buy, 100, 1});
    }
    const auto all = sweep.submit({9, Side::sell, 100, 8, Tif::ioc});
    require(all.trade_count == capacity && all.executed == 8, "maximum trade result capacity");
    CheckedEngine reuse;
    reuse.submit({1, Side::sell, 100, 1});
    reuse.submit({2, Side::sell, 100, 1});
    reuse.cancel(1);
    reuse.submit({3, Side::sell, 100, 1});
    require(reuse.submit({4, Side::buy, 100, 1}).trades[0].maker == 2,
            "slot reuse must not change FIFO");
    CheckedEngine exhausted;
    for (unsigned id = 1; id <= max_id; ++id) {
        require(exhausted.submit({id, Side::buy, 1, 1, Tif::ioc}).canceled == 1,
                "accepted IOC identity");
    }
    require(exhausted.engine.state().accepted_count == max_id, "bounded session ID count");
    require(exhausted.submit({1, Side::buy, 1, 1}).status == Status::duplicate,
            "exhausted session must not reuse ID");
    require(exhausted.submit({max_id + 1, Side::buy, 1, 1}).status == Status::invalid,
            "session ID domain boundary");
    CheckedEngine fault;
    fault.submit({1, Side::sell, 100, 5});
    const auto before = fault.engine.state();
    for (const unsigned quantity : {3U, 8U}) {
        bool caught = false;
        try {
            static_cast<void>(fault.engine.submit({2, Side::buy, 101, quantity}, true));
        } catch (const InjectedFailure&) {
            caught = true;
        }
        require(caught && fault.engine.state() == before, "precommit exception changed state");
    }
    fault.oracle.compare(fault.engine);
    require(fault.submit({2, Side::buy, 101, 3}).executed == 3, "retry after rollback");
}

std::uint32_t random_next(std::uint32_t& state) {
    state = state * 1664525U + 1013904223U;
    return state;
}

void bounded_traces() {
    std::uint32_t random = 0x62a1bc39U;
    for (unsigned trace = 0; trace < 128; ++trace) {
        CheckedEngine test;
        for (unsigned step = 0; step < 128; ++step) {
            const auto bits = random_next(random);
            const unsigned id = 1 + ((bits >> 3) % max_id);
            if ((bits % 5U) == 0) {
                test.cancel(id);
            } else {
                const Side side = (bits & 1U) != 0 ? Side::buy : Side::sell;
                const unsigned price = 95U + ((bits >> 12) % 11U);
                const unsigned quantity = 1U + ((bits >> 20) % max_quantity);
                const Tif tif = (bits & 6U) == 0 ? Tif::ioc : Tif::gtc;
                test.submit({id, side, price, quantity, tif});
            }
        }
    }
}
} // namespace

int main() {
    try {
        deterministic_tests();
        bounded_traces();
        std::cout << "Price/time, maker price, IOC/GTC, cancel, rollback: OK\n"
                  << "128 traces x 128 commands agree with unit-lot oracle.\n";
    } catch (const std::exception& error) {
        std::cerr << "ERROR: " << error.what() << '\n';
        return 1;
    }
}
