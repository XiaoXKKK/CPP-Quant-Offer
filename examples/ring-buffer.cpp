#include <array>
#include <cstddef>
#include <deque>
#include <iostream>
#include <memory>
#include <optional>
#include <stdexcept>
#include <type_traits>
#include <utility>

namespace {
// Single-owner, non-reentrant operations: T construction/destruction must not
// call this ring. Construction failure preserves occupancy and cursors; effects
// through aliased arguments or external state remain T's responsibility.
template<class T, std::size_t N>
class RingBuffer {
    static_assert(N > 0, "RingBuffer capacity must be positive");
    static_assert(std::is_nothrow_destructible_v<T>, "RingBuffer requires nonthrowing destruction");
public:
    RingBuffer() = default;
    RingBuffer(const RingBuffer&) = delete;
    RingBuffer& operator=(const RingBuffer&) = delete;
    RingBuffer(RingBuffer&&) = delete;
    RingBuffer& operator=(RingBuffer&&) = delete;
    ~RingBuffer() noexcept { clear(); }

    [[nodiscard]] std::size_t size() const noexcept { return size_; }
    [[nodiscard]] bool empty() const noexcept { return size_ == 0; }
    [[nodiscard]] bool full() const noexcept { return size_ == N; }
    static constexpr std::size_t capacity() noexcept { return N; }

    template<class... Args>
    bool try_emplace(Args&&... args) {
        if (full()) { return false; }
        // Invariant: tail denotes a disengaged slot whenever size < N.
        slots_[tail_].emplace(std::forward<Args>(args)...);
        // If T construction throws, no cursor or count has changed.
        tail_ = advance(tail_);
        ++size_;
        return true;
    }

    T* front() noexcept { return at(0); }
    const T* front() const noexcept { return at(0); }
    T* at(std::size_t logical_index) noexcept {
        if (logical_index >= size_) { return nullptr; }
        return std::addressof(*slots_[physical_index(logical_index)]);
    }
    const T* at(std::size_t logical_index) const noexcept {
        if (logical_index >= size_) { return nullptr; }
        return std::addressof(*slots_[physical_index(logical_index)]);
    }
    bool pop_front() noexcept {
        if (empty()) { return false; }
        slots_[head_].reset();
        head_ = advance(head_);
        --size_;
        return true;
    }
    void clear() noexcept {
        while (pop_front()) {}
    }

private:
    static constexpr std::size_t advance(std::size_t index) noexcept {
        return index == N - 1 ? 0 : index + 1;
    }
    std::size_t physical_index(std::size_t offset) const noexcept {
        // Equivalent to (head + offset) % N, without overflowing head + offset.
        const auto until_end = N - head_;
        return offset < until_end ? head_ + offset : offset - until_end;
    }
    std::array<std::optional<T>, N> slots_{};
    std::size_t head_ = 0;
    std::size_t tail_ = 0;
    std::size_t size_ = 0;
};

void require(bool condition, const char* message) {
    if (!condition) { throw std::runtime_error(message); }
}

struct ConstructionFailure {};
struct Payload {
    static inline int live = 0;
    int value;
    explicit Payload(int value_in) : value(value_in) { ++live; }
    Payload(const Payload&) = delete;
    Payload& operator=(const Payload&) = delete;
    ~Payload() noexcept { --live; }
};
struct Item {
    static inline int live = 0;
    static inline int constructed = 0;
    static inline int destroyed = 0;
    std::unique_ptr<Payload> payload;
    explicit Item(int value) : payload(std::make_unique<Payload>(value)) {
        if (value == -1) { throw ConstructionFailure{}; }
        ++live;
        ++constructed;
    }
    Item(const Item&) = delete;
    Item& operator=(const Item&) = delete;
    Item(Item&&) = delete;
    Item& operator=(Item&&) = delete;
    ~Item() noexcept { --live; ++destroyed; }
    int value() const noexcept { return payload->value; }
};

void lifetime_tests() {
    require(Item::live == 0 && Payload::live == 0, "initial lifetime counters");
    {
        RingBuffer<Item, 3> ring;
        require(ring.empty() && !ring.front() && !ring.pop_front(), "empty behavior");
        require(ring.try_emplace(1), "first emplacement");
        Item* borrowed = ring.front();
        require(ring.try_emplace(2) && ring.try_emplace(3), "fill");
        require(ring.front() == borrowed && borrowed->value() == 1, "insertion moved a live item");
        require(!ring.try_emplace(-1) && Item::live == 3 && Payload::live == 3,
                "full insertion must not construct T");
        require(ring.pop_front(), "pop destroys first");
        // borrowed is invalid now; never dereference it again.
        require(Item::live == 2 && Payload::live == 2, "pop lifetime");
        const auto before_constructed = Item::constructed;
        bool caught = false;
        try { static_cast<void>(ring.try_emplace(-1)); }
        catch (const ConstructionFailure&) { caught = true; }
        require(caught && ring.size() == 2 && ring.front()->value() == 2 &&
                    ring.at(1)->value() == 3 && Item::constructed == before_constructed &&
                    Item::live == 2 && Payload::live == 2, "constructor rollback");
        require(ring.try_emplace(4) && ring.at(2)->value() == 4, "retry into wrapped slot");
        const auto& view = ring;
        require(view.front()->value() == 2 && !view.at(3), "const view and bounds");
        for (const int expected : {2, 3, 4}) {
            require(ring.front()->value() == expected && ring.pop_front(), "FIFO after wrap");
        }
        require(ring.empty() && Item::live == 0 && Payload::live == 0, "drain lifetime");
        ring.try_emplace(5);
        ring.try_emplace(6);
        ring.clear();
        ring.clear();
        require(ring.empty() && Item::live == 0 && Payload::live == 0, "clear lifetime");
        ring.try_emplace(7); // Destructor must clean the remaining object.
    }
    require(Item::live == 0 && Payload::live == 0 && Item::constructed == Item::destroyed,
            "final destruction accounting");
    RingBuffer<Item, 1> single;
    for (int value = 0; value < 20; ++value) {
        require(single.try_emplace(value) && single.full() && !single.try_emplace(-1), "capacity one full");
        require(single.front()->value() == value && single.pop_front() && single.empty(), "capacity one wrap");
    }
}

template<std::size_t N>
void compare(const RingBuffer<int, N>& ring, const std::deque<int>& oracle) {
    require(ring.size() == oracle.size() && ring.empty() == oracle.empty() &&
                ring.full() == (oracle.size() == N), "size state mismatch");
    for (std::size_t i = 0; i < oracle.size(); ++i) {
        require(ring.at(i) && *ring.at(i) == oracle[i], "FIFO content mismatch");
    }
    require(!ring.at(oracle.size()), "out-of-range view");
}

template<std::size_t N>
void exhaustive_traces() {
    constexpr unsigned length = 8;
    constexpr unsigned traces = 6561; // 3^8: push, pop, clear.
    for (unsigned trace = 0; trace < traces; ++trace) {
        RingBuffer<int, N> ring;
        std::deque<int> oracle;
        unsigned operations = trace;
        for (unsigned step = 0; step < length; ++step) {
            const auto operation = operations % 3U;
            operations /= 3U;
            if (operation == 0) {
                const bool accepted = oracle.size() < N;
                require(ring.try_emplace(static_cast<int>(step)) == accepted, "push result");
                if (accepted) { oracle.push_back(static_cast<int>(step)); }
            } else if (operation == 1) {
                const bool removed = !oracle.empty();
                require(ring.pop_front() == removed, "pop result");
                if (removed) { oracle.pop_front(); }
            } else {
                ring.clear();
                oracle.clear();
            }
            compare(ring, oracle);
        }
    }
}

void repeated_wrap() {
    RingBuffer<int, 3> ring;
    std::deque<int> oracle;
    for (int i = 0; i < 10000; ++i) {
        if (oracle.size() == 3) {
            require(ring.pop_front(), "steady-state pop");
            oracle.pop_front();
        }
        require(ring.try_emplace(i), "steady-state push");
        oracle.push_back(i);
        compare(ring, oracle);
    }
}
} // namespace

int main() {
    try {
        lifetime_tests();
        exhaustive_traces<1>();
        exhaustive_traces<3>();
        repeated_wrap();
        require(Item::live == 0 && Payload::live == 0 && Item::constructed == Item::destroyed,
                "all tracked objects reclaimed");
        std::cout << "Nonmovable lifetime, constructor rollback, capacity 1/3: OK\n"
                  << "2 x 6561 exhaustive traces and 10000 wrap steps agree with deque.\n";
    } catch (const std::exception& error) {
        std::cerr << "ERROR: " << error.what() << '\n';
        return 1;
    }
}
