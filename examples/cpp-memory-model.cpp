#include <array>
#include <atomic>
#include <cassert>
#include <cstddef>
#include <iostream>
#include <thread>

// Educational SPSC: exactly one producer and one consumer, no concurrent reset.
template <std::size_t Capacity> class Spsc {
    static_assert(Capacity > 0);
    static constexpr std::size_t slots = Capacity + 1;
    std::array<int, slots> data_{};
    // 64 is a chosen alignment, not a portable assertion about cache-line size.
    alignas(64) std::atomic<std::size_t> head_{0};
    alignas(64) std::atomic<std::size_t> tail_{0};
public:
    bool push(int value) {
        const auto head = head_.load(std::memory_order_relaxed);
        const auto next = (head + 1) % slots;
        if (next == tail_.load(std::memory_order_acquire)) return false;
        data_[head] = value;
        head_.store(next, std::memory_order_release);
        return true;
    }
    bool pop(int& value) {
        const auto tail = tail_.load(std::memory_order_relaxed);
        if (tail == head_.load(std::memory_order_acquire)) return false;
        value = data_[tail];
        tail_.store((tail + 1) % slots, std::memory_order_release);
        return true;
    }
};
int main() {
    Spsc<2> small;
    int value = -1;
    assert(!small.pop(value));
    assert(small.push(7));
    assert(small.push(8));
    assert(!small.push(9));
    assert(small.pop(value) && value == 7);
    assert(small.push(9)); // wrap-around
    assert(small.pop(value) && value == 8);
    assert(small.pop(value) && value == 9);
    assert(!small.pop(value));
    Spsc<8> queue;
    constexpr int count = 100000;
    std::thread producer([&] {
        for (int i = 0; i < count; ++i) while (!queue.push(i)) std::this_thread::yield();
    });
    std::thread consumer([&] {
        for (int expected = 0; expected < count; ++expected) {
            int received;
            while (!queue.pop(received)) std::this_thread::yield();
            assert(received == expected);
        }
    });
    producer.join();
    consumer.join();
    std::cout << "SPSC: " << count << " ordered values; atomic<size_t> always lock-free="
              << std::atomic<std::size_t>::is_always_lock_free << '\n';
}
