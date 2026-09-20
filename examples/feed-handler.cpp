#include <algorithm>
#include <array>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <limits>
#include <span>
#include <stdexcept>
#include <type_traits>
#include <utility>
#include <vector>

// FH1 is a fictional teaching protocol, not an exchange protocol.
using Bytes = std::span<const std::uint8_t>;
enum class Side : std::uint8_t { bid = 1, ask = 2 };
struct Event {
    std::uint32_t session;
    std::uint32_t sequence;
    std::uint32_t instrument;
    Side side;
    std::uint64_t price_e8;
    std::uint32_t quantity;
    bool operator==(const Event&) const = default;
};
static_assert(std::is_nothrow_copy_assignable_v<Event>);

class Publisher {
    std::array<Event, 8> events_{};
    std::size_t count_ = 0;
public:
    // Single-threaded batch commit. This is not a concurrent queue.
    bool publish(std::span<const Event> batch) noexcept {
        if (batch.size() > events_.size() - count_) return false;
        for (std::size_t i = 0; i < batch.size(); ++i) events_[count_ + i] = batch[i];
        count_ += batch.size();
        return true;
    }
    std::span<const Event> events() const noexcept { return {events_.data(), count_}; }
};

enum class Status { published, duplicate, wrong_session, malformed,
                    sequence_range, gap, backpressure, blocked };

// Callers establish each field's bounds before these byte-wise reads.
std::uint16_t be16(Bytes bytes, std::size_t offset) noexcept {
    return static_cast<std::uint16_t>(
        (static_cast<std::uint32_t>(bytes[offset]) << 8U) | bytes[offset + 1]);
}
std::uint32_t be32(Bytes bytes, std::size_t offset) noexcept {
    std::uint32_t value = 0;
    for (std::size_t i = 0; i < 4; ++i) value = (value << 8U) | bytes[offset + i];
    return value;
}

class Handler {
    std::uint32_t session_;
    std::uint32_t next_;
    bool valid_ = true;
    Publisher& publisher_;

    Status invalidate(Status why) noexcept { valid_ = false; return why; }
public:
    // The caller supplies an already established session and next sequence.
    // Snapshot acquisition and recovery are deliberately outside this demo.
    Handler(std::uint32_t session, std::uint32_t next, Publisher& publisher)
        : session_(session), next_(next), publisher_(publisher) {
        if (session == 0 || next == 0) throw std::invalid_argument("invalid anchor");
    }
    bool valid() const noexcept { return valid_; }
    std::uint32_t next_sequence() const noexcept { return next_; }

    Status ingest(Bytes bytes) noexcept {
        if (!valid_) return Status::blocked;
        constexpr std::size_t header = 12;
        constexpr std::size_t record = 12;
        if (bytes.size() < header || bytes[0] != 0x46 || bytes[1] != 0x48 ||
            bytes[2] != 1 || bytes[3] == 0 || bytes[3] > 4) {
            return invalidate(Status::malformed);
        }
        const std::size_t count = bytes[3];
        if (bytes.size() != header + count * record) return invalidate(Status::malformed);
        const auto session = be32(bytes, 4);
        const auto first = be32(bytes, 8);
        if (session == 0) return invalidate(Status::malformed);
        const auto end = static_cast<std::uint64_t>(first) + count;
        if (first == 0 || end > std::numeric_limits<std::uint32_t>::max()) {
            return invalidate(Status::sequence_range);
        }
        if (session != session_) return Status::wrong_session;

        std::array<Event, 4> staged{};
        for (std::size_t i = 0; i < count; ++i) {
            const auto offset = header + i * record;
            const auto locate = be16(bytes, offset);
            const auto side = bytes[offset + 2];
            const auto flags = bytes[offset + 3];
            const auto price = be32(bytes, offset + 4);
            const auto quantity = be32(bytes, offset + 8);
            if ((locate != 1 && locate != 2) || (side != 1 && side != 2) || flags != 0 ||
                (quantity == 0 && price != 0) || (quantity != 0 && price == 0)) {
                return invalidate(Status::malformed);
            }
            staged[i] = {session, static_cast<std::uint32_t>(first + i),
                         static_cast<std::uint32_t>(1000U + locate),
                         static_cast<Side>(side), static_cast<std::uint64_t>(price) * 10000ULL,
                         quantity};
        }
        if (end <= next_) return Status::duplicate;
        if (first > next_) return invalidate(Status::gap);
        const auto skip = static_cast<std::size_t>(next_ - first);
        const auto unseen = std::span<const Event>(staged.data() + skip, count - skip);
        if (!publisher_.publish(unseen)) return invalidate(Status::backpressure);
        next_ = static_cast<std::uint32_t>(end);
        return Status::published;
    }
};

struct WireQuote {
    std::uint16_t locate;
    std::uint8_t side;
    std::uint32_t price_e4;
    std::uint32_t quantity;
};
void append16(std::vector<std::uint8_t>& out, std::uint16_t value) {
    out.push_back(static_cast<std::uint8_t>(value >> 8U));
    out.push_back(static_cast<std::uint8_t>(value));
}
void append32(std::vector<std::uint8_t>& out, std::uint32_t value) {
    for (unsigned shift : {24U, 16U, 8U, 0U}) {
        out.push_back(static_cast<std::uint8_t>(value >> shift));
    }
}
std::vector<std::uint8_t> packet(std::uint32_t session, std::uint32_t first,
                               std::span<const WireQuote> quotes) {
    if (quotes.empty() || quotes.size() > 4) throw std::invalid_argument("fixture size");
    std::vector<std::uint8_t> bytes{0x46, 0x48, 1, static_cast<std::uint8_t>(quotes.size())};
    bytes.reserve(60);
    append32(bytes, session);
    append32(bytes, first);
    for (const auto& quote : quotes) {
        append16(bytes, quote.locate);
        bytes.push_back(quote.side);
        bytes.push_back(0);
        append32(bytes, quote.price_e4);
        append32(bytes, quote.quantity);
    }
    return bytes;
}

constexpr std::array<std::uint8_t, 36> golden{
    0x46, 0x48, 0x01, 0x02, 0, 0, 0, 7, 0, 0, 0, 100,
    0, 1, 1, 0, 0, 0, 0x30, 0xd4, 0, 0, 0, 10,
    0, 2, 2, 0, 0, 0, 0x61, 0xa9, 0, 0, 0, 20};
constexpr std::array<Event, 3> oracle{{
    {7, 100, 1001, Side::bid, 125000000ULL, 10},
    {7, 101, 1002, Side::ask, 250010000ULL, 20},
    {7, 102, 1001, Side::bid, 0, 0}}};

void verify_normalization_and_overlap() {
    Publisher output;
    Handler handler(7, 100, output);
    auto input = golden;
    assert(handler.ingest(input) == Status::published);
    input.fill(0); // Published events own values; the input bytes may be reused.
    assert(std::equal(output.events().begin(), output.events().end(), oracle.begin()));
    assert(handler.next_sequence() == 102 && handler.valid());
    assert(handler.ingest(golden) == Status::duplicate);
    constexpr std::array<WireQuote, 2> overlap{{{2, 2, 25001, 20}, {1, 1, 0, 0}}};
    assert(handler.ingest(packet(7, 101, overlap)) == Status::published);
    assert(output.events().size() == oracle.size());
    assert(std::equal(output.events().begin(), output.events().end(), oracle.begin()));
    assert(handler.next_sequence() == 103 && handler.valid());
    std::cout << "normalization/ownership/duplicate/overlap: 3 oracle events passed\n";
}

void expect_bad(Bytes input, Status expected = Status::malformed) {
    Publisher output;
    Handler handler(7, 100, output);
    assert(handler.ingest(input) == expected);
    assert(output.events().empty() && handler.next_sequence() == 100 && !handler.valid());
    assert(handler.ingest(golden) == Status::blocked);
}

void verify_invalid_frames() {
    for (std::size_t length = 0; length < golden.size(); ++length) {
        expect_bad(Bytes(golden.data(), length));
    }
    auto extra = std::vector<std::uint8_t>(golden.begin(), golden.end());
    extra.push_back(0);
    expect_bad(extra);
    for (const auto& [offset, value] : std::array<std::pair<std::size_t, std::uint8_t>, 9>{{
             {0, 0}, {2, 2}, {3, 0}, {3, 5}, {7, 0}, {13, 3}, {26, 3}, {27, 1}, {35, 0}}}) {
        auto input = golden;
        input[offset] = value;
        expect_bad(input);
    }
    auto zero_price = golden;
    zero_price[18] = 0;
    zero_price[19] = 0;
    expect_bad(zero_price);
    auto exhausted = golden;
    std::fill(exhausted.begin() + 8, exhausted.begin() + 12, 0xff);
    expect_bad(exhausted, Status::sequence_range);
    auto zero_sequence = golden;
    zero_sequence[11] = 0;
    expect_bad(zero_sequence, Status::sequence_range);
    std::cout << "truncation/fields/late-record/range failures: no partial publication\n";
}

void verify_state_and_capacity() {
    constexpr std::array<WireQuote, 1> one{{{1, 1, 1, 1}}};
    Publisher output;
    Handler handler(7, 100, output);
    assert(handler.ingest(packet(8, 100, one)) == Status::wrong_session);
    assert(handler.valid() && handler.next_sequence() == 100 && output.events().empty());
    assert(handler.ingest(packet(7, 101, one)) == Status::gap);
    assert(!handler.valid() && handler.next_sequence() == 100 && output.events().empty());
    assert(handler.ingest(golden) == Status::blocked);

    Publisher bounded;
    Handler full(7, 100, bounded);
    constexpr std::array<WireQuote, 4> four{{one[0], one[0], one[0], one[0]}};
    assert(full.ingest(packet(7, 100, four)) == Status::published);
    assert(full.ingest(packet(7, 104, std::span(four).first(3))) == Status::published);
    assert(bounded.events().size() == 7 && full.next_sequence() == 107);
    assert(full.ingest(packet(7, 107, std::span(four).first(2))) == Status::backpressure);
    assert(bounded.events().size() == 7 && full.next_sequence() == 107 && !full.valid());

    Publisher final_output;
    constexpr auto maximum = std::numeric_limits<std::uint32_t>::max();
    Handler end(7, maximum - 1, final_output);
    assert(end.ingest(packet(7, maximum - 1, one)) == Status::published);
    assert(end.next_sequence() == maximum && final_output.events().size() == 1);
    assert(end.ingest(packet(7, maximum, one)) == Status::sequence_range);
    assert(!end.valid() && end.next_sequence() == maximum && final_output.events().size() == 1);
    std::cout << "session/gap/backpressure/sequence exhaustion: state checks passed\n";
}

int main() {
    verify_normalization_and_overlap();
    verify_invalid_frames();
    verify_state_and_capacity();
    std::cout << "FH1 teaching protocol only; no transport, recovery, or trading implemented\n";
}
