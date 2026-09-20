#include <array>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <limits>
#include <span>
#include <vector>

// In-memory teaching model: no file I/O, fsync, election, or durable sink.
using Word = std::uint32_t;
using Total = std::uint64_t;
constexpr Word magic = 0x52504c31U;
constexpr Word format_version = 1;
constexpr std::size_t frame_bytes = 40;
constexpr std::size_t capacity = 8;
using Frame = std::array<std::uint8_t, frame_bytes>;

Word get_word(std::span<const std::uint8_t> bytes, std::size_t offset) {
    assert(offset <= bytes.size() && bytes.size() - offset >= 4);
    Word value = 0;
    for (std::size_t i = 0; i < 4; ++i) value = (value << 8) | bytes[offset + i];
    return value;
}
void put_word(Frame& bytes, std::size_t offset, Word value) {
    assert(offset <= bytes.size() && bytes.size() - offset >= 4);
    for (std::size_t i = 0; i < 4; ++i) {
        bytes[offset + i] = static_cast<std::uint8_t>(value >> (24 - 8 * i));
    }
}
Word checksum(std::span<const std::uint8_t> bytes) {
    Word hash = 2166136261U;
    for (auto byte : bytes) hash = (hash ^ byte) * 16777619U;
    return hash; // Defined unsigned wrap; accidental corruption only, not authentication.
}
void seal(Frame& frame) { put_word(frame, 36, checksum(std::span(frame).first(36))); }

struct Input {
    Word stream;
    Word sequence;
    Word logical_time;
    Word draw;
    Word amount;
    Word config;
    Word previous_checksum;
};
Frame encode(const Input& input) {
    Frame frame{};
    const std::array<Word, 9> fields{magic, format_version, input.stream, input.sequence,
        input.logical_time, input.draw, input.amount, input.config, input.previous_checksum};
    for (std::size_t i = 0; i < fields.size(); ++i) put_word(frame, i * 4, fields[i]);
    seal(frame);
    return frame;
}
void append(std::vector<std::uint8_t>& log, const Frame& frame) {
    assert(log.size() <= (capacity - 1) * frame_bytes);
    log.insert(log.end(), frame.begin(), frame.end());
}
struct Intent {
    Word stream = 0;
    Word sequence = 0;
    Total amount = 0;
    bool operator==(const Intent&) const = default;
};
struct State {
    Word schema = 1;
    Word stream = 7;
    Word sequence = 0;
    Word logical_time = 0;
    Word config = 1;
    Word last_checksum = 0;
    Total balance = 0;
    std::array<Intent, capacity> pending{};
    std::size_t pending_count = 0;
    bool operator==(const State&) const = default;
};
enum class Status { ok, too_large, truncated, format, checksum, foreign, version,
                    gap, duplicate_unknown, conflict, chain, invalid_input,
                    arithmetic, outbox_full, target };

// The snapshot and target are trusted inputs from an external committed-prefix protocol.
// State includes pending output intents. We do not discover commitment or durability.
Status replay(const State& snapshot, std::span<const std::uint8_t> log,
              Word target, State& published) {
    if (log.size() > capacity * frame_bytes) return Status::too_large;
    if (log.size() % frame_bytes != 0) return Status::truncated;
    if (snapshot.schema != 1 || snapshot.config != 1) return Status::version;
    if (snapshot.stream == 0 || snapshot.pending_count > capacity) return Status::invalid_input;
    if (target < snapshot.sequence) return Status::target;
    State candidate = snapshot;
    std::array<Frame, capacity> seen{};
    std::size_t seen_count = 0;
    for (std::size_t offset = 0; offset < log.size(); offset += frame_bytes) {
        const auto bytes = log.subspan(offset, frame_bytes);
        if (get_word(bytes, 0) != magic || get_word(bytes, 4) != format_version) return Status::format;
        if (checksum(bytes.first(36)) != get_word(bytes, 36)) return Status::checksum;
        const Input input{get_word(bytes, 8), get_word(bytes, 12), get_word(bytes, 16),
            get_word(bytes, 20), get_word(bytes, 24), get_word(bytes, 28), get_word(bytes, 32)};
        if (input.stream != candidate.stream) return Status::foreign;
        if (input.config != candidate.config) return Status::version;
        Frame frame{};
        for (std::size_t i = 0; i < frame_bytes; ++i) frame[i] = bytes[i];
        if (input.sequence <= candidate.sequence) {
            bool duplicate = false;
            for (std::size_t i = 0; i < seen_count; ++i) {
                if (get_word(seen[i], 12) == input.sequence) {
                    if (seen[i] != frame) return Status::conflict;
                    duplicate = true;
                    break;
                }
            }
            if (duplicate) continue;
            return Status::duplicate_unknown; // Snapshot overlap lacks original identity bytes.
        }
        // input.sequence > candidate.sequence, so this +1 cannot wrap.
        if (input.sequence != candidate.sequence + 1) return Status::gap;
        if (input.previous_checksum != candidate.last_checksum) return Status::chain;
        if (input.logical_time < candidate.logical_time || input.draw > 9 || input.amount == 0) {
            return Status::invalid_input;
        }
        // Config 1 fixes the multiplier. Time and draw come only from the log.
        const Total increment = static_cast<Total>(input.amount) * 2 + input.draw + input.logical_time % 5;
        if (increment > std::numeric_limits<Total>::max() - candidate.balance) return Status::arithmetic;
        if (candidate.pending_count == capacity) return Status::outbox_full;
        candidate.balance += increment;
        candidate.sequence = input.sequence;
        candidate.logical_time = input.logical_time;
        candidate.last_checksum = get_word(bytes, 36);
        candidate.pending[candidate.pending_count++] = {input.stream, input.sequence, increment};
        seen[seen_count++] = frame;
    }
    if (candidate.sequence != target) return Status::target;
    published = candidate; // Single-owner replacement after all validation, no concurrent readers.
    return Status::ok;
}

enum class Delivery { applied, duplicate, fenced, conflict, full, invalid };
class Sink {
public:
    bool activate(Word epoch) { // Trusted external control; NOT a leader election algorithm.
        if (epoch <= epoch_) return false;
        epoch_ = epoch;
        return true;
    }
    Delivery deliver(Word epoch, const Intent& intent) {
        if (epoch_ == 0 || epoch != epoch_) return Delivery::fenced;
        if (intent.stream == 0 || intent.sequence == 0 || intent.amount == 0) return Delivery::invalid;
        for (std::size_t i = 0; i < count_; ++i) {
            if (seen_[i].stream == intent.stream && seen_[i].sequence == intent.sequence) {
                return seen_[i] == intent ? Delivery::duplicate : Delivery::conflict;
            }
        }
        if (count_ == capacity || intent.amount > std::numeric_limits<Total>::max() - applied_) {
            return Delivery::full;
        }
        seen_[count_++] = intent;
        applied_ += intent.amount;
        return Delivery::applied;
    }
    Total applied() const { return applied_; }
    std::size_t effects() const { return count_; }
private:
    Word epoch_ = 0;
    std::array<Intent, capacity> seen_{};
    std::size_t count_ = 0;
    Total applied_ = 0;
};

std::array<Frame, 4> fixture() {
    std::array<Frame, 4> frames{};
    frames[0] = encode({7, 1, 10, 1, 3, 1, 0});
    frames[1] = encode({7, 2, 12, 2, 4, 1, get_word(frames[0], 36)});
    frames[2] = encode({7, 3, 15, 0, 1, 1, get_word(frames[1], 36)});
    frames[3] = encode({7, 4, 18, 3, 2, 1, get_word(frames[2], 36)});
    return frames;
}
std::vector<std::uint8_t> join(std::span<const Frame> frames) {
    std::vector<std::uint8_t> log;
    for (const auto& frame : frames) append(log, frame);
    return log;
}

void test_rebuild_and_delivery() {
    const auto frames = fixture();
    const auto log = join(frames);
    State full;
    assert(replay(State{}, log, 4, full) == Status::ok);
    assert(full.balance == 31 && full.pending_count == 4);
    const std::array<Total, 4> expected{7, 12, 2, 10}; // Independently hand calculated.
    for (std::size_t i = 0; i < expected.size(); ++i) assert(full.pending[i].amount == expected[i]);
    State snapshot;
    assert(replay(State{}, join(std::span(frames).first(2)), 2, snapshot) == Status::ok);
    assert(snapshot.balance == 19 && snapshot.pending_count == 2);
    State restored;
    assert(replay(snapshot, join(std::span(frames).last(2)), 4, restored) == Status::ok);
    assert(restored == full); // Pending outputs before snapshot were preserved as well.
    auto repeated = log;
    append(repeated, frames[1]);
    State with_duplicate;
    assert(replay(State{}, repeated, 4, with_duplicate) == Status::ok && with_duplicate == full);

    Sink sink;
    assert(sink.deliver(0, full.pending[0]) == Delivery::fenced);
    assert(sink.activate(1));
    assert(sink.deliver(1, {7, 0, 1}) == Delivery::invalid);
    assert(sink.deliver(1, full.pending[0]) == Delivery::applied);
    // Simulated lost acknowledgment: sender does not know the first output was applied.
    assert(sink.activate(2));
    assert(!sink.activate(1));
    assert(sink.deliver(2, restored.pending[0]) == Delivery::duplicate);
    assert(sink.deliver(1, restored.pending[1]) == Delivery::fenced);
    for (std::size_t i = 1; i < restored.pending_count; ++i) {
        assert(sink.deliver(2, restored.pending[i]) == Delivery::applied);
    }
    auto conflict = restored.pending[0];
    ++conflict.amount;
    assert(sink.deliver(2, conflict) == Delivery::conflict);
    assert(sink.applied() == 31 && sink.effects() == 4);
    std::cout << "full=snapshot+suffix: balance=31 outputs=4; sink effects=4\n";
}

void test_corruption_and_boundaries() {
    const auto frames = fixture();
    const auto log = join(frames);
    State sentinel;
    sentinel.balance = 999;
    auto fails_unchanged = [&](const State& base, std::span<const std::uint8_t> bytes, Word target) {
        State published = sentinel;
        assert(replay(base, bytes, target, published) != Status::ok);
        assert(published == sentinel);
    };
    for (std::size_t length = 0; length < log.size(); ++length) {
        fails_unchanged(State{}, std::span(log).first(length), 4);
    }
    for (std::size_t i = 0; i < log.size(); ++i) {
        auto damaged = log;
        damaged[i] ^= 1U;
        fails_unchanged(State{}, damaged, 4);
    }
    auto gap = join(std::span(frames).first(1));
    append(gap, frames[2]);
    fails_unchanged(State{}, gap, 3);
    auto reorder = join(std::span(frames).last(2));
    fails_unchanged(State{}, reorder, 4);
    for (const auto& change : std::array<std::array<Word, 2>, 6>{{
             {4, 2}, {8, 8}, {16, 0}, {20, 10}, {28, 2}, {32, 123}}}) {
        auto changed = frames;
        put_word(changed[1], change[0], change[1]);
        seal(changed[1]); // Recompute checksum so semantic validation is exercised.
        fails_unchanged(State{}, join(changed), 4);
    }
    auto duplicate_conflict = log;
    auto altered = frames[1];
    put_word(altered, 24, 9);
    seal(altered);
    append(duplicate_conflict, altered);
    fails_unchanged(State{}, duplicate_conflict, 4);
    State snapshot;
    assert(replay(State{}, join(std::span(frames).first(2)), 2, snapshot) == Status::ok);
    fails_unchanged(snapshot, log, 4); // Covered frames cannot be authenticated by the snapshot alone.
    fails_unchanged(State{}, log, 3);
    fails_unchanged(State{}, log, 5);
    State wrong_version;
    wrong_version.schema = 2;
    fails_unchanged(wrong_version, {}, 0);
    State overflow;
    overflow.balance = std::numeric_limits<Total>::max();
    fails_unchanged(overflow, join(std::span(frames).first(1)), 1);
    State full_outbox;
    full_outbox.sequence = 8;
    full_outbox.pending_count = capacity;
    for (std::size_t i = 0; i < capacity; ++i) {
        full_outbox.pending[i] = {7, static_cast<Word>(i + 1), 1};
    }
    const std::array<Frame, 1> ninth{encode({7, 9, 1, 0, 1, 1, 0})};
    fails_unchanged(full_outbox, join(ninth), 9);
    State near_end;
    near_end.sequence = std::numeric_limits<Word>::max() - 1;
    const std::array<Frame, 1> final_frame{encode({7, std::numeric_limits<Word>::max(), 1, 0, 1, 1, 0})};
    State exhausted;
    assert(replay(near_end, join(final_frame), std::numeric_limits<Word>::max(), exhausted) == Status::ok);
    assert(exhausted.balance == 3);
    fails_unchanged(exhausted, join(std::span(frames).first(1)), exhausted.sequence);
    std::vector<std::uint8_t> excessive((capacity + 1) * frame_bytes, 0);
    fails_unchanged(State{}, excessive, 0);
    State empty;
    assert(replay(State{}, {}, 0, empty) == Status::ok && empty == State{});
    std::cout << "truncation_offsets=160 corruption_offsets=160; failure publication unchanged\n";
}

int main() {
    test_rebuild_and_delivery();
    test_corruption_and_boundaries();
    std::cout << "memory model only: no durability, consensus or authenticated log\n";
}
