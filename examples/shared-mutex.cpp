#include <atomic>
#include <cassert>
#include <iostream>
#include <mutex>
#include <shared_mutex>
#include <thread>
#include <vector>

struct Snapshot { int version; int checksum; };
class Config {
    mutable std::shared_mutex mutex_;
    Snapshot state_{0, 0};
public:
    Snapshot read() const {
        std::shared_lock lock(mutex_);
        return state_; // copy before unlocking; never return an internal reference
    }
    bool replace_if_version(int expected, int next) {
        std::unique_lock lock(mutex_);
        if (state_.version != expected) return false;
        state_ = {next, -next};
        return true;
    }
};
int main() {
    Config config;
    std::atomic<bool> start{false};
    std::atomic<int> errors{0};
    std::vector<std::thread> readers;
    for (int r = 0; r < 4; ++r) readers.emplace_back([&] {
        while (!start.load(std::memory_order_acquire)) std::this_thread::yield();
        for (int i = 0; i < 5000; ++i) {
            const auto snapshot = config.read();
            if (snapshot.version + snapshot.checksum != 0) errors.fetch_add(1, std::memory_order_relaxed);
        }
    });
    std::thread writer([&] {
        start.store(true, std::memory_order_release);
        for (int i = 0; i < 5000; ++i) {
            const bool updated = config.replace_if_version(i, i + 1);
            assert(updated);
        }
    });
    writer.join();
    for (auto& reader : readers) reader.join();
    assert(errors.load() == 0);
    assert(config.read().version == 5000);
    assert(!config.replace_if_version(0, 9000)); // stale-version rejection
    std::cout << "consistent snapshots; version=5000; stale update rejected\n";
}
