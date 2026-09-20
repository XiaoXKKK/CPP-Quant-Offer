#include <algorithm>
#include <array>
#include <cassert>
#include <cerrno>
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <limits>
#include <optional>
#include <span>
#include <stdexcept>
#include <system_error>

#include <fcntl.h>
#include <poll.h>
#include <sys/socket.h>
#include <unistd.h>

// Teaching protocol: one event per 12-byte datagram, all integers big-endian.
// This is not a wire format from an exchange. Sequence is modulo 2^16.
constexpr std::size_t wire_size = 12;
constexpr std::uint16_t max_delta = 1000;
using Frame = std::array<unsigned char, wire_size>;

struct Packet {
    std::uint8_t channel;
    std::uint32_t session;
    std::uint16_t sequence;
    std::uint16_t delta;
};

Frame encode(Packet p) {
    Frame bytes{0x55, 0x44, 1, p.channel};
    for (unsigned i = 0; i < 4; ++i) {
        bytes[4 + i] = static_cast<unsigned char>(p.session >> (24U - 8U * i));
    }
    bytes[8] = static_cast<unsigned char>(p.sequence >> 8U);
    bytes[9] = static_cast<unsigned char>(p.sequence);
    bytes[10] = static_cast<unsigned char>(p.delta >> 8U);
    bytes[11] = static_cast<unsigned char>(p.delta);
    return bytes;
}

std::optional<Packet> decode(std::span<const unsigned char> bytes) noexcept {
    if (bytes.size() != wire_size || bytes[0] != 0x55 || bytes[1] != 0x44 ||
        bytes[2] != 1) {
        return std::nullopt;
    }
    std::uint32_t session = 0;
    for (std::size_t i = 4; i < 8; ++i) {
        session = (session << 8U) | bytes[i];
    }
    const auto sequence = static_cast<std::uint16_t>(
        static_cast<unsigned>(bytes[8]) * 256U + bytes[9]);
    const auto delta = static_cast<std::uint16_t>(
        static_cast<unsigned>(bytes[10]) * 256U + bytes[11]);
    if (delta > max_delta) {
        return std::nullopt;
    }
    return Packet{bytes[3], session, sequence, delta};
}

enum class Outcome {
    no_baseline, applied, old_or_duplicate, gap, ambiguous, other_session,
    other_channel, needs_recovery, malformed, truncated, overflow
};

struct State {
    bool initialized = false;
    bool valid = false;
    std::uint32_t session = 0;
    std::uint16_t next = 0;
    std::uint64_t value = 0;
    bool operator==(const State&) const = default;
};

class Receiver {
public:
    explicit Receiver(std::uint8_t channel) noexcept : channel_(channel) {}

    // A trusted caller installs a complete snapshot and its exact next sequence.
    // Ordinary datagrams never authorize a session switch or recovery completion.
    void install_baseline(std::uint32_t session, std::uint16_t next,
                          std::uint64_t snapshot_value) noexcept {
        state_ = State{true, true, session, next, snapshot_value};
    }

    Outcome ingest(std::span<const unsigned char> bytes,
                   bool was_truncated = false) noexcept {
        if (was_truncated) {
            state_.valid = false;
            return Outcome::truncated;
        }
        const auto packet = decode(bytes);
        if (!packet) {
            state_.valid = false;
            return Outcome::malformed;
        }
        if (packet->channel != channel_) {
            return Outcome::other_channel;
        }
        if (!state_.initialized) {
            return Outcome::no_baseline;
        }
        if (packet->session != state_.session) {
            return Outcome::other_session;
        }
        if (!state_.valid) {
            return Outcome::needs_recovery;
        }
        const auto distance =
            (static_cast<std::uint32_t>(packet->sequence) + 65536U - state_.next)
            % 65536U;
        if (distance == 32768U) {
            state_.valid = false;
            return Outcome::ambiguous;
        }
        if (distance > 32768U) {
            return Outcome::old_or_duplicate;
        }
        if (distance != 0) {
            state_.valid = false;
            return Outcome::gap;
        }
        if (packet->delta > std::numeric_limits<std::uint64_t>::max() - state_.value) {
            state_.valid = false;
            return Outcome::overflow;
        }
        state_.value += packet->delta;
        state_.next = static_cast<std::uint16_t>(
            (static_cast<std::uint32_t>(state_.next) + 1U) % 65536U);
        return Outcome::applied;
    }

    State state() const noexcept { return state_; }

private:
    std::uint8_t channel_;
    State state_{};
};

void check_state_machine() {
    Receiver receiver(7);
    assert(receiver.ingest(encode({7, 77, 0, 1})) == Outcome::no_baseline);
    receiver.install_baseline(77, 65535, 100);
    assert(receiver.ingest(encode({7, 77, 65535, 2})) == Outcome::applied);
    assert(receiver.state().next == 0 && receiver.state().value == 102);
    assert(receiver.ingest(encode({7, 77, 65535, 2})) == Outcome::old_or_duplicate);
    assert(receiver.ingest(encode({7, 77, 0, 3})) == Outcome::applied);
    assert(receiver.state().next == 1 && receiver.state().value == 105);

    const State before_gap = receiver.state();
    assert(receiver.ingest(encode({7, 77, 2, 4})) == Outcome::gap);
    assert(!receiver.state().valid);
    assert(receiver.state().next == before_gap.next);
    assert(receiver.state().value == before_gap.value);
    assert(receiver.ingest(encode({7, 77, 1, 5})) == Outcome::needs_recovery);
    assert(receiver.ingest(encode({7, 77, 3, 6})) == Outcome::needs_recovery);

    // Model a separately verified snapshot/handoff covering all earlier events.
    receiver.install_baseline(77, 3, 200);
    assert(receiver.ingest(encode({7, 77, 3, 4})) == Outcome::applied);
    const State recovered = receiver.state();
    assert(recovered.valid && recovered.next == 4 && recovered.value == 204);
    assert(receiver.ingest(encode({8, 77, 4, 1})) == Outcome::other_channel);
    assert(receiver.ingest(encode({7, 78, 0, 1})) == Outcome::other_session);
    assert(receiver.state() == recovered);
    receiver.install_baseline(78, 0, 10);
    assert(receiver.ingest(encode({7, 77, 4, 1})) == Outcome::other_session);
    assert(receiver.ingest(encode({7, 78, 0, 0})) == Outcome::applied);
    assert(receiver.state().value == 10 && receiver.state().next == 1);

    receiver.install_baseline(78, 0, 0);
    assert(receiver.ingest(encode({7, 78, 32767, 1})) == Outcome::gap);
    receiver.install_baseline(78, 0, 0);
    assert(receiver.ingest(encode({7, 78, 32768, 1})) == Outcome::ambiguous);
    receiver.install_baseline(78, 0, 0);
    assert(receiver.ingest(encode({7, 78, 32769, 1})) == Outcome::old_or_duplicate);
    receiver.install_baseline(78, 65535, 0);
    assert(receiver.ingest(encode({7, 78, 0, 1})) == Outcome::gap);

    receiver.install_baseline(78, 0, std::numeric_limits<std::uint64_t>::max());
    assert(receiver.ingest(encode({7, 78, 0, 1})) == Outcome::overflow);
    assert(!receiver.state().valid && receiver.state().next == 0);
    assert(receiver.state().value == std::numeric_limits<std::uint64_t>::max());
}

void check_parser_boundaries() {
    const Frame valid = encode({7, 0x12345678U, 65535, max_delta});
    const auto decoded = decode(valid);
    assert(decoded && decoded->session == 0x12345678U);
    assert(decoded->sequence == 65535 && decoded->delta == max_delta);
    for (std::size_t length = 0; length < wire_size; ++length) {
        Receiver receiver(7);
        receiver.install_baseline(1, 0, 0);
        assert(receiver.ingest(std::span(valid).first(length)) == Outcome::malformed);
        assert(!receiver.state().valid && receiver.state().next == 0);
    }
    std::array<unsigned char, wire_size + 1> oversized{};
    std::copy(valid.begin(), valid.end(), oversized.begin());
    assert(!decode(oversized));
    auto bad_magic = valid;
    bad_magic[0] = 0;
    assert(!decode(bad_magic));
    auto bad_version = valid;
    bad_version[2] = 2;
    assert(!decode(bad_version));
    assert(!decode(encode({7, 1, 0, max_delta + 1})));

    // All six permutations: only a contiguous prefix can be applied.
    std::array<std::uint16_t, 3> order{0, 1, 2};
    do {
        Receiver receiver(7);
        receiver.install_baseline(1, 0, 0);
        std::uint16_t prefix = 0;
        bool gap = false;
        for (const auto seq : order) {
            const auto result = receiver.ingest(encode({7, 1, seq, 1}));
            if (gap) {
                assert(result == Outcome::needs_recovery);
            } else if (seq == prefix) {
                assert(result == Outcome::applied);
                ++prefix;
            } else {
                assert(result == Outcome::gap);
                gap = true;
            }
        }
        assert(receiver.state().next == prefix && receiver.state().value == prefix);
        assert(receiver.state().valid == !gap);
    } while (std::next_permutation(order.begin(), order.end()));
}

class UniqueFd {
public:
    UniqueFd() noexcept = default;
    UniqueFd(const UniqueFd&) = delete;
    UniqueFd& operator=(const UniqueFd&) = delete;
    ~UniqueFd() { reset(); }
    int get() const noexcept { return fd_; }
    void reset(int replacement = -1) noexcept {
        if (fd_ >= 0) {
            ::close(fd_);  // Linux: do not retry close on EINTR.
        }
        fd_ = replacement;
    }
private:
    int fd_ = -1;
};

struct DatagramPair {
    UniqueFd sender;
    UniqueFd receiver;
    DatagramPair() {
        int raw[2]{};
        if (::socketpair(AF_UNIX, SOCK_DGRAM | SOCK_NONBLOCK | SOCK_CLOEXEC, 0, raw) < 0) {
            throw std::system_error(errno, std::generic_category(), "socketpair");
        }
        sender.reset(raw[0]);
        receiver.reset(raw[1]);
    }
};

void send_datagram(int fd, std::span<const unsigned char> bytes) {
    const auto sent = ::send(fd, bytes.data(), bytes.size(), MSG_DONTWAIT | MSG_NOSIGNAL);
    if (sent < 0) {
        throw std::system_error(errno, std::generic_category(), "send");
    }
    if (static_cast<std::size_t>(sent) != bytes.size()) {
        throw std::runtime_error("unexpected partial datagram send");
    }
}

struct Received {
    std::size_t copied;
    int flags;
};

Received receive_datagram(int fd, std::span<unsigned char> buffer) {
    pollfd descriptor{fd, POLLIN, 0};
    const int ready = ::poll(&descriptor, 1, 1000);
    if (ready < 0) {
        throw std::system_error(errno, std::generic_category(), "poll");
    }
    if (ready == 0) {
        throw std::runtime_error("datagram receive timed out");
    }
    if ((descriptor.revents & (POLLERR | POLLHUP | POLLNVAL)) != 0 ||
        (descriptor.revents & POLLIN) == 0) {
        throw std::runtime_error("unexpected poll events");
    }
    iovec vector{buffer.data(), buffer.size()};
    msghdr message{};
    message.msg_iov = &vector;
    message.msg_iovlen = 1;
    // MSG_TRUNC is inspected as an OUTPUT flag, not supplied as an input flag.
    const auto size = ::recvmsg(fd, &message, MSG_DONTWAIT);
    if (size < 0) {
        throw std::system_error(errno, std::generic_category(), "recvmsg");
    }
    if (static_cast<std::size_t>(size) > buffer.size()) {
        throw std::runtime_error("copied length exceeds supplied buffer");
    }
    return {static_cast<std::size_t>(size), message.msg_flags};
}

void check_local_datagrams() {
    DatagramPair pair;
    const auto first = encode({7, 77, 100, 1});
    const auto second = encode({7, 77, 101, 2});
    send_datagram(pair.sender.get(), first);
    send_datagram(pair.sender.get(), second);

    std::array<unsigned char, 8> small{};
    const auto cut = receive_datagram(pair.receiver.get(), small);
    assert(cut.copied == small.size() && (cut.flags & MSG_TRUNC) != 0);
    Receiver receiver(7);
    receiver.install_baseline(77, 100, 0);
    assert(receiver.ingest(std::span(small).first(cut.copied), true) == Outcome::truncated);
    assert(!receiver.state().valid && receiver.state().next == 100);

    Frame full{};
    const auto complete = receive_datagram(pair.receiver.get(), full);
    assert(complete.copied == full.size() && (complete.flags & MSG_TRUNC) == 0);
    assert(full == second);  // The remainder of first was discarded, not queued.
    assert(receiver.ingest(full) == Outcome::needs_recovery);

    send_datagram(pair.sender.get(), {});
    const auto empty = receive_datagram(pair.receiver.get(), full);
    assert(empty.copied == 0 && (empty.flags & MSG_TRUNC) == 0);
    assert(receiver.ingest(std::span(full).first(0)) == Outcome::malformed);

    unsigned char probe{};
    const auto absent = ::recv(pair.receiver.get(), &probe, 1, MSG_DONTWAIT);
    const int absent_error = errno;
    assert(absent == -1 && (absent_error == EAGAIN || absent_error == EWOULDBLOCK));
}

void check_exception_cleanup() {
    struct InjectedFailure {};
    int first = -1;
    int second = -1;
    bool caught = false;
    try {
        DatagramPair pair;
        first = pair.sender.get();
        second = pair.receiver.get();
        throw InjectedFailure{};
    } catch (const InjectedFailure&) {
        caught = true;
    }
    assert(caught && first >= 0 && second >= 0);
    const int first_result = ::fcntl(first, F_GETFD);
    const int first_error = errno;
    const int second_result = ::fcntl(second, F_GETFD);
    const int second_error = errno;
    assert(first_result == -1 && first_error == EBADF);
    assert(second_result == -1 && second_error == EBADF);
}

int main() {
    try {
        check_state_machine();
        check_parser_boundaries();
        check_local_datagrams();
        check_exception_cleanup();
        std::cout << "Sequence boundaries, recovery gate, local datagram truncation and fd cleanup: OK\n";
    } catch (const std::exception& error) {
        std::cerr << "test failed: " << error.what() << '\n';
        return 1;
    }
}
