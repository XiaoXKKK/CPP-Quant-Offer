#include <algorithm>
#include <array>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <limits>
#include <optional>

class TimerWheel {
public:
    using Tick = std::uint64_t;
    static constexpr std::size_t capacity = 8;
    static constexpr std::size_t bucket_count = 4;

    class Handle {
        friend class TimerWheel;
        const TimerWheel* owner_ = nullptr;
        std::size_t index_ = capacity;
        std::uint64_t id_ = 0;
        Handle(const TimerWheel* owner, std::size_t index, std::uint64_t id)
            : owner_(owner), index_(index), id_(id) {}
    public:
        Handle() = default;
    };
    enum class Error { none, closed, not_future, full, id_exhausted };
    struct Scheduled {
        Error error;
        std::optional<Handle> handle;
    };
    struct Event {
        std::uint32_t token = 0;
        Tick deadline = 0;
        Tick emitted_at = 0;
    };
    struct Batch {
        std::array<Event, capacity> events{};
        std::size_t count = 0;
    };

    // Logical ticks only. No clock, background thread, or callback execution.
    explicit TimerWheel(Tick initial_tick = 0) noexcept : now_(initial_tick) {
        heads_.fill(capacity);
        for (std::size_t i = 0; i < capacity; ++i) {
            slots_[i].next = i + 1;
        }
    }
    TimerWheel(const TimerWheel&) = delete;
    TimerWheel& operator=(const TimerWheel&) = delete;
    TimerWheel(TimerWheel&&) = delete;
    TimerWheel& operator=(TimerWheel&&) = delete;

    Scheduled schedule(Tick deadline, std::uint32_t token) noexcept {
        if (closed_) return {Error::closed, std::nullopt};
        if (deadline <= now_) return {Error::not_future, std::nullopt};
        if (free_head_ == capacity) return {Error::full, std::nullopt};
        // Never wrap an issued identity. The maximal id is deliberately unused.
        if (next_id_ == std::numeric_limits<std::uint64_t>::max()) {
            return {Error::id_exhausted, std::nullopt};
        }
        const auto index = free_head_;
        auto& slot = slots_[index];
        free_head_ = slot.next;
        slot.deadline = deadline;
        slot.token = token;
        slot.id = next_id_++;
        slot.active = true;
        slot.previous = capacity;
        const auto bucket = bucket_for(deadline);
        slot.next = heads_[bucket];
        if (slot.next != capacity) slots_[slot.next].previous = index;
        heads_[bucket] = index;
        ++pending_;
        return {Error::none, Handle(this, index, slot.id)};
    }

    bool cancel(const Handle& handle) noexcept {
        if (handle.owner_ != this || handle.index_ >= capacity) return false;
        const auto& slot = slots_[handle.index_];
        if (!slot.active || slot.id != handle.id_) return false;
        release(handle.index_);
        return true;
    }

    std::optional<Batch> advance_one() noexcept {
        if (closed_ || now_ == std::numeric_limits<Tick>::max()) return std::nullopt;
        ++now_;
        Batch batch;
        auto index = heads_[bucket_for(now_)];
        while (index != capacity) {
            const auto& slot = slots_[index];
            const auto next = slot.next;
            if (slot.deadline <= now_) {
                assert(batch.count < capacity);
                batch.events[batch.count++] = {slot.token, slot.deadline, now_};
                release(index);
            }
            index = next;
        }
        return batch;
    }

    std::size_t stop() noexcept {
        const auto cancelled = pending_;
        closed_ = true;
        heads_.fill(capacity);
        for (std::size_t i = 0; i < capacity; ++i) {
            slots_[i].active = false;
            slots_[i].next = i + 1;
        }
        free_head_ = 0;
        pending_ = 0;
        return cancelled;
    }
    Tick now() const noexcept { return now_; }
    std::size_t pending() const noexcept { return pending_; }

private:
    struct Slot {
        Tick deadline = 0;
        std::uint32_t token = 0;
        std::uint64_t id = 0;
        std::size_t previous = capacity;
        std::size_t next = capacity;
        bool active = false;
    };
    static std::size_t bucket_for(Tick tick) noexcept {
        return static_cast<std::size_t>(tick % bucket_count);
    }
    void release(std::size_t index) noexcept {
        auto& slot = slots_[index];
        if (slot.previous == capacity) heads_[bucket_for(slot.deadline)] = slot.next;
        else slots_[slot.previous].next = slot.next;
        if (slot.next != capacity) slots_[slot.next].previous = slot.previous;
        slot.active = false;
        slot.next = free_head_;
        free_head_ = index;
        --pending_;
    }
    std::array<Slot, capacity> slots_{};
    std::array<std::size_t, bucket_count> heads_{};
    std::size_t free_head_ = 0;
    std::size_t pending_ = 0;
    Tick now_;
    std::uint64_t next_id_ = 1;
    bool closed_ = false;
};

void check_against_scan_oracle() {
    TimerWheel wheel;
    using Error = TimerWheel::Error;
    constexpr std::array<TimerWheel::Tick, 8> deadlines{1, 4, 5, 8, 9, 9, 12, 20};
    std::array<TimerWheel::Handle, 8> handles;
    for (std::size_t i = 0; i < deadlines.size(); ++i) {
        const auto result = wheel.schedule(deadlines[i], static_cast<std::uint32_t>(i + 1));
        assert(result.error == Error::none && result.handle);
        handles[i] = *result.handle;
    }
    assert(wheel.schedule(21, 88).error == Error::full);
    assert(wheel.schedule(0, 88).error == Error::not_future);
    assert(wheel.cancel(handles[3]) && !wheel.cancel(handles[3]));
    const auto replacement = wheel.schedule(9, 99);
    assert(replacement.handle && !wheel.cancel(handles[3]));
    TimerWheel other;
    assert(!other.cancel(*replacement.handle));
    assert(!wheel.cancel(TimerWheel::Handle{}));

    struct Expected { TimerWheel::Tick deadline; std::uint32_t token; };
    // Independent reference list: the cancelled token 4 is absent.
    constexpr std::array<Expected, 8> oracle{{
        {1, 1}, {4, 2}, {5, 3}, {9, 5}, {9, 6}, {12, 7}, {20, 8}, {9, 99}
    }};
    std::size_t emitted = 0;
    for (TimerWheel::Tick tick = 1; tick <= 24; ++tick) {
        const auto batch = wheel.advance_one();
        assert(batch && wheel.now() == tick);
        std::array<std::uint32_t, TimerWheel::capacity> actual_tokens{};
        std::array<std::uint32_t, TimerWheel::capacity> expected_tokens{};
        std::size_t expected_count = 0;
        for (const auto& expected : oracle) {
            if (expected.deadline == tick) expected_tokens[expected_count++] = expected.token;
        }
        assert(batch->count == expected_count);
        for (std::size_t i = 0; i < batch->count; ++i) {
            const auto& event = batch->events[i];
            assert(event.deadline == tick && event.emitted_at == tick);
            actual_tokens[i] = event.token;
        }
        std::sort(actual_tokens.begin(), actual_tokens.end());
        std::sort(expected_tokens.begin(), expected_tokens.end());
        assert(actual_tokens == expected_tokens);
        emitted += batch->count;
        assert(wheel.pending() == oracle.size() - emitted);
    }
    assert(emitted == 8 && wheel.pending() == 0);
    for (const auto& handle : handles) assert(!wheel.cancel(handle));
    std::cout << "oracle: 24 ticks, 8 exact emissions, 3 simultaneous at tick 9\n";
}

void check_boundaries_and_stop() {
    TimerWheel burst;
    for (std::size_t i = 0; i < TimerWheel::capacity; ++i) {
        assert(burst.schedule(1, static_cast<std::uint32_t>(i + 1)).handle);
    }
    const auto all_due = burst.advance_one();
    assert(all_due && all_due->count == TimerWheel::capacity && burst.pending() == 0);
    std::uint32_t token_sum = 0;
    for (const auto& event : all_due->events) token_sum += event.token;
    assert(token_sum == 36);

    TimerWheel wheel;
    const auto handle = wheel.schedule(1, 42);
    assert(handle.handle);
    const auto emitted = wheel.advance_one();
    assert(emitted && emitted->count == 1);
    assert(!wheel.cancel(*handle.handle));
    assert(wheel.schedule(1, 1).error == TimerWheel::Error::not_future);
    const auto later = wheel.schedule(100, 7);
    assert(later.handle && wheel.stop() == 1 && wheel.stop() == 0);
    assert(!wheel.cancel(*later.handle) && !wheel.advance_one());
    assert(wheel.schedule(101, 9).error == TimerWheel::Error::closed);
    assert(emitted->events[0].token == 42); // stop cannot revoke an owned batch.

    const auto maximum = std::numeric_limits<TimerWheel::Tick>::max();
    TimerWheel near_limit(maximum - 1);
    assert(near_limit.schedule(maximum, 55).handle);
    const auto last = near_limit.advance_one();
    assert(last && last->count == 1 && last->events[0].deadline == maximum);
    assert(!near_limit.advance_one() && near_limit.now() == maximum);
    std::cout << "boundaries: stale/cross-wheel/closed rejected; tick limit does not wrap\n";
}

int main() {
    check_against_scan_oracle();
    check_boundaries_and_stop();
    std::cout << "logical ticks only; no wall-clock latency or callback execution measured\n";
}
