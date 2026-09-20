#include <array>
#include <atomic>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <memory>
#include <optional>
#include <thread>
#include <type_traits>
#include <utility>
#include <vector>

// Exactly one producer and one consumer. No concurrent reset or destruction.
// Slots counts physical slots; one slot is reserved to distinguish full/empty.
template <class T, std::size_t Slots>
class SpscQueue {
    static_assert(Slots >= 2);
    static_assert(std::is_nothrow_move_constructible_v<T>);
    static_assert(std::is_nothrow_destructible_v<T>);

    std::array<std::optional<T>, Slots> slots_{};
    std::atomic<std::size_t> write_{0};
    std::atomic<std::size_t> read_{0};

    static constexpr std::size_t next(std::size_t index) noexcept {
        return index + 1 == Slots ? 0 : index + 1;
    }

public:
    static constexpr std::size_t capacity = Slots - 1;
    SpscQueue() = default;
    SpscQueue(const SpscQueue&) = delete;
    SpscQueue& operator=(const SpscQueue&) = delete;

    // On false, item is untouched. Only the producer may call this operation.
    bool try_push(T&& item) noexcept {
        const auto write = write_.load(std::memory_order_relaxed);
        const auto after = next(write);
        if (after == read_.load(std::memory_order_acquire)) return false;
        assert(!slots_[write].has_value());
        slots_[write].emplace(std::move(item));
        write_.store(after, std::memory_order_release);
        return true;
    }

    // Returns an owning value, never a reference into storage about to be reused.
    std::optional<T> try_pop() noexcept {
        const auto read = read_.load(std::memory_order_relaxed);
        if (read == write_.load(std::memory_order_acquire)) return std::nullopt;
        assert(slots_[read].has_value());
        std::optional<T> result(std::in_place, std::move(*slots_[read]));
        slots_[read].reset();
        read_.store(next(read), std::memory_order_release);
        return result;
    }
};

struct Counters {
    std::atomic<std::size_t> created{0};
    std::atomic<std::size_t> destroyed{0};
    std::atomic<std::size_t> live{0};
};

struct Resource {
    Counters& counters;
    std::uint64_t sequence;
    std::uint64_t checksum;

    Resource(Counters& counts, std::uint64_t id) noexcept
        : counters(counts), sequence(id), checksum(id ^ 0x5a5aULL) {
        counters.created.fetch_add(1, std::memory_order_relaxed);
        counters.live.fetch_add(1, std::memory_order_relaxed);
    }
    ~Resource() noexcept {
        counters.destroyed.fetch_add(1, std::memory_order_relaxed);
        counters.live.fetch_sub(1, std::memory_order_relaxed);
    }
};

struct Packet {
    std::unique_ptr<Resource> resource;
    Packet(Counters& counts, std::uint64_t id)
        : resource(std::make_unique<Resource>(counts, id)) {}
    Packet(Packet&&) noexcept = default;
    Packet& operator=(Packet&&) noexcept = default;
    Packet(const Packet&) = delete;
    Packet& operator=(const Packet&) = delete;
    ~Packet() = default;
};

static_assert(!std::is_default_constructible_v<Packet>);
static_assert(!std::is_copy_constructible_v<Packet>);
static_assert(std::is_nothrow_move_constructible_v<Packet>);
static_assert(std::is_nothrow_destructible_v<Packet>);

void check_packet(const Packet& packet, std::uint64_t expected) {
    assert(packet.resource);
    assert(packet.resource->sequence == expected);
    assert(packet.resource->checksum == (expected ^ 0x5a5aULL));
}

template <std::size_t Slots>
void boundary_test(Counters& counts) {
    SpscQueue<Packet, Slots> queue;
    constexpr auto capacity = decltype(queue)::capacity;
    for (std::size_t cycle = 0; cycle < 128; ++cycle) {
        assert(!queue.try_pop());
        for (std::size_t i = 0; i < capacity; ++i) {
            Packet packet(counts, i);
            const bool pushed = queue.try_push(std::move(packet));
            assert(pushed && !packet.resource);
        }
        Packet pending(counts, capacity);
        const auto* owned = pending.resource.get();
        const bool full_push = queue.try_push(std::move(pending));
        assert(!full_push && pending.resource.get() == owned);
        check_packet(pending, capacity);
        {
            auto first = queue.try_pop();
            assert(first);
            check_packet(*first, 0);
        }
        const bool retried = queue.try_push(std::move(pending));
        assert(retried && !pending.resource);
        for (std::size_t expected = 1; expected <= capacity; ++expected) {
            auto packet = queue.try_pop();
            assert(packet);
            check_packet(*packet, expected);
        }
        assert(!queue.try_pop());
        assert(counts.live.load(std::memory_order_relaxed) == 0);
    }
}

void undrained_destruction_test(Counters& counts) {
    {
        SpscQueue<Packet, 3> queue;
        Packet first(counts, 10);
        Packet second(counts, 11);
        const bool a = queue.try_push(std::move(first));
        const bool b = queue.try_push(std::move(second));
        assert(a && b && counts.live.load(std::memory_order_relaxed) == 2);
        // No thread is active: destroying engaged optional slots is now safe.
    }
    assert(counts.live.load(std::memory_order_relaxed) == 0);
}

template <std::size_t Slots>
void transfer_test(Counters& counts) {
    constexpr std::size_t count = 6000;
    // Allocation happens before thread startup; the queue only moves ownership.
    std::vector<Packet> input;
    input.reserve(count);
    for (std::size_t i = 0; i < count; ++i) input.emplace_back(counts, i);
    SpscQueue<Packet, Slots> queue;
    std::jthread producer([&] {
        for (auto& packet : input) {
            while (!queue.try_push(std::move(packet))) std::this_thread::yield();
            assert(!packet.resource);
        }
    });
    // The calling thread is the sole consumer. Both sides know the exact count.
    for (std::size_t expected = 0; expected < count;) {
        auto packet = queue.try_pop();
        if (!packet) {
            std::this_thread::yield();
            continue;
        }
        check_packet(*packet, expected);
        ++expected;
    }
    producer.join();
    assert(!queue.try_pop());
    assert(counts.live.load(std::memory_order_relaxed) == 0);
    assert(counts.created.load(std::memory_order_relaxed) ==
           counts.destroyed.load(std::memory_order_relaxed));
}

int main() {
    Counters counts;
    boundary_test<2>(counts);
    boundary_test<3>(counts);
    boundary_test<5>(counts);
    undrained_destruction_test(counts);
    for (int round = 0; round < 3; ++round) {
        transfer_test<2>(counts);
        transfer_test<3>(counts);
        transfer_test<17>(counts);
    }
    assert(counts.created.load(std::memory_order_relaxed) == 55282);
    assert(counts.destroyed.load(std::memory_order_relaxed) == 55282);
    std::cout << "SPSC checks passed; resources="
              << counts.destroyed.load(std::memory_order_relaxed)
              << "; always_lock_free=" << std::atomic<std::size_t>::is_always_lock_free
              << '\n';
}
