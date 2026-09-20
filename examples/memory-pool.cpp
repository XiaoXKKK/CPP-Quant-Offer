#include <array>
#include <cassert>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <limits>
#include <memory>
#include <optional>
#include <stdexcept>
#include <type_traits>
#include <utility>

// Single-threaded. T constructors/destructors must not reenter this pool.
// Handles and borrowed pointers may only be used while their pool is alive.
template <class T, std::size_t N, std::unsigned_integral Generation = std::uint32_t>
class TypedPool {
    static_assert(N > 0);
    static_assert(!std::is_same_v<Generation, bool>);
    static_assert(!std::is_array_v<T> && !std::is_const_v<T> && !std::is_volatile_v<T>);
    static_assert(std::is_nothrow_destructible_v<T>);

    struct Slot {
        alignas(T) std::byte storage[sizeof(T)];
        std::size_t next = N;
        Generation generation = 1;
        T* object = nullptr;
        bool live = false;
    };

public:
    class Handle {
        friend class TypedPool;
        const TypedPool* owner_ = nullptr;
        std::size_t index_ = N;
        Generation generation_ = 0;

        Handle(const TypedPool* owner, std::size_t index, Generation generation)
            : owner_(owner), index_(index), generation_(generation) {}

    public:
        Handle() = default;
    };

    TypedPool() noexcept {
        for (std::size_t i = 0; i < N; ++i) {
            slots_[i].next = i + 1;
        }
    }
    TypedPool(const TypedPool&) = delete;
    TypedPool& operator=(const TypedPool&) = delete;
    TypedPool(TypedPool&&) = delete;
    TypedPool& operator=(TypedPool&&) = delete;

    ~TypedPool() noexcept {
        // Slot order, not reverse construction order.
        for (auto& slot : slots_) {
            if (slot.live) {
                std::destroy_at(slot.object);
            }
        }
    }

    template <class... Args>
    std::optional<Handle> try_emplace(Args&&... args) {
        if (free_head_ == N) {
            return std::nullopt;
        }
        const auto index = free_head_;
        auto& slot = slots_[index];
        free_head_ = slot.next;
        --free_count_;
        try {
            slot.object = std::construct_at(reinterpret_cast<T*>(slot.storage),
                                            std::forward<Args>(args)...);
        } catch (...) {
            slot.next = free_head_;
            free_head_ = index;
            ++free_count_;
            throw;
        }
        slot.live = true;
        ++live_count_;
        return Handle(this, index, slot.generation);
    }

    T* get(const Handle& handle) noexcept {
        if (!matches(handle)) {
            return nullptr;
        }
        return slots_[handle.index_].object;
    }

    bool destroy(const Handle& handle) noexcept {
        if (!matches(handle)) {
            return false;
        }
        auto& slot = slots_[handle.index_];
        T* object = slot.object;
        slot.live = false;
        slot.object = nullptr;
        std::destroy_at(object);
        --live_count_;
        if (slot.generation == std::numeric_limits<Generation>::max()) {
            ++retired_count_;
        } else {
            ++slot.generation;
            slot.next = free_head_;
            free_head_ = handle.index_;
            ++free_count_;
        }
        return true;
    }

    std::size_t available() const noexcept { return free_count_; }
    std::size_t live() const noexcept { return live_count_; }
    std::size_t retired() const noexcept { return retired_count_; }

private:
    bool matches(const Handle& handle) const noexcept {
        return handle.owner_ == this && handle.index_ < N &&
               slots_[handle.index_].live &&
               slots_[handle.index_].generation == handle.generation_;
    }

    std::array<Slot, N> slots_{};
    std::size_t free_head_ = 0;
    std::size_t free_count_ = N;
    std::size_t live_count_ = 0;
    std::size_t retired_count_ = 0;
};

struct Resource {
    inline static int alive = 0;
    std::unique_ptr<int> value;

    explicit Resource(int input) : value(std::make_unique<int>(input)) { ++alive; }
    ~Resource() noexcept { --alive; }
    Resource(const Resource&) = delete;
    Resource& operator=(const Resource&) = delete;
};

struct alignas(64) Tracked {
    inline static int alive = 0;
    inline static int constructed = 0;
    inline static int destroyed = 0;
    inline static int attempts = 0;
    Resource resource;

    explicit Tracked(int input, bool fail = false) : resource(input) {
        ++attempts;
        if (fail) {
            throw std::runtime_error("injected constructor failure");
        }
        ++alive;
        ++constructed;
    }
    ~Tracked() noexcept {
        --alive;
        ++destroyed;
    }
    int value() const noexcept { return *resource.value; }
};

static_assert(alignof(Tracked) >= 64);

void check_lifecycle() {
    using Pool = TypedPool<Tracked, 2>;
    {
        Pool pool;
        const auto first = pool.try_emplace(11);
        assert(first && pool.live() == 1 && pool.available() == 1);
        bool caught = false;
        try {
            static_cast<void>(pool.try_emplace(22, true));
        } catch (const std::runtime_error&) {
            caught = true;
        }
        assert(caught && pool.live() == 1 && pool.available() == 1);
        assert(Tracked::alive == 1 && Resource::alive == 1);
        assert(Tracked::constructed == 1 && Tracked::destroyed == 0);
        assert(pool.get(*first)->value() == 11);

        const auto second = pool.try_emplace(22);
        assert(second && pool.live() == 2 && pool.available() == 0);
        const int attempts = Tracked::attempts;
        assert(!pool.try_emplace(44) && Tracked::attempts == attempts);

        void* address = pool.get(*second);
        void* aligned = address;
        std::size_t space = sizeof(Tracked);
        assert(std::align(alignof(Tracked), sizeof(Tracked), aligned, space) == address);

        const auto stale = *first;
        assert(pool.destroy(*first));
        assert(pool.get(stale) == nullptr && !pool.destroy(stale));
        const auto replacement = pool.try_emplace(33);
        assert(replacement && pool.get(*replacement)->value() == 33);
        assert(pool.get(stale) == nullptr && !pool.destroy(stale));
        const Pool::Handle invalid;
        assert(pool.get(invalid) == nullptr && !pool.destroy(invalid));
        Pool other;
        assert(other.get(*replacement) == nullptr && !other.destroy(*replacement));
        assert(other.live() == 0 && other.available() == 2);
        assert(pool.live() + pool.available() + pool.retired() == 2);
        // Remaining objects are destroyed by pool; no borrowed pointer escapes.
    }
    assert(Tracked::alive == 0 && Resource::alive == 0);
    assert(Tracked::attempts == 4 && Tracked::constructed == 3 && Tracked::destroyed == 3);
    std::cout << "lifecycle: 3 constructed, 3 destroyed, 1 constructor failure\n";
}

void check_generation_retirement() {
    using Pool = TypedPool<Tracked, 1, std::uint8_t>;
    Pool pool;
    std::optional<Pool::Handle> first;
    const int constructed_before = Tracked::constructed;
    const int destroyed_before = Tracked::destroyed;
    constexpr unsigned limit = std::numeric_limits<std::uint8_t>::max();
    for (unsigned cycle = 1; cycle <= limit; ++cycle) {
        const auto handle = pool.try_emplace(static_cast<int>(cycle));
        assert(handle && pool.get(*handle)->value() == static_cast<int>(cycle));
        if (cycle == 1) {
            first = handle;
        }
        assert(pool.destroy(*handle));
        assert(pool.get(*first) == nullptr && !pool.destroy(*first));
        assert(pool.live() == 0);
        assert(pool.available() == (cycle < limit ? 1U : 0U));
        assert(pool.retired() == (cycle < limit ? 0U : 1U));
    }
    assert(!pool.try_emplace(999));
    assert(Tracked::constructed - constructed_before == static_cast<int>(limit));
    assert(Tracked::destroyed - destroyed_before == static_cast<int>(limit));
    assert(Tracked::alive == 0 && Resource::alive == 0);
    std::cout << "generation: 255 lifetimes, 1 retired slot, no wrap\n";
}

int main() {
    check_lifecycle();
    check_generation_retirement();
    std::cout << "alignment: Tracked=" << alignof(Tracked)
              << " max_align_t=" << alignof(std::max_align_t) << '\n';
    std::cout << "all typed-pool checks passed\n";
}
