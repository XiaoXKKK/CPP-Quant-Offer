#include <algorithm>
#include <array>
#include <atomic>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <limits>
#include <memory>
#include <stdexcept>

// Fictional protocol: one session/sequence domain, four additive counters.
// A complete snapshot at W includes every event with sequence <= W.
using Number = std::uint64_t;
constexpr std::size_t instruments = 4;
constexpr std::size_t buffer_capacity = 8;
constexpr Number maximum = std::numeric_limits<Number>::max();
using Quantities = std::array<Number, instruments>;

struct Token {
    Number session;
    Number attempt;
    bool operator==(const Token&) const = default;
};
struct Delta {
    Number session;
    Number sequence;
    std::size_t instrument;
    Number add;
    bool operator==(const Delta&) const = default;
};
struct Snapshot {
    Number session;
    Number watermark;
    Quantities quantities;
    bool complete;
};
struct Published {
    Number session = 0;
    Number watermark = 0;
    Quantities quantities{};
    bool valid = false;
};
enum class Result {
    accepted, duplicate, foreign, inactive, poisoned, buffer_full,
    malformed, conflicting_duplicate, incomplete_snapshot, bad_cut,
    gap, quantity_overflow, sequence_exhausted, published
};

class Recovery {
public:
    explicit Recovery(Number last_attempt = 0)
        : attempt_(last_attempt), published_(std::make_shared<const Published>()) {}

    // Only this owner thread calls begin, buffer or commit.
    Token begin(Number session) {
        if (session == 0) throw std::invalid_argument("zero session");
        if (attempt_ == maximum) throw std::overflow_error("attempt identity exhausted");
        auto invalid = *read();
        invalid.valid = false; // Keep the last complete payload for diagnostics.
        auto next = std::make_shared<const Published>(invalid);
        token_ = {session, ++attempt_};
        count_ = 0;
        poisoned_ = false;
        active_ = true;
        published_.store(std::move(next), std::memory_order_release);
        return token_;
    }

    std::shared_ptr<const Published> read() const {
        return published_.load(std::memory_order_acquire);
    }

    Result buffer(Token token, const Delta& delta) {
        if (token != token_ || delta.session != token_.session) return Result::foreign;
        if (!active_) return Result::inactive;
        if (poisoned_) return Result::poisoned;
        if (delta.sequence == 0 || delta.instrument >= instruments) {
            return poison(Result::malformed);
        }
        for (std::size_t i = 0; i < count_; ++i) {
            if (buffer_[i].sequence == delta.sequence) {
                if (buffer_[i] == delta) return Result::duplicate;
                return poison(Result::conflicting_duplicate);
            }
        }
        if (count_ == buffer_capacity) return poison(Result::buffer_full);
        buffer_[count_++] = delta;
        return Result::accepted;
    }

    Result commit(Token token, const Snapshot& snapshot, Number cut) {
        if (token != token_ || snapshot.session != token_.session) return Result::foreign;
        if (!active_) return Result::inactive;
        if (poisoned_) return Result::poisoned;
        if (!snapshot.complete) return Result::incomplete_snapshot;
        // This protocol requires session rollover before sequence exhaustion.
        if (snapshot.watermark == maximum || cut == maximum) return Result::sequence_exhausted;
        const auto before = read();
        if (cut < snapshot.watermark ||
            (before->session == snapshot.session && cut < before->watermark)) {
            return Result::bad_cut;
        }
        auto ordered = buffer_;
        for (std::size_t i = 1; i < count_; ++i) {
            const auto value = ordered[i];
            std::size_t j = i;
            while (j != 0 && ordered[j - 1].sequence > value.sequence) {
                ordered[j] = ordered[j - 1];
                --j;
            }
            ordered[j] = value;
        }
        Published candidate{snapshot.session, snapshot.watermark, snapshot.quantities, true};
        for (std::size_t i = 0; i < count_; ++i) {
            const auto& delta = ordered[i];
            if (delta.sequence <= snapshot.watermark) continue;
            if (delta.sequence > cut) return Result::bad_cut;
            // candidate.watermark < cut < maximum before every accepted step.
            if (delta.sequence != candidate.watermark + 1) return Result::gap;
            auto& quantity = candidate.quantities[delta.instrument];
            if (delta.add > maximum - quantity) return Result::quantity_overflow;
            quantity += delta.add;
            candidate.watermark = delta.sequence;
        }
        if (candidate.watermark != cut) return Result::gap;
        // Allocation may throw; the old published handle remains unchanged.
        auto next = std::make_shared<const Published>(candidate);
        published_.store(std::move(next), std::memory_order_release);
        active_ = false;
        count_ = 0;
        return Result::published;
    }

private:
    Result poison(Result reason) {
        poisoned_ = true;
        return reason;
    }
    Number attempt_;
    Token token_{};
    std::array<Delta, buffer_capacity> buffer_{};
    std::size_t count_ = 0;
    bool active_ = false;
    bool poisoned_ = false;
    std::atomic<std::shared_ptr<const Published>> published_;
};

void seed(Recovery& recovery) {
    const auto token = recovery.begin(7);
    assert(recovery.commit(token, {7, 9, {5, 6, 7, 8}, true}, 9) == Result::published);
}

void test_permutations() {
    const std::array<Delta, 3> events{{{7, 11, 0, 2}, {7, 12, 1, 3}, {7, 13, 0, 5}}};
    const Quantities expected{17, 23, 30, 40}; // Hand-derived, not replayed by the implementation.
    std::array<std::size_t, 3> order{0, 1, 2};
    std::size_t permutations = 0;
    do {
        Recovery recovery;
        seed(recovery);
        const auto retained_old_reader = recovery.read();
        const auto token = recovery.begin(7);
        const auto invalid = recovery.read();
        assert(!invalid->valid && invalid->quantities == retained_old_reader->quantities);
        assert(retained_old_reader->valid); // Old handles are NOT remotely revoked.
        assert(recovery.buffer(token, {7, 10, 0, 999}) == Result::accepted); // Snapshot overlap.
        for (auto i : order) assert(recovery.buffer(token, events[i]) == Result::accepted);
        assert(recovery.buffer(token, events[1]) == Result::duplicate);
        assert(recovery.commit(token, {7, 10, {10, 20, 30, 40}, true}, 13) == Result::published);
        const auto current = recovery.read();
        assert(current->valid && current->watermark == 13 && current->quantities == expected);
        assert((retained_old_reader->quantities == Quantities{5, 6, 7, 8}));
        assert(recovery.buffer(token, {7, 14, 0, 1}) == Result::inactive);
        ++permutations;
    } while (std::next_permutation(order.begin(), order.end()));
    assert(permutations == 6);
    std::cout << "permutations=6 overlap/duplicate/immutable-reader passed\n";
}

void test_gap_and_identity() {
    Recovery recovery;
    seed(recovery);
    const auto stale = recovery.begin(7);
    const auto token = recovery.begin(7);
    const auto invalid = recovery.read();
    assert(recovery.buffer(stale, {7, 11, 0, 2}) == Result::foreign);
    assert(recovery.buffer(token, {6, 11, 0, 2}) == Result::foreign);
    assert(recovery.commit(stale, {7, 10, {}, true}, 10) == Result::foreign);
    assert(recovery.commit(token, {6, 10, {}, true}, 10) == Result::foreign);
    assert(recovery.commit(token, {7, 10, {}, false}, 10) == Result::incomplete_snapshot);
    assert(recovery.commit(token, {7, 8, {}, true}, 8) == Result::bad_cut);
    assert(recovery.buffer(token, {7, 13, 0, 5}) == Result::accepted);
    assert(recovery.buffer(token, {7, 11, 0, 2}) == Result::accepted);
    const Snapshot snapshot{7, 10, {10, 20, 30, 40}, true};
    assert(recovery.commit(token, snapshot, 13) == Result::gap);
    assert(recovery.read() == invalid); // No partial replay was published.
    assert(recovery.buffer(token, {7, 12, 1, 3}) == Result::accepted);
    assert(recovery.commit(token, snapshot, 12) == Result::bad_cut);
    assert(recovery.read() == invalid);
    assert(recovery.commit(token, snapshot, 14) == Result::gap);
    assert(recovery.read() == invalid);
    assert(recovery.commit(token, snapshot, 13) == Result::published);
    assert((recovery.read()->quantities == Quantities{17, 23, 30, 40}));
    const auto new_session = recovery.begin(8);
    assert(recovery.commit(token, snapshot, 13) == Result::foreign);
    assert(recovery.commit(new_session, {8, 0, {1, 2, 3, 4}, true}, 0) == Result::published);
    assert(recovery.read()->session == 8 && recovery.read()->watermark == 0);
    std::cout << "gap repair and stale session/attempt passed\n";
}

void test_fatal_and_overflow() {
    Recovery recovery;
    seed(recovery);
    auto token = recovery.begin(7);
    auto invalid = recovery.read();
    for (Number i = 0; i < buffer_capacity; ++i) {
        assert(recovery.buffer(token, {7, 20 + i, 0, 1}) == Result::accepted);
    }
    assert(recovery.buffer(token, {7, 28, 0, 1}) == Result::buffer_full);
    assert(recovery.commit(token, {7, 28, {}, true}, 28) == Result::poisoned);
    assert(recovery.read() == invalid);
    token = recovery.begin(7);
    assert(recovery.commit(token, {7, 28, {30, 0, 0, 0}, true}, 28) == Result::published);

    token = recovery.begin(7);
    invalid = recovery.read();
    assert(recovery.buffer(token, {7, 29, 0, 1}) == Result::accepted);
    assert(recovery.buffer(token, {7, 29, 0, 2}) == Result::conflicting_duplicate);
    assert(recovery.commit(token, {7, 28, {}, true}, 29) == Result::poisoned);
    assert(recovery.read() == invalid);

    token = recovery.begin(7);
    invalid = recovery.read();
    assert(recovery.buffer(token, {7, 29, 0, 1}) == Result::accepted);
    assert(recovery.commit(token, {7, 28, {maximum, 0, 0, 0}, true}, 29) == Result::quantity_overflow);
    assert(recovery.read() == invalid);
    assert(recovery.commit(token, {7, maximum, {}, true}, maximum) == Result::sequence_exhausted);
    assert(recovery.read() == invalid);

    token = recovery.begin(7);
    assert(recovery.buffer(token, {7, 0, 0, 1}) == Result::malformed);
    token = recovery.begin(7);
    assert(recovery.buffer(token, {7, 30, instruments, 1}) == Result::malformed);
    token = recovery.begin(7);
    assert(recovery.commit(token, {7, maximum - 1, {}, true}, maximum - 1) == Result::published);
    assert(recovery.read()->watermark == maximum - 1);

    Recovery exhausted(maximum);
    const auto original = exhausted.read();
    bool threw = false;
    try { (void)exhausted.begin(7); } catch (const std::overflow_error&) { threw = true; }
    assert(threw && exhausted.read() == original);
    bool zero_session = false;
    try { (void)exhausted.begin(0); } catch (const std::invalid_argument&) { zero_session = true; }
    assert(zero_session);
    std::cout << "bounded buffer/conflict/arithmetic/identity exhaustion passed\n";
}

int main() {
    test_permutations();
    test_gap_and_identity();
    test_fatal_and_overflow();
    std::cout << "fictional protocol only; no exchange or network integration\n";
}
