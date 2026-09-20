#include <algorithm>
#include <array>
#include <cassert>
#include <cerrno>
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <optional>
#include <span>
#include <stdexcept>
#include <system_error>
#include <utility>
#include <sys/socket.h>
#include <unistd.h>

using Byte = std::uint8_t;
constexpr std::size_t max_body = 16;
struct Frame {
    std::array<Byte, max_body> bytes{};
    std::size_t size = 0;
    bool operator==(const Frame&) const = default;
};
enum class Status { need_input, frame_ready, failed, ended };
enum class ParseError { none, oversized, truncated };
struct FeedResult { std::size_t consumed; Status status; };

// Four-byte big-endian body length, 0..16 bytes. At most one ready frame.
class Decoder {
    std::uint32_t length_ = 0;
    std::size_t header_bytes_ = 0;
    Frame frame_{};
    Status status_ = Status::need_input;
    ParseError error_ = ParseError::none;
    bool eof_ = false;
public:
    FeedResult feed(std::span<const Byte> input) noexcept {
        if (status_ != Status::need_input) return {0, status_};
        std::size_t used = 0;
        while (used < input.size()) {
            if (header_bytes_ < 4) {
                length_ = (length_ << 8) | input[used++];
                if (++header_bytes_ != 4) continue;
                if (length_ > max_body) {
                    error_ = ParseError::oversized;
                    status_ = Status::failed;
                    break;
                }
                if (length_ == 0) {
                    status_ = Status::frame_ready;
                    break;
                }
            } else {
                frame_.bytes[frame_.size++] = input[used++];
                if (frame_.size == length_) {
                    status_ = Status::frame_ready;
                    break;
                }
            }
        }
        return {used, status_};
    }

    std::optional<Frame> take() noexcept {
        if (status_ != Status::frame_ready) return std::nullopt;
        Frame result = frame_;
        frame_ = {};
        length_ = 0;
        header_bytes_ = 0;
        status_ = eof_ ? Status::ended : Status::need_input;
        return result;
    }

    // Call only after all bytes returned by recv have been fed and consumed.
    Status finish() noexcept {
        eof_ = true;
        if (status_ == Status::failed || status_ == Status::ended ||
            status_ == Status::frame_ready) return status_;
        if (header_bytes_ != 0) {
            error_ = ParseError::truncated;
            status_ = Status::failed;
        } else status_ = Status::ended;
        return status_;
    }
    ParseError error() const noexcept { return error_; }
};

struct Wire {
    std::array<Byte, 64> bytes{};
    std::size_t size = 0;
    std::span<const Byte> view() const noexcept { return {bytes.data(), size}; }
    bool append(std::span<const Byte> body) noexcept {
        if (body.size() > max_body) return false;
        const auto needed = 4 + body.size(); // body was bounded before addition.
        if (needed > bytes.size() - size) return false;
        const auto length = static_cast<std::uint32_t>(body.size());
        for (unsigned shift : {24U, 16U, 8U, 0U})
            bytes[size++] = static_cast<Byte>((length >> shift) & 0xffU);
        for (Byte byte : body) bytes[size++] = byte;
        return true;
    }
};

struct Frames {
    std::array<Frame, 4> values{};
    std::size_t size = 0;
    void add(const Frame& frame) { assert(size < values.size()); values[size++] = frame; }
    bool operator==(const Frames&) const = default;
};

void feed_all(Decoder& decoder, std::span<const Byte> bytes, Frames& output) {
    while (!bytes.empty()) {
        const auto result = decoder.feed(bytes);
        assert(result.status != Status::failed && result.status != Status::ended);
        bytes = bytes.subspan(result.consumed);
        if (result.status == Status::frame_ready) output.add(*decoder.take());
        else assert(result.consumed != 0);
    }
}

Wire test_wire() {
    Wire wire;
    const std::array<Byte, 3> small{0x41, 0x00, 0xff};
    std::array<Byte, max_body> large{};
    for (std::size_t i = 0; i < large.size(); ++i) large[i] = static_cast<Byte>(i);
    const bool a = wire.append(small);
    const bool b = wire.append({});
    const bool c = wire.append(large);
    assert(a && b && c && wire.size == 31);
    return wire;
}

Frames parser_tests(const Wire& wire) {
    Decoder baseline;
    Frames expected;
    feed_all(baseline, wire.view(), expected);
    assert(baseline.finish() == Status::ended && expected.size == 3);
    assert(expected.values[0].size == 3 && expected.values[0].bytes[1] == 0);
    assert(expected.values[1].size == 0 && expected.values[2].size == max_body);
    for (std::size_t split = 0; split <= wire.size; ++split) {
        Decoder decoder;
        Frames actual;
        feed_all(decoder, wire.view().first(split), actual);
        feed_all(decoder, wire.view().subspan(split), actual);
        assert(actual == expected && decoder.finish() == Status::ended);
    }
    Decoder bytewise;
    Frames bytewise_output;
    for (std::size_t i = 0; i < wire.size; ++i)
        feed_all(bytewise, wire.view().subspan(i, 1), bytewise_output);
    assert(bytewise_output == expected && bytewise.finish() == Status::ended);

    Decoder held;
    const auto first = held.feed(wire.view());
    assert(first.status == Status::frame_ready && first.consumed == 7);
    const auto remaining = wire.view().subspan(first.consumed);
    const auto blocked = held.feed(remaining);
    assert(blocked.consumed == 0 && blocked.status == Status::frame_ready);
    assert(*held.take() == expected.values[0]);
    Frames rest;
    feed_all(held, remaining, rest);
    assert(rest.size == 2 && held.finish() == Status::ended);

    for (const auto bad : {std::array<Byte, 4>{0, 0, 0, 17},
                           std::array<Byte, 4>{0xff, 0xff, 0xff, 0xff}}) {
        Decoder decoder;
        const auto result = decoder.feed(bad);
        assert(result.consumed == 4 && result.status == Status::failed);
        assert(decoder.error() == ParseError::oversized);
        assert(decoder.feed(wire.view()).consumed == 0);
        assert(decoder.finish() == Status::failed);
    }
    for (std::size_t prefix = 0; prefix <= 7; ++prefix) {
        Decoder decoder;
        const auto result = decoder.feed(wire.view().first(prefix));
        assert(result.consumed == prefix);
        if (prefix == 0) assert(decoder.finish() == Status::ended);
        else if (prefix == 7) {
            assert(decoder.finish() == Status::frame_ready);
            assert(*decoder.take() == expected.values[0]);
            assert(decoder.finish() == Status::ended);
        } else {
            assert(decoder.finish() == Status::failed);
            assert(decoder.error() == ParseError::truncated);
        }
        assert(decoder.feed(wire.view()).consumed == 0);
    }
    Wire bounded;
    std::array<Byte, max_body> full{};
    for (int i = 0; i < 3; ++i) { const bool ok = bounded.append(full); assert(ok); }
    const bool overflow = bounded.append(full);
    const std::array<Byte, max_body + 1> too_large{};
    const bool invalid = bounded.append(too_large);
    assert(!overflow && !invalid && bounded.size == 60);
    return expected;
}

struct IoResult { ssize_t count; int error; };
enum class WriteState { complete, would_block, retry_later, failed };
struct WriteResult { WriteState state; int error = 0; };
bool would_block(int error) noexcept { return error == EAGAIN || error == EWOULDBLOCK; }

template <class Send>
WriteResult flush(std::span<const Byte> bytes, std::size_t& offset, Send send) {
    assert(offset <= bytes.size());
    for (unsigned attempt = 0; offset < bytes.size() && attempt < 16; ++attempt) {
        const auto result = send(bytes.subspan(offset));
        if (result.count > 0) {
            const auto sent = static_cast<std::size_t>(result.count);
            if (sent > bytes.size() - offset) return {WriteState::failed, EIO};
            offset += sent;
        } else if (result.count == 0) return {WriteState::failed, EIO};
        else if (result.error == EINTR) continue;
        else if (would_block(result.error)) return {WriteState::would_block};
        else return {WriteState::failed, result.error};
    }
    return {offset == bytes.size() ? WriteState::complete : WriteState::retry_later};
}

void scripted_send_test() {
    const std::array<Byte, 6> bytes{1, 2, 3, 4, 5, 6};
    const std::array<IoResult, 5> script{{{-1, EINTR}, {2, 0}, {-1, EAGAIN}, {1, 0}, {3, 0}}};
    std::array<Byte, 6> written{};
    std::size_t step = 0, copied = 0, offset = 0;
    auto send = [&](std::span<const Byte> input) {
        assert(step < script.size());
        const auto result = script[step++];
        if (result.count > 0) {
            const auto n = static_cast<std::size_t>(result.count);
            assert(n <= input.size() && n <= written.size() - copied);
            std::copy_n(input.begin(), n, written.begin() + static_cast<std::ptrdiff_t>(copied));
            copied += n;
        }
        return result;
    };
    assert(flush(bytes, offset, send).state == WriteState::would_block && offset == 2);
    assert(flush(bytes, offset, send).state == WriteState::complete && written == bytes);
    offset = 0;
    assert(flush(bytes, offset, [](auto) { return IoResult{-1, EINTR}; }).state == WriteState::retry_later);
    assert(offset == 0);
    assert(flush(bytes, offset, [](auto) { return IoResult{0, 0}; }).state == WriteState::failed);
}

class Fd {
    int fd_ = -1;
public:
    explicit Fd(int fd) noexcept : fd_(fd) {}
    Fd(const Fd&) = delete;
    Fd& operator=(const Fd&) = delete;
    Fd(Fd&& other) noexcept : fd_(std::exchange(other.fd_, -1)) {}
    Fd& operator=(Fd&&) = delete;
    ~Fd() { reset(); }
    int get() const noexcept { return fd_; }
    void reset() noexcept {
        const int old = std::exchange(fd_, -1);
        if (old >= 0) static_cast<void>(::close(old)); // Linux: do not retry close.
    }
};

std::array<Fd, 2> socket_pair() {
    int descriptors[2];
    if (::socketpair(AF_UNIX, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, 0, descriptors) < 0)
        throw std::system_error(errno, std::generic_category(), "socketpair");
    return {Fd(descriptors[0]), Fd(descriptors[1])};
}

IoResult send_some(int fd, std::span<const Byte> bytes) {
    const auto count = ::send(fd, bytes.data(), std::min(bytes.size(), std::size_t{3}), MSG_NOSIGNAL);
    return {count, count < 0 ? errno : 0};
}
IoResult recv_some(int fd, std::span<Byte> bytes) {
    assert(!bytes.empty());
    const auto count = ::recv(fd, bytes.data(), bytes.size(), 0);
    return {count, count < 0 ? errno : 0};
}

void socket_test(const Wire& wire, const Frames& expected) {
    auto sockets = socket_pair();
    std::array<Byte, 5> buffer{};
    const auto empty = recv_some(sockets[1].get(), buffer);
    assert(empty.count == -1 && would_block(empty.error));
    Decoder decoder;
    Frames actual;
    std::size_t offset = 0;
    bool shutdown_sent = false, eof = false;
    // Nonblocking calls and a finite test budget prevent a stalled demo.
    for (unsigned turn = 0; turn < 1024 && !eof; ++turn) {
        const auto result = flush(wire.view(), offset, [&](auto bytes) {
            return send_some(sockets[0].get(), bytes);
        });
        if (result.state == WriteState::failed)
            throw std::system_error(result.error, std::generic_category(), "send");
        if (result.state == WriteState::complete && !shutdown_sent) {
            if (::shutdown(sockets[0].get(), SHUT_WR) < 0)
                throw std::system_error(errno, std::generic_category(), "shutdown");
            shutdown_sent = true;
        }
        const auto received = recv_some(sockets[1].get(), buffer);
        if (received.count > 0)
            feed_all(decoder, std::span(buffer).first(static_cast<std::size_t>(received.count)), actual);
        else if (received.count == 0) eof = true;
        else if (received.error != EINTR && !would_block(received.error))
            throw std::system_error(received.error, std::generic_category(), "recv");
    }
    if (!eof) throw std::runtime_error("socket test exhausted progress budget");
    assert(shutdown_sent && offset == wire.size && actual == expected);
    assert(decoder.finish() == Status::ended);

    // Receiving EOF in one direction does not disable the reverse direction.
    const std::array<Byte, 1> ack{0x7f};
    offset = 0;
    bool got_ack = false;
    for (unsigned turn = 0; turn < 1024 && !got_ack; ++turn) {
        const auto result = flush(ack, offset, [&](auto bytes) { return send_some(sockets[1].get(), bytes); });
        if (result.state == WriteState::failed)
            throw std::system_error(result.error, std::generic_category(), "reverse send");
        const auto received = recv_some(sockets[0].get(), buffer);
        if (received.count > 0) { assert(received.count == 1 && buffer[0] == ack[0]); got_ack = true; }
        else if (received.count == 0) throw std::runtime_error("unexpected reverse EOF");
        else if (received.error != EINTR && !would_block(received.error))
            throw std::system_error(received.error, std::generic_category(), "reverse recv");
    }
    if (!got_ack) throw std::runtime_error("reverse test exhausted progress budget");

    auto broken = socket_pair();
    broken[1].reset();
    const auto result = send_some(broken[0].get(), ack);
    assert(result.count == -1 && result.error == EPIPE); // MSG_NOSIGNAL keeps the process alive.
}

int main() {
    try {
        const auto wire = test_wire();
        const auto expected = parser_tests(wire);
        scripted_send_test();
        socket_test(wire, expected);
        std::cout << "framing checks passed; split points=32; frames=3; half-close=ok\n";
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        return 1;
    }
}
