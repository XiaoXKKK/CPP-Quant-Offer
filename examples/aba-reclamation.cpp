#include <array>
#include <atomic>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <exception>
#include <iostream>
#include <memory>
#include <new>
#include <optional>
#include <thread>
#include <utility>
#include <vector>

// All slots stay alive. Only the logical stack membership becomes incorrect.
void check_index_aba() {
    std::array<int, 3> next{1, 2, -1};  // A -> B -> C
    std::array<bool, 3> member{true, true, true};
    std::atomic<int> head{0};
    int expected = head.load();
    const int stale_next = next[static_cast<std::size_t>(expected)];

    head.store(1);  // Another logical operation removes A.
    member[0] = false;
    head.store(2);  // It then removes B.
    member[1] = false;
    next[0] = 2;
    member[0] = true;
    head.store(0);  // A is reinserted, now followed by C.

    const bool accepted = head.compare_exchange_strong(expected, stale_next);
    assert(accepted);
    assert(head.load() == 1 && !member[1]);  // Resurrected the removed B.
}

constexpr std::uint32_t tagged(unsigned index, unsigned generation) {
    return static_cast<std::uint32_t>((generation & 3U) * 256U + index);
}

void check_tag_and_wrap() {
    std::atomic<std::uint32_t> head{tagged(0, 0)};
    auto expected = head.load();
    head.store(tagged(1, 1));
    head.store(tagged(2, 2));
    head.store(tagged(0, 3));
    assert(!head.compare_exchange_strong(expected, tagged(1, 1)));

    // Independent two-bit counter model: four changes alias the old identity.
    head.store(tagged(0, 0));
    expected = head.load();
    head.store(tagged(1, 1));
    head.store(tagged(2, 2));
    head.store(tagged(1, 3));
    head.store(tagged(0, 0));
    assert(head.compare_exchange_strong(expected, tagged(2, 1)));
}

void check_ownership_comparison() {
    int payload = 7;  // Outlives every alias below; no alias deletes this int.
    auto owner_a = std::make_shared<int>(1);
    auto owner_b = std::make_shared<int>(2);
    std::shared_ptr<int> a(owner_a, &payload);
    std::shared_ptr<int> b(owner_b, &payload);
    assert(a.get() == b.get());
    assert(a.owner_before(b) || b.owner_before(a));
    std::atomic<std::shared_ptr<int>> current{a};
    auto expected = b;
    assert(!current.compare_exchange_strong(expected, std::shared_ptr<int>{}));
    assert(!expected.owner_before(a) && !a.owner_before(expected));
    assert(current.load().get() == &payload);
}

struct AllocationFault {
    static inline std::atomic<bool> fail_next{false};
};

template <class T>
struct TestAllocator {
    using value_type = T;
    TestAllocator() noexcept = default;
    template <class U>
    TestAllocator(const TestAllocator<U>&) noexcept {}

    T* allocate(std::size_t count) {
        if (AllocationFault::fail_next.exchange(false, std::memory_order_relaxed)) {
            throw std::bad_alloc{};
        }
        return std::allocator<T>{}.allocate(count);
    }

    void deallocate(T* pointer, std::size_t count) noexcept {
        std::allocator<T>{}.deallocate(pointer, count);
    }

    template <class U>
    bool operator==(const TestAllocator<U>&) const noexcept {
        return true;
    }
};

struct Node {
    using Ptr = std::shared_ptr<const Node>;
    const int value;
    const Ptr next;
    static inline std::atomic<std::size_t> created{0};
    static inline std::atomic<std::size_t> destroyed{0};
    static inline std::atomic<std::size_t> live{0};

    Node(int item, Ptr tail) noexcept : value(item), next(std::move(tail)) {
        created.fetch_add(1, std::memory_order_relaxed);
        live.fetch_add(1, std::memory_order_relaxed);
    }
    ~Node() {
        destroyed.fetch_add(1, std::memory_order_relaxed);
        live.fetch_sub(1, std::memory_order_relaxed);
    }
};

struct StackTestAccess;

// Ownership baseline, not a lock-free or allocation-free implementation.
class OwnedStack {
public:
    OwnedStack() = default;
    OwnedStack(const OwnedStack&) = delete;
    OwnedStack& operator=(const OwnedStack&) = delete;
    OwnedStack(OwnedStack&&) = delete;
    OwnedStack& operator=(OwnedStack&&) = delete;

    void push(int value) {
        auto old = head_.load(std::memory_order_acquire);
        for (;;) {
            Node::Ptr candidate =
                std::allocate_shared<Node>(TestAllocator<Node>{}, value, old);
            if (head_.compare_exchange_strong(old, candidate,
                                             std::memory_order_acq_rel,
                                             std::memory_order_acquire)) {
                return;
            }
            // old owns the latest observed head. Build a fresh immutable node.
        }
    }

    std::optional<int> pop() noexcept {
        auto old = head_.load(std::memory_order_acquire);
        while (old) {
            const auto next = old->next;  // old keeps the node alive here.
            const int value = old->value;
            if (head_.compare_exchange_strong(old, next,
                                             std::memory_order_acq_rel,
                                             std::memory_order_acquire)) {
                return value;
            }
        }
        return std::nullopt;
    }

    Node::Ptr snapshot() const noexcept {
        return head_.load(std::memory_order_acquire);
    }

    bool head_is_lock_free() const noexcept { return head_.is_lock_free(); }

private:
    std::atomic<Node::Ptr> head_{};
    friend struct StackTestAccess;
};

struct StackTestAccess {
    static bool stale_pop(OwnedStack& stack, Node::Ptr expected) noexcept {
        assert(expected);
        const auto next = expected->next;
        return stack.head_.compare_exchange_strong(expected, next,
                                                  std::memory_order_acq_rel,
                                                  std::memory_order_acquire);
    }
};

void assert_all_reclaimed() {
    assert(Node::live.load() == 0);
    assert(Node::created.load() == Node::destroyed.load());
}

void check_lifetime_and_failure() {
    assert_all_reclaimed();
    OwnedStack stack;
    assert(!stack.pop());
    stack.push(10);
    stack.push(20);
    auto held = stack.snapshot();
    assert(Node::live.load() == 2);

    AllocationFault::fail_next.store(true);
    bool caught = false;
    try {
        stack.push(30);
    } catch (const std::bad_alloc&) {
        caught = true;
    }
    assert(caught && !AllocationFault::fail_next.load());
    assert(stack.snapshot() == held && Node::live.load() == 2);

    assert(stack.pop() == 20);
    assert(stack.pop() == 10);
    assert(!stack.pop());
    assert(Node::live.load() == 2);  // held still owns the entire old suffix.
    held.reset();
    assert_all_reclaimed();

    stack.push(7);
    held = stack.snapshot();
    assert(stack.pop() == 7);
    stack.push(7);  // Same value, freshly allocated node and ownership.
    assert(held.get() != stack.snapshot().get());
    assert(!StackTestAccess::stale_pop(stack, held));
    assert(stack.pop() == 7 && !stack.pop());
    assert(Node::live.load() == 1);
    held.reset();
    assert_all_reclaimed();
}

void check_bounded_concurrency() {
    constexpr std::size_t workers = 4;
    constexpr int per_worker = 32;
    constexpr std::size_t total = workers * per_worker;
    OwnedStack stack;
    std::array<std::exception_ptr, workers> failures{};
    {
        std::vector<std::jthread> threads;
        threads.reserve(workers);
        for (std::size_t id = 0; id < workers; ++id) {
            threads.emplace_back([&, id] {
                try {
                    for (int i = 0; i < per_worker; ++i) {
                        stack.push(static_cast<int>(id) * per_worker + i);
                    }
                } catch (...) {
                    failures[id] = std::current_exception();
                }
            });
        }
    }  // Every started jthread joins, including during stack unwinding.
    for (const auto& failure : failures) {
        if (failure) {
            std::rethrow_exception(failure);
        }
    }
    assert(Node::live.load() == total);

    std::array<std::atomic<unsigned>, total> seen{};
    {
        std::vector<std::jthread> threads;
        threads.reserve(workers);
        for (std::size_t id = 0; id < workers; ++id) {
            threads.emplace_back([&] {
                while (const auto value = stack.pop()) {
                    assert(*value >= 0 && static_cast<std::size_t>(*value) < total);
                    seen[static_cast<std::size_t>(*value)].fetch_add(
                        1, std::memory_order_relaxed);
                }
            });
        }
    }
    for (const auto& count : seen) {
        assert(count.load() == 1);
    }
    assert(!stack.pop());
    assert_all_reclaimed();
    std::cout << "atomic shared_ptr head lock-free: " << std::boolalpha
              << stack.head_is_lock_free() << '\n';
}

int main() {
    try {
        check_index_aba();
        check_tag_and_wrap();
        check_ownership_comparison();
        check_lifetime_and_failure();
        check_bounded_concurrency();
        assert_all_reclaimed();
        std::cout << "ABA models, ownership, delayed destruction and 128 items: OK\n";
    } catch (const std::exception& error) {
        std::cerr << "test failed: " << error.what() << '\n';
        return 1;
    }
}
