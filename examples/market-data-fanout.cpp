#include <array>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <limits>
#include <optional>
#include <span>
#include <type_traits>

// Deterministic teaching model. No networking or concurrent access.
constexpr std::uint32_t epoch = 7;
using Quantities = std::array<std::int64_t, 2>;
struct Delta {
    std::uint32_t generation;
    std::uint32_t sequence;
    std::uint8_t instrument;
    std::int32_t change;
    bool operator==(const Delta&) const = default;
};
struct Snapshot {
    std::uint32_t generation;
    std::uint32_t through_sequence;
    Quantities quantities;
    bool operator==(const Snapshot&) const = default;
};
static_assert(std::is_nothrow_copy_assignable_v<Delta>);
static_assert(std::is_nothrow_copy_assignable_v<Snapshot>);

enum class Reason { none, slow_consumer, source_invalid };
struct Health {
    bool valid = true;
    std::uint32_t generation = epoch;
    Reason reason = Reason::none;
    std::uint32_t failed_at = 0;
    void invalidate(Reason why, std::uint32_t sequence) noexcept {
        if (valid) { // Sticky first failure; no automatic recovery.
            valid = false;
            reason = why;
            failed_at = sequence;
        }
    }
};
enum class Delivery { not_attempted, accepted, invalidated, inactive };

class DeltaConsumer {
    friend class Fanout;
    std::array<Delta, 4> queue_{};
    std::size_t head_ = 0;
    std::size_t count_ = 0;
    Health health_{}; // Outside the bounded data queue.

    Delivery offer(std::span<const Delta> batch) noexcept {
        if (!health_.valid) return Delivery::inactive;
        if (batch.size() > queue_.size() - count_) {
            health_.invalidate(Reason::slow_consumer, batch.front().sequence);
            return Delivery::invalidated;
        }
        for (std::size_t i = 0; i < batch.size(); ++i) {
            queue_[(head_ + count_ + i) % queue_.size()] = batch[i];
        }
        count_ += batch.size();
        return Delivery::accepted;
    }
public:
    Health health() const noexcept { return health_; }
    std::size_t pending() const noexcept { return count_; }
    std::optional<Delta> pop() noexcept {
        if (!health_.valid || count_ == 0) return std::nullopt;
        const auto result = queue_[head_];
        queue_[head_] = {};
        head_ = (head_ + 1) % queue_.size();
        --count_;
        return result; // Owning value, not a borrowed ring slot.
    }
    std::size_t discard_invalid_backlog() noexcept {
        if (health_.valid) return 0;
        const auto discarded = count_;
        queue_ = {};
        head_ = 0;
        count_ = 0;
        return discarded; // Reclamation does not restore health.
    }
};

class LatestConsumer {
    friend class Fanout;
    Health health_{};
    std::optional<Snapshot> latest_;
public:
    Health health() const noexcept { return health_; }
    std::optional<Snapshot> take() noexcept {
        if (!health_.valid) return std::nullopt;
        const auto result = latest_;
        latest_.reset();
        return result;
    }
};

struct Report {
    bool source_accepted = false;
    std::array<Delivery, 2> delivery{Delivery::not_attempted, Delivery::not_attempted};
};

class Fanout {
    std::array<DeltaConsumer, 2> consumers_{};
    LatestConsumer latest_{};
    Quantities quantities_{100, 200}; // An already established source anchor at sequence 0.
    std::uint32_t next_ = 1;
    bool valid_ = true;

    Report reject_source() noexcept {
        valid_ = false;
        for (auto& consumer : consumers_) {
            consumer.health_.invalidate(Reason::source_invalid, next_);
        }
        latest_.health_.invalidate(Reason::source_invalid, next_);
        return {};
    }
public:
    DeltaConsumer& consumer(std::size_t index) { return consumers_.at(index); }
    LatestConsumer& latest() noexcept { return latest_; }
    Quantities state() const noexcept { return quantities_; }
    std::uint32_t next_sequence() const noexcept { return next_; }
    bool valid() const noexcept { return valid_; }

    Report broadcast(std::span<const Delta> batch) noexcept {
        if (!valid_) return {};
        if (batch.empty() || batch.size() > 3 ||
            batch.size() > std::numeric_limits<std::uint32_t>::max() - next_) {
            return reject_source();
        }
        auto staged = quantities_;
        for (std::size_t i = 0; i < batch.size(); ++i) {
            const auto& delta = batch[i];
            if (delta.generation != epoch || delta.sequence != next_ + i ||
                delta.instrument >= staged.size() || delta.change == 0 ||
                delta.change < -1000 || delta.change > 1000) {
                return reject_source();
            }
            const auto updated = staged[delta.instrument] + delta.change;
            if (updated < 0 || updated > 1000) return reject_source();
            staged[delta.instrument] = updated;
        }

        // Source commit does not imply that every subscriber accepted the batch.
        quantities_ = staged;
        next_ += static_cast<std::uint32_t>(batch.size());
        Report report;
        report.source_accepted = true;
        for (std::size_t i = 0; i < consumers_.size(); ++i) {
            report.delivery[i] = consumers_[i].offer(batch);
        }
        // This is a complete two-instrument state, never just the last delta.
        latest_.latest_ = Snapshot{epoch, next_ - 1, quantities_};
        return report;
    }
};

constexpr std::array<Delta, 6> events{{
    {epoch, 1, 0, 5}, {epoch, 2, 1, -10},
    {epoch, 3, 0, -3}, {epoch, 4, 1, 20},
    {epoch, 5, 0, 8}, {epoch, 6, 1, -5}}};
constexpr std::array<Quantities, 6> oracle{{
    {105, 200}, {105, 190}, {102, 190},
    {102, 210}, {110, 210}, {110, 205}}};

void verify_slow_consumer_isolation() {
    Fanout fanout;
    Quantities fast_state{100, 200};
    std::size_t applied = 0;
    assert(!fanout.latest().take());
    assert(!fanout.consumer(0).pop());
    for (std::size_t round = 0; round < 3; ++round) {
        std::array<Delta, 2> batch{events[round * 2], events[round * 2 + 1]};
        const auto report = fanout.broadcast(batch);
        batch = {}; // Consumer queues must own their copies.
        assert(report.source_accepted && report.delivery[0] == Delivery::accepted);
        assert(report.delivery[1] == (round < 2 ? Delivery::accepted : Delivery::invalidated));
        assert(fanout.state() == oracle[round * 2 + 1]);
        for (std::size_t i = 0; i < 2; ++i) {
            const auto next = fanout.consumer(0).pop();
            assert(next && *next == events[applied]);
            fast_state[next->instrument] += next->change;
            assert(fast_state == oracle[applied]);
            ++applied;
        }
        assert(!fanout.consumer(0).pop());
    }
    const auto slow = fanout.consumer(1).health();
    assert(!slow.valid && slow.generation == epoch && slow.reason == Reason::slow_consumer);
    assert(slow.failed_at == 5 && fanout.consumer(1).pending() == 4);
    assert(!fanout.consumer(1).pop()); // Failure is visible despite a full data queue.
    const auto snapshot = fanout.latest().take();
    assert((snapshot && *snapshot == Snapshot{epoch, 6, {110, 205}}));
    assert(!fanout.latest().take());
    assert(fanout.consumer(1).discard_invalid_backlog() == 4);
    assert(!fanout.consumer(1).health().valid && fanout.consumer(1).pending() == 0);

    constexpr std::array<Delta, 1> final{{{epoch, 7, 0, -10}}};
    const auto continuation = fanout.broadcast(final);
    assert(continuation.source_accepted);
    assert(continuation.delivery[0] == Delivery::accepted);
    assert(continuation.delivery[1] == Delivery::inactive);
    assert((fanout.state() == Quantities{100, 205}));
    assert(fanout.next_sequence() == 8 && fanout.valid());
    std::cout << "fast: 6 ordered deltas match oracle; slow: invalid at 5 with 4 pending\n"
              << "latest: complete state at 6 is [110,205]; slow reclamation does not reactivate\n";
}

void verify_rejected_source(std::span<const Delta> batch) {
    Fanout fanout;
    const auto result = fanout.broadcast(batch);
    assert(!result.source_accepted && !fanout.valid());
    assert((fanout.state() == Quantities{100, 200}));
    assert(fanout.next_sequence() == 1);
    for (std::size_t i = 0; i < 2; ++i) {
        assert(!fanout.consumer(i).health().valid);
        assert(fanout.consumer(i).health().reason == Reason::source_invalid);
        assert(fanout.consumer(i).pending() == 0);
    }
    assert(!fanout.latest().health().valid && !fanout.latest().take());
    assert(!fanout.broadcast(std::span(events).first(2)).source_accepted);
}

void verify_failures_and_value_lifetime() {
    // First delta is valid, second would make instrument 1 negative: no partial source commit.
    constexpr std::array<Delta, 2> underflow{{{epoch, 1, 0, 5}, {epoch, 2, 1, -999}}};
    verify_rejected_source(underflow);
    verify_rejected_source(std::span<const Delta>{});
    verify_rejected_source(std::span(events).first(4));
    for (const auto invalid : std::array<Delta, 5>{{
             {epoch, 2, 0, 1}, {epoch + 1, 1, 0, 1}, {epoch, 1, 2, 1},
             {epoch, 1, 0, 0}, {epoch, 1, 0, 1001}}}) {
        verify_rejected_source(std::span<const Delta>(&invalid, 1));
    }
    Fanout partial_capacity;
    assert(partial_capacity.broadcast(std::span(events).first(3)).source_accepted);
    for (std::size_t i = 0; i < 3; ++i) assert(partial_capacity.consumer(0).pop());
    const auto partial = partial_capacity.broadcast(std::span(events).subspan(3, 2));
    assert(partial.source_accepted && partial.delivery[0] == Delivery::accepted);
    assert(partial.delivery[1] == Delivery::invalidated);
    assert(partial_capacity.consumer(1).pending() == 3); // One free slot did not accept a prefix.
    assert((partial_capacity.state() == Quantities{110, 210}));

    std::optional<Snapshot> owned;
    {
        Fanout source;
        assert(source.broadcast(std::span(events).first(2)).source_accepted);
        owned = source.latest().take();
    }
    assert((owned && *owned == Snapshot{epoch, 2, {105, 190}}));
    std::cout << "invalid source batches commit nothing; owned snapshot survives producer lifetime\n";
}

int main() {
    verify_slow_consumer_isolation();
    verify_failures_and_value_lifetime();
    std::cout << "single-thread model only; no networking, persistence, or recovery implemented\n";
}
