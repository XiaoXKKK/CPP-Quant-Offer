#include <atomic>
#include <cassert>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <thread>

struct Packed { std::atomic<std::uint64_t> value{0}; };
struct alignas(64) Padded { std::atomic<std::uint64_t> value{0}; };
template <class Counter> long long run() {
    alignas(64) Counter counters[2];
    constexpr int iterations = 500000;
    std::atomic<int> ready{0};
    std::atomic<bool> go{false};
    auto worker = [&](std::size_t index) {
        ready.fetch_add(1, std::memory_order_release);
        while (!go.load(std::memory_order_acquire)) std::this_thread::yield();
        for (int n = 0; n < iterations; ++n) counters[index].value.fetch_add(1, std::memory_order_relaxed);
    };
    std::thread first(worker, 0), second(worker, 1);
    while (ready.load(std::memory_order_acquire) != 2) std::this_thread::yield();
    const auto begin = std::chrono::steady_clock::now();
    go.store(true, std::memory_order_release);
    first.join(); second.join();
    const auto end = std::chrono::steady_clock::now();
    assert(counters[0].value.load() == iterations);
    assert(counters[1].value.load() == iterations);
    return std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count();
}
int main() {
    std::cout << "chosen alignment=64; sizeof(packed)=" << sizeof(Packed)
              << "; sizeof(padded)=" << sizeof(Padded) << '\n';
    for (int trial = 0; trial < 4; ++trial) {
        long long packed, padded;
        if (trial % 2 == 0) { packed = run<Packed>(); padded = run<Padded>(); }
        else { padded = run<Padded>(); packed = run<Packed>(); }
        std::cout << "trial=" << trial << " packed_us=" << packed << " padded_us=" << padded << '\n';
    }
    std::cout << "No speedup assertion: pin distinct physical cores and inspect perf c2c for measurement.\n";
}
