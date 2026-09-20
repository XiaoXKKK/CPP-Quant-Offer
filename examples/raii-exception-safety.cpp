#include <array>
#include <cassert>
#include <cstddef>
#include <iostream>
#include <memory>
#include <span>
#include <stdexcept>
#include <type_traits>
#include <utility>
#include <vector>

struct InjectedFailure {};

// Test-only counters. All tests are single-threaded and keep at most two resources.
struct Resource {
    static inline int live = 0;
    static inline std::array<int, 2> released{};
    static inline std::size_t released_count = 0;
    int id;

    explicit Resource(int value) noexcept : id(value) { ++live; }
    Resource(const Resource&) = delete;
    Resource& operator=(const Resource&) = delete;
    ~Resource() noexcept {
        assert(live > 0);
        assert(released_count < released.size());
        released[released_count++] = id;
        --live;
    }

    static void reset_trace() noexcept {
        assert(live == 0);
        released.fill(0);
        released_count = 0;
    }
};

std::unique_ptr<Resource> acquire(int id, bool fail) {
    if (fail) throw InjectedFailure{};
    return std::make_unique<Resource>(id);
}

class Session {
    std::unique_ptr<Resource> first_;
    std::unique_ptr<Resource> second_;

public:
    static inline int destructor_calls = 0;

    // 1: first acquisition; 2: second acquisition; 3: constructor body.
    explicit Session(int fail_at)
        : first_(acquire(1, fail_at == 1)),
          second_(acquire(2, fail_at == 2)) {
        if (fail_at == 3) throw InjectedFailure{};
    }
    ~Session() noexcept { ++destructor_calls; }
};

class Ledger {
    std::vector<int> entries_;

    static void checkpoint(std::size_t completed, std::size_t fail_after) {
        if (completed == fail_after) throw InjectedFailure{};
    }

    static void validate(int entry) {
        if (entry < 0) throw std::invalid_argument("negative entry");
    }

public:
    // Return an owning snapshot: no borrowed views survive a successful commit.
    [[nodiscard]] std::vector<int> snapshot() const { return entries_; }

    // On failure, a prefix may remain. Every stored entry is nonnegative.
    void append_basic(std::span<const int> batch, std::size_t fail_after) {
        std::size_t completed = 0;
        for (int entry : batch) {
            checkpoint(completed, fail_after);
            validate(entry);
            entries_.push_back(entry);
            ++completed;
        }
        checkpoint(completed, fail_after);
    }

    // On failure, entries_ is unchanged. The allocator is std::allocator<int>.
    void append_strong(std::span<const int> batch, std::size_t fail_after) {
        auto staged = entries_;
        std::size_t completed = 0;
        for (int entry : batch) {
            checkpoint(completed, fail_after);
            validate(entry);
            staged.push_back(entry);
            ++completed;
        }
        checkpoint(completed, fail_after);
        static_assert(noexcept(entries_.swap(staged)));
        entries_.swap(staged);
    }
};

void test_construction_cleanup() {
    Session::destructor_calls = 0;
    for (int fail_at = 1; fail_at <= 3; ++fail_at) {
        Resource::reset_trace();
        bool caught = false;
        try {
            Session session(fail_at);
        } catch (const InjectedFailure&) {
            caught = true;
        }
        assert(caught);
        assert(Resource::live == 0);
        assert(Session::destructor_calls == 0);
        const auto count = static_cast<std::size_t>(fail_at - 1);
        assert(Resource::released_count == count);
        if (fail_at == 2) assert(Resource::released[0] == 1);
        if (fail_at == 3) {
            assert(Resource::released[0] == 2);
            assert(Resource::released[1] == 1);
        }
    }
    Resource::reset_trace();
    {
        Session session(0);
        assert(Resource::live == 2);
    }
    assert(Session::destructor_calls == 1);
    assert(Resource::live == 0);
    assert(Resource::released_count == 2);
    assert(Resource::released[0] == 2 && Resource::released[1] == 1);
}

void test_owner_transfer() {
    Resource::reset_trace();
    {
        auto owner = acquire(7, false);
        auto receiver = std::move(owner);
        assert(!owner && receiver && receiver->id == 7);
        assert(Resource::live == 1);
        receiver.reset();
        receiver.reset();
        assert(Resource::live == 0);
        assert(Resource::released_count == 1);
    }
    assert(Resource::released_count == 1);
}

void test_guarantees() {
    constexpr std::array seed{10};
    constexpr std::array batch{20, 30, 40};
    constexpr std::size_t no_failure = batch.size() + 1;
    for (std::size_t fail_after = 0; fail_after <= batch.size(); ++fail_after) {
        Ledger strong;
        strong.append_strong(seed, no_failure);
        const auto before = strong.snapshot();
        bool caught = false;
        try {
            strong.append_strong(batch, fail_after);
        } catch (const InjectedFailure&) {
            caught = true;
        }
        assert(caught && strong.snapshot() == before);

        Ledger basic;
        basic.append_basic(seed, no_failure);
        caught = false;
        try {
            basic.append_basic(batch, fail_after);
        } catch (const InjectedFailure&) {
            caught = true;
        }
        assert(caught);
        auto expected = before;
        for (std::size_t i = 0; i < fail_after; ++i) expected.push_back(batch[i]);
        assert(basic.snapshot() == expected);
        basic.append_basic(seed, no_failure);
        expected.push_back(10);
        assert(basic.snapshot() == expected);
    }

    Ledger ledger;
    ledger.append_strong({}, no_failure);
    assert(ledger.snapshot().empty());
    ledger.append_strong(batch, no_failure);
    const std::vector<int> expected{20, 30, 40};
    assert(ledger.snapshot() == expected);
    constexpr std::array invalid{50, -1, 60};
    bool caught = false;
    try {
        ledger.append_strong(invalid, no_failure);
    } catch (const std::invalid_argument&) {
        caught = true;
    }
    assert(caught && ledger.snapshot() == expected);
    ledger.append_strong({}, no_failure);
    assert(ledger.snapshot() == expected);
}

int main() {
    static_assert(std::is_nothrow_destructible_v<Resource>);
    static_assert(std::is_nothrow_move_constructible_v<std::unique_ptr<Resource>>);
    test_construction_cleanup();
    test_owner_transfer();
    test_guarantees();
    assert(Resource::live == 0);
    std::cout << "RAII cleanup, ownership transfer, basic/strong guarantees: OK\n";
}
