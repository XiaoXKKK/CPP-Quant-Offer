#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <iostream>
#include <list>
#include <memory>
#include <new>
#include <optional>
#include <type_traits>
#include <unordered_map>
#include <utility>
#include <vector>

// Single-threaded test instrumentation, shared by allocator rebinds.
struct AllocationProbe {
    static inline std::optional<std::size_t> remaining;
    static inline std::size_t calls = 0;
    static inline std::size_t live_blocks = 0;
};

template <class T>
struct FaultAllocator {
    using value_type = T;
    using is_always_equal = std::true_type;

    FaultAllocator() noexcept = default;
    template <class U>
    FaultAllocator(const FaultAllocator<U>&) noexcept {}

    [[nodiscard]] T* allocate(std::size_t count) {
        ++AllocationProbe::calls;
        if (AllocationProbe::remaining) {
            if (*AllocationProbe::remaining == 0) throw std::bad_alloc{};
            --*AllocationProbe::remaining;
        }
        T* result = std::allocator<T>{}.allocate(count);
        ++AllocationProbe::live_blocks;
        return result;
    }

    void deallocate(T* pointer, std::size_t count) noexcept {
        assert(AllocationProbe::live_blocks > 0);
        --AllocationProbe::live_blocks;
        std::allocator<T>{}.deallocate(pointer, count);
    }

    template <class U>
    bool operator==(const FaultAllocator<U>&) const noexcept { return true; }
};

struct IntHash {
    std::size_t operator()(int key) const noexcept { return std::hash<int>{}(key); }
};

struct IntEqual {
    bool operator()(int left, int right) const noexcept { return left == right; }
};

using Entry = std::pair<int, int>;
struct CacheTestAccess;

class LruCache {
    using Order = std::list<Entry, FaultAllocator<Entry>>;
    using Position = Order::iterator;
    using IndexEntry = std::pair<const int, Position>;
    using Index = std::unordered_map<int, Position, IntHash, IntEqual,
                                     FaultAllocator<IndexEntry>>;

    std::size_t capacity_;
    Order order_;  // MRU at front; LRU at back.
    Index index_;  // Destroy index before the list to which its iterators refer.
    friend struct CacheTestAccess;

public:
    explicit LruCache(std::size_t capacity) : capacity_(capacity) {}
    LruCache(const LruCache&) = delete;
    LruCache& operator=(const LruCache&) = delete;
    LruCache(LruCache&&) = delete;
    LruCache& operator=(LruCache&&) = delete;

    [[nodiscard]] std::size_t size() const noexcept { return order_.size(); }

    [[nodiscard]] std::optional<int> get(int key) {
        const auto found = index_.find(key);
        if (found == index_.end()) return std::nullopt;
        const auto node = found->second;
        order_.splice(order_.begin(), order_, node);
        return node->second;  // An owning int copy, not a borrowed reference.
    }

    void put(int key, int value) {
        if (capacity_ == 0) return;
        if (const auto found = index_.find(key); found != index_.end()) {
            found->second->second = value;
            order_.splice(order_.begin(), order_, found->second);
            return;
        }

        order_.emplace_front(key, value);
        try {
            const bool inserted = index_.emplace(key, order_.begin()).second;
            assert(inserted);  // Fixed equality, no concurrency, and a preceding miss.
        } catch (...) {
            order_.pop_front();
            throw;
        }

        // Fixed int hash/equality/destruction and deallocation do not throw.
        // No map iterator obtained before emplace is used after a possible rehash.
        if (order_.size() > capacity_) {
            const auto erased = index_.erase(order_.back().first);
            assert(erased == 1);
            order_.pop_back();
        }
    }

    bool erase(int key) {
        const auto found = index_.find(key);
        if (found == index_.end()) return false;
        const auto node = found->second;
        index_.erase(found);
        order_.erase(node);
        return true;
    }

    [[nodiscard]] std::vector<Entry> snapshot() const {
        return {order_.begin(), order_.end()};
    }
};

struct CacheTestAccess {
    static void check(const LruCache& cache) {
        assert(cache.order_.size() == cache.index_.size());
        assert(cache.size() <= cache.capacity_);
        for (auto node = cache.order_.begin(); node != cache.order_.end(); ++node) {
            const auto found = cache.index_.find(node->first);
            assert(found != cache.index_.end());
            assert(found->second == node);
        }
    }

    static void rehash(LruCache& cache, std::size_t buckets) {
        cache.index_.rehash(buckets);
    }
};

// Independent, deliberately linear reference model: no linked nodes or stored iterators.
class Oracle {
    std::size_t capacity_;
    std::vector<Entry> entries_;

public:
    explicit Oracle(std::size_t capacity) : capacity_(capacity) {}
    std::optional<int> get(int key) {
        const auto found = std::find_if(entries_.begin(), entries_.end(),
                                        [key](const Entry& entry) { return entry.first == key; });
        if (found == entries_.end()) return std::nullopt;
        const Entry entry = *found;
        entries_.erase(found);
        entries_.insert(entries_.begin(), entry);
        return entry.second;
    }
    void put(int key, int value) {
        if (capacity_ == 0) return;
        erase(key);
        entries_.insert(entries_.begin(), Entry{key, value});
        if (entries_.size() > capacity_) entries_.pop_back();
    }
    bool erase(int key) {
        const auto old_size = entries_.size();
        std::erase_if(entries_, [key](const Entry& entry) { return entry.first == key; });
        return old_size != entries_.size();
    }
    const std::vector<Entry>& snapshot() const noexcept { return entries_; }
};

void test_boundaries() {
    LruCache disabled(0);
    disabled.put(1, 10);
    assert(disabled.size() == 0 && !disabled.get(1) && !disabled.erase(1));

    LruCache single(1);
    single.put(1, 10);
    single.put(1, 11);
    assert(single.size() == 1 && single.get(1) == 11);
    single.put(2, 20);
    assert(!single.get(1) && single.get(2) == 20);
    assert(single.erase(2) && !single.erase(2));

    LruCache cache(2);
    cache.put(1, 10);
    cache.put(2, 20);
    const auto saved_value = cache.get(1);
    cache.put(3, 30);
    assert(!cache.get(2));
    cache.put(3, 31);
    cache.put(4, 40);
    assert(!cache.get(1) && saved_value == 10);
    assert((cache.snapshot() == std::vector<Entry>{{4, 40}, {3, 31}}));
    CacheTestAccess::check(disabled);
    CacheTestAccess::check(single);
    CacheTestAccess::check(cache);
}

void test_against_oracle() {
    for (std::size_t capacity = 0; capacity <= 4; ++capacity) {
        LruCache cache(capacity);
        Oracle oracle(capacity);
        std::uint32_t state = 0x12345678U;
        for (std::size_t step = 0; step < 2000; ++step) {
            state = state * 1664525U + 1013904223U;  // Defined unsigned wraparound.
            const int key = static_cast<int>((state >> 8U) % 7U) - 3;
            const int value = static_cast<int>((state >> 16U) % 1000U);
            switch (state % 4U) {
            case 0:
            case 1:
                cache.put(key, value);
                oracle.put(key, value);
                break;
            case 2:
                assert(cache.get(key) == oracle.get(key));
                break;
            default:
                assert(cache.erase(key) == oracle.erase(key));
                break;
            }
            if (step % 31 == 0) {
                CacheTestAccess::rehash(cache, 1 + step % 257);
            }
            CacheTestAccess::check(cache);
            assert(cache.snapshot() == oracle.snapshot());
        }
    }
}

void seed_cache(LruCache& cache, bool full) {
    if (full) {
        cache.put(1, 10);
        cache.put(2, 20);
    }
}

void test_allocation_failures() {
    for (bool full : {false, true}) {
        std::size_t allocation_count = 0;
        {
            LruCache cache(2);
            seed_cache(cache, full);
            const auto before = AllocationProbe::calls;
            cache.put(3, 30);
            allocation_count = AllocationProbe::calls - before;
        }
        assert(allocation_count > 0);
        for (std::size_t fail_at = 0; fail_at < allocation_count; ++fail_at) {
            LruCache cache(2);
            seed_cache(cache, full);
            const auto before = cache.snapshot();
            const auto blocks_before = AllocationProbe::live_blocks;
            AllocationProbe::remaining = fail_at;
            bool caught = false;
            try {
                cache.put(3, 30);
            } catch (const std::bad_alloc&) {
                caught = true;
            }
            AllocationProbe::remaining.reset();
            assert(caught);
            assert(cache.snapshot() == before);
            assert(AllocationProbe::live_blocks == blocks_before);
            CacheTestAccess::check(cache);
            cache.put(3, 30);  // The failed operation did not make the cache unusable.
            assert(cache.get(3) == 30);
        }
    }
}

int main() {
    static_assert(!std::is_copy_constructible_v<LruCache>);
    static_assert(!std::is_move_constructible_v<LruCache>);
    test_boundaries();
    test_against_oracle();
    test_allocation_failures();
    assert(AllocationProbe::live_blocks == 0);
    std::cout << "LRU boundaries, 10000 oracle steps, rehash and allocation rollback: OK\n";
}
