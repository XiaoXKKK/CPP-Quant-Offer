#include <algorithm>
#include <array>
#include <cstdint>
#include <iostream>
#include <optional>
#include <span>
#include <stdexcept>

namespace {
constexpr unsigned business_session = 7;
constexpr unsigned max_id = 16;
constexpr std::size_t queue_capacity = 4;
constexpr std::size_t frame_size = 5;
using Frame = std::array<std::uint8_t, frame_size>;
enum class Phase { queued, sending, awaiting_ack, unknown, final };
enum class Verdict { accepted, rejected, conflict };
enum class Enqueue { accepted, invalid, duplicate, full, frozen };
enum class Delivery { applied, duplicate, stale, unexpected, conflict };
enum class EvidenceKind { known, absent_unfenced, absent_fenced };
struct Receipt {
    unsigned session = 0;
    unsigned id = 0;
    unsigned quantity = 0;
    Verdict verdict = Verdict::rejected;
    bool operator==(const Receipt&) const = default;
};
struct Evidence {
    EvidenceKind kind;
    Receipt receipt;
};
void require(bool condition, const char* message) {
    if (!condition) {
        throw std::runtime_error(message);
    }
}

// Fictional peer: stable business-session ID and stored id+payload+result.
// 'accepted' acknowledges a request only; it is not a fill or settlement.
class Peer {
public:
    Receipt receive(unsigned session, unsigned id, unsigned quantity) noexcept {
        if (session != business_session || id == 0 || id > max_id || quantity == 0 || quantity > 8) {
            return {session, id, quantity, Verdict::conflict};
        }
        if (results_[id]) {
            if (results_[id]->quantity != quantity) {
                return {session, id, quantity, Verdict::conflict};
            }
            return *results_[id];
        }
        const Receipt result{session, id, quantity,
                             quantity == 8 ? Verdict::rejected : Verdict::accepted};
        results_[id] = result;
        ++applications_;
        return result;
    }
    Evidence lookup(unsigned id, unsigned quantity, bool old_transport_fenced) const noexcept {
        if (id <= max_id && results_[id]) {
            return {EvidenceKind::known, *results_[id]};
        }
        return {old_transport_fenced ? EvidenceKind::absent_fenced : EvidenceKind::absent_unfenced,
                {business_session, id, quantity, Verdict::rejected}};
    }
    unsigned applications() const noexcept { return applications_; }
private:
    std::array<std::optional<Receipt>, max_id + 1> results_{};
    unsigned applications_ = 0;
};

struct WriteResult {
    std::size_t bytes = 0;
    bool fatal = false;
};

// Synchronous, bounded transport script. Complete frames apply immediately.
// Closing discards its incomplete frame and fences all further work on it.
class ScriptTransport {
public:
    ScriptTransport(Peer& peer, unsigned generation) : peer_(peer), generation_(generation) {}
    unsigned generation() const noexcept { return generation_; }
    WriteResult write(std::span<const std::uint8_t> bytes, std::size_t allowance,
                      bool inject_fatal) noexcept {
        if (!open_ || inject_fatal) {
            close();
            return {0, true};
        }
        const auto n = std::min(bytes.size(), allowance);
        if (n > frame_size - used_) {
            close();
            return {0, true};
        }
        std::copy_n(bytes.begin(), n, frame_.begin() + static_cast<std::ptrdiff_t>(used_));
        used_ += n;
        if (used_ == frame_size) {
            const auto checksum = static_cast<std::uint8_t>(frame_[0] ^ frame_[1] ^ frame_[2] ^ frame_[3]);
            if (frame_[0] != 0xa5U || frame_[4] != checksum) {
                close();
                return {n, true};
            }
            ack_ = peer_.receive(frame_[1], frame_[2], frame_[3]);
            used_ = 0;
        }
        return {n, false};
    }
    void close() noexcept { open_ = false; used_ = 0; }
    std::optional<Receipt> take_ack() noexcept {
        auto result = ack_;
        ack_.reset();
        return result;
    }
    Evidence query_after_close(unsigned id, unsigned quantity) const {
        require(!open_, "absence proof requires old transport to be fenced");
        return peer_.lookup(id, quantity, true);
    }
private:
    Peer& peer_;
    unsigned generation_;
    Frame frame_{};
    std::size_t used_ = 0;
    bool open_ = true;
    std::optional<Receipt> ack_;
};

class Gateway {
    struct Entry {
        unsigned id = 0;
        unsigned quantity = 0;
        Frame frame{};
        std::size_t offset = 0;
        Phase phase = Phase::queued;
    };
public:
    bool connect(unsigned remote_session) noexcept {
        if (frozen_ || connected_) {
            return false;
        }
        if (remote_session != business_session || generation_ == 16) {
            frozen_ = true;
            return false;
        }
        ++generation_;
        connected_ = true;
        return true;
    }
    unsigned generation() const noexcept { return generation_; }
    bool frozen() const noexcept { return frozen_; }
    std::size_t size() const noexcept { return size_; }
    Phase phase() const {
        require(size_ != 0, "empty gateway has no front phase");
        return queue_[0].phase;
    }
    std::size_t offset() const { require(size_ != 0, "empty offset"); return queue_[0].offset; }
    Enqueue enqueue(unsigned id, unsigned quantity) noexcept {
        if (frozen_) { return Enqueue::frozen; }
        if (id == 0 || id > max_id || quantity == 0 || quantity > 8) { return Enqueue::invalid; }
        if (seen_[id]) { return Enqueue::duplicate; }
        if (size_ == queue_capacity) { return Enqueue::full; }
        Frame frame{0xa5U, static_cast<std::uint8_t>(business_session),
                    static_cast<std::uint8_t>(id), static_cast<std::uint8_t>(quantity), 0};
        frame[4] = static_cast<std::uint8_t>(frame[0] ^ frame[1] ^ frame[2] ^ frame[3]);
        queue_[size_++] = {id, quantity, frame, 0, Phase::queued};
        seen_[id] = true;
        return Enqueue::accepted;
    }
    void disconnect(unsigned generation) noexcept {
        if (!connected_ || generation != generation_) { return; }
        connected_ = false;
        if (size_ != 0 && queue_[0].phase != Phase::final && queue_[0].offset != 0) {
            queue_[0].phase = Phase::unknown;
        }
    }
    bool pump(ScriptTransport& transport, std::size_t allowance, bool fatal = false) noexcept {
        if (!connected_ || frozen_ || size_ == 0 || transport.generation() != generation_) {
            return false;
        }
        auto& front = queue_[0];
        if (front.phase != Phase::queued && front.phase != Phase::sending) { return false; }
        const auto result = transport.write(std::span<const std::uint8_t>(front.frame).subspan(front.offset),
                                            allowance, fatal);
        front.offset += result.bytes;
        if (result.fatal) {
            disconnect(generation_);
            return true;
        }
        front.phase = front.offset == frame_size ? Phase::awaiting_ack
                       : front.offset == 0 ? Phase::queued : Phase::sending;
        return true;
    }
    Delivery acknowledge(unsigned generation, Receipt receipt) noexcept {
        if (!connected_ || generation != generation_) { return Delivery::stale; }
        if (frozen_) { return Delivery::unexpected; }
        if (receipt.session != business_session ||
            (receipt.verdict != Verdict::accepted && receipt.verdict != Verdict::rejected)) {
            frozen_ = true;
            return Delivery::conflict;
        }
        if (receipt.id == 0 || receipt.id > max_id) { return Delivery::unexpected; }
        if (completed_[receipt.id]) {
            if (*completed_[receipt.id] == receipt) { return Delivery::duplicate; }
            frozen_ = true;
            return Delivery::conflict;
        }
        if (size_ == 0 || queue_[0].id != receipt.id || queue_[0].phase != Phase::awaiting_ack) {
            return Delivery::unexpected;
        }
        if (queue_[0].quantity != receipt.quantity) {
            frozen_ = true;
            return Delivery::conflict;
        }
        finalize(receipt);
        return Delivery::applied;
    }
    bool resolve(unsigned generation, Evidence evidence) noexcept {
        if (!connected_ || frozen_ || generation != generation_ || size_ == 0 ||
            queue_[0].phase != Phase::unknown) { return false; }
        auto& front = queue_[0];
        const auto receipt = evidence.receipt;
        if (receipt.session != business_session || receipt.id != front.id ||
            receipt.quantity != front.quantity ||
            (receipt.verdict != Verdict::accepted && receipt.verdict != Verdict::rejected) ||
            (evidence.kind != EvidenceKind::known && evidence.kind != EvidenceKind::absent_fenced &&
             evidence.kind != EvidenceKind::absent_unfenced)) {
            frozen_ = true;
            return false;
        }
        if (evidence.kind == EvidenceKind::known) {
            finalize(receipt);
            return true;
        }
        if (evidence.kind == EvidenceKind::absent_fenced) {
            front.offset = 0;
            front.phase = Phase::queued;
            return true;
        }
        return false; // A plain NotFound is not proof that old work cannot still apply.
    }
    std::optional<Receipt> pop_result() noexcept {
        if (size_ == 0 || queue_[0].phase != Phase::final) { return std::nullopt; }
        const auto result = completed_[queue_[0].id];
        for (std::size_t i = 1; i < size_; ++i) { queue_[i - 1] = queue_[i]; }
        queue_[--size_] = {};
        return result;
    }
private:
    void finalize(Receipt receipt) noexcept {
        completed_[receipt.id] = receipt;
        queue_[0].phase = Phase::final;
    }
    std::array<Entry, queue_capacity> queue_{};
    std::array<bool, max_id + 1> seen_{};
    std::array<std::optional<Receipt>, max_id + 1> completed_{};
    std::size_t size_ = 0;
    unsigned generation_ = 0;
    bool connected_ = false;
    bool frozen_ = false;
};

void partial_write_tests() {
    // All 16 ways to place cuts between the five frame bytes.
    for (unsigned pattern = 0; pattern < 16; ++pattern) {
        Peer peer;
        Gateway gateway;
        require(gateway.enqueue(1, 3) == Enqueue::accepted, "enqueue");
        require(gateway.connect(business_session), "connect");
        ScriptTransport transport(peer, gateway.generation());
        std::size_t previous = 0;
        for (std::size_t position = 1; position <= frame_size; ++position) {
            if (position == frame_size || (pattern & (1U << (position - 1))) != 0) {
                require(gateway.pump(transport, 0), "would-block script");
                require(gateway.offset() == previous, "would-block changed offset");
                require(gateway.pump(transport, position - previous), "write chunk");
                require(gateway.offset() == position, "partial offset");
                require(peer.applications() == (position == frame_size ? 1U : 0U), "early apply");
                previous = position;
            }
        }
        require(gateway.phase() == Phase::awaiting_ack && !gateway.pop_result(), "send is not ack");
        const auto receipt = transport.take_ack();
        require(receipt && *receipt == Receipt{7, 1, 3, Verdict::accepted}, "script receipt");
        require(gateway.acknowledge(gateway.generation(), *receipt) == Delivery::applied, "app ack");
        require(gateway.pop_result() == receipt && gateway.size() == 0, "result consumption");
        require(gateway.acknowledge(gateway.generation(), *receipt) == Delivery::duplicate, "duplicate ack");
        require(gateway.enqueue(1, 3) == Enqueue::duplicate, "business ID reused");
    }
}

void disconnect_tests() {
    for (std::size_t sent = 0; sent <= frame_size; ++sent) {
        Peer peer;
        Gateway gateway;
        gateway.enqueue(1, 4);
        gateway.enqueue(2, 2);
        require(gateway.connect(7), "first connect");
        const auto old_generation = gateway.generation();
        ScriptTransport old(peer, old_generation);
        gateway.pump(old, sent);
        const auto delayed = old.take_ack();
        old.close();
        gateway.disconnect(old_generation);
        require(gateway.phase() == (sent == 0 ? Phase::queued : Phase::unknown), "failure classification");
        require(gateway.connect(7), "same business-session reconnect");
        ScriptTransport current(peer, gateway.generation());
        require(!gateway.pump(old, 5), "old transport reused");
        if (delayed) {
            require(gateway.acknowledge(old_generation, *delayed) == Delivery::stale, "late old ack");
        }
        if (sent != 0) {
            require(!gateway.pump(current, 5), "unknown retried automatically");
            if (sent < frame_size) {
                require(!gateway.resolve(gateway.generation(), peer.lookup(1, 4, false)), "unfenced absence");
            }
            require(gateway.resolve(gateway.generation(), old.query_after_close(1, 4)), "authoritative resolution");
        }
        if (gateway.phase() == Phase::queued) {
            gateway.pump(current, 5);
            const auto ack = current.take_ack();
            require(ack && gateway.acknowledge(gateway.generation(), *ack) == Delivery::applied, "retry result");
        }
        require(peer.applications() == 1 && gateway.pop_result() == Receipt{7, 1, 4, Verdict::accepted},
                "exact expected first business application");
        require(gateway.phase() == Phase::queued, "second command overtook first");
        gateway.pump(current, 5);
        const auto ack = current.take_ack();
        require(ack && gateway.acknowledge(gateway.generation(), *ack) == Delivery::applied, "second ack");
        require(peer.applications() == 2 && gateway.pop_result() == Receipt{7, 2, 2, Verdict::accepted},
                "ordered second result");
    }
}

void boundary_tests() {
    Peer peer;
    Gateway gateway;
    require(gateway.enqueue(0, 1) == Enqueue::invalid && gateway.enqueue(17, 1) == Enqueue::invalid &&
                gateway.enqueue(1, 0) == Enqueue::invalid && gateway.enqueue(1, 9) == Enqueue::invalid,
            "input boundaries");
    for (unsigned id = 1; id <= 4; ++id) { require(gateway.enqueue(id, 8) == Enqueue::accepted, "fill queue"); }
    require(gateway.enqueue(5, 1) == Enqueue::full, "bounded backpressure");
    gateway.connect(7);
    ScriptTransport transport(peer, gateway.generation());
    require(gateway.acknowledge(gateway.generation(), {7, 1, 8, Verdict::rejected}) == Delivery::unexpected,
            "ack before send");
    gateway.pump(transport, 5, true);
    require(gateway.phase() == Phase::queued && peer.applications() == 0, "fatal before bytes");
    gateway.connect(7);
    ScriptTransport current(peer, gateway.generation());
    gateway.pump(current, 5);
    const auto receipt = current.take_ack();
    require(receipt && receipt->verdict == Verdict::rejected, "business rejection");
    require(gateway.acknowledge(gateway.generation(), *receipt) == Delivery::applied, "reject ack");
    require(gateway.enqueue(5, 1) == Enqueue::full, "unconsumed result uses capacity");
    require(gateway.pop_result() == receipt && gateway.enqueue(5, 1) == Enqueue::accepted, "consumer releases capacity");
    require(peer.receive(7, 1, 8) == *receipt && peer.applications() == 1, "peer duplicate result");
    require(peer.receive(7, 1, 7).verdict == Verdict::conflict && peer.applications() == 1, "payload conflict");
    require(gateway.acknowledge(gateway.generation(), {7, 1, 8, Verdict::accepted}) == Delivery::conflict &&
                gateway.frozen(), "contradictory reply freezes");
    Gateway changed;
    require(!changed.connect(8) && changed.frozen(), "business-session change requires reconciliation");
    Gateway epochs;
    for (unsigned i = 0; i < 16; ++i) {
        require(epochs.connect(7), "bounded connection generation");
        epochs.disconnect(epochs.generation());
    }
    require(!epochs.connect(7) && epochs.frozen(), "generation exhaustion must not wrap");
}

void callback_validation_tests() {
    {
        Peer peer;
        Gateway gateway;
        gateway.enqueue(1, 1);
        gateway.connect(7);
        ScriptTransport transport(peer, gateway.generation());
        gateway.pump(transport, 5);
        require(gateway.acknowledge(gateway.generation(), {7, 1, 1, static_cast<Verdict>(99)}) ==
                    Delivery::conflict && gateway.frozen() && !gateway.pop_result(),
                "invalid ack verdict must not finalize");
    }
    for (const bool invalid_kind : {false, true}) {
        Peer peer;
        Gateway gateway;
        gateway.enqueue(1, 1);
        gateway.connect(7);
        const auto old_generation = gateway.generation();
        ScriptTransport transport(peer, old_generation);
        gateway.pump(transport, 1);
        transport.close();
        gateway.disconnect(old_generation);
        gateway.connect(7);
        const auto absent = transport.query_after_close(1, 1);
        require(!gateway.resolve(old_generation, absent) && gateway.phase() == Phase::unknown &&
                    !gateway.frozen() && !gateway.pop_result(), "stale query callback changed state");
        Evidence bad{EvidenceKind::known, {7, 1, 1, Verdict::accepted}};
        if (invalid_kind) {
            bad.kind = static_cast<EvidenceKind>(99);
        } else {
            bad.receipt.verdict = static_cast<Verdict>(99);
        }
        require(!gateway.resolve(gateway.generation(), bad) && gateway.frozen() &&
                    gateway.phase() == Phase::unknown && !gateway.pop_result(),
                "invalid query evidence must not finalize");
    }
}
} // namespace

int main() {
    try {
        partial_write_tests();
        disconnect_tests();
        boundary_tests();
        callback_validation_tests();
        std::cout << "16 frame partitions, 6 disconnect positions, session/ID/backpressure: OK\n"
                  << "Script transport only; no network or real orders.\n";
    } catch (const std::exception& error) {
        std::cerr << "ERROR: " << error.what() << '\n';
        return 1;
    }
}
