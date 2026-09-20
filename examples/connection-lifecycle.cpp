#include <algorithm>
#include <cassert>
#include <cerrno>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <limits>
#include <optional>
#include <stdexcept>
#include <system_error>
#include <utility>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <poll.h>
#include <sys/socket.h>
#include <unistd.h>

namespace model {
using Tick = std::uint64_t;
enum class Phase { connecting, open, draining, write_closed, closed };
enum class Reason { none, graceful, connect_error, io_error, timeout };
enum class Kind { connect_result, enqueue, sent, peer_eof, begin_drain, tick, io_error };
struct Event { Kind kind; std::uint64_t value = 0; int error = 0; };
struct Handle { std::size_t slot; std::uint64_t generation; };
struct Snapshot {
    Phase phase;
    std::size_t pending;
    bool read_eof;
};

// These actions are recorded, not syscalls. Fake fd numbers are never used by Linux.
struct Actions {
    unsigned closes = 0;
    unsigned shutdowns = 0;
    int last_closed = -1;
    bool fail_shutdown = false;
    void close(int fd) noexcept { ++closes; last_closed = fd; }
    bool shutdown_write(int) noexcept { ++shutdowns; return !fail_shutdown; }
};

class Connection {
    Actions& actions_;
    int fd_;
    Phase phase_ = Phase::connecting;
    std::size_t pending_ = 0;
    bool read_eof_ = false;
    Tick deadline_;
    Reason reason_ = Reason::none;

    void close_once(Reason reason) noexcept {
        if (phase_ == Phase::closed) return;
        phase_ = Phase::closed;
        reason_ = reason;
        actions_.close(std::exchange(fd_, -1));
    }
    void advance_close() noexcept {
        if (phase_ == Phase::draining && pending_ == 0) {
            if (!actions_.shutdown_write(fd_)) { close_once(Reason::io_error); return; }
            phase_ = Phase::write_closed;
        }
        if (phase_ == Phase::write_closed && read_eof_) close_once(Reason::graceful);
    }
public:
    Connection(Actions& actions, int fd, Tick deadline) noexcept
        : actions_(actions), fd_(fd), deadline_(deadline) {}
    Connection(const Connection&) = delete;
    Connection& operator=(const Connection&) = delete;
    ~Connection() { if (fd_ >= 0) actions_.close(std::exchange(fd_, -1)); }

    bool apply(Event event) noexcept {
        switch (event.kind) {
        case Kind::connect_result:
            if (phase_ != Phase::connecting) return false;
            if (event.error != 0) close_once(Reason::connect_error);
            else phase_ = Phase::open;
            return true;
        case Kind::enqueue:
            if (phase_ != Phase::open || event.value > 32 - pending_) return false;
            pending_ += static_cast<std::size_t>(event.value);
            return true;
        case Kind::sent:
            if ((phase_ != Phase::open && phase_ != Phase::draining) || event.value > pending_)
                return false;
            pending_ -= static_cast<std::size_t>(event.value);
            advance_close();
            return true;
        case Kind::peer_eof:
            if (phase_ == Phase::connecting || phase_ == Phase::closed) return false;
            read_eof_ = true;
            advance_close();
            return true;
        case Kind::begin_drain:
            if (phase_ == Phase::open) { phase_ = Phase::draining; deadline_ = event.value; }
            else if (phase_ == Phase::draining || phase_ == Phase::write_closed)
                deadline_ = std::min(deadline_, event.value); // A repeated request never extends it.
            else return false;
            advance_close();
            return true;
        case Kind::tick:
            if ((phase_ == Phase::connecting || phase_ == Phase::draining ||
                 phase_ == Phase::write_closed) && event.value >= deadline_)
                close_once(Reason::timeout);
            return true;
        case Kind::io_error:
            close_once(Reason::io_error);
            return true;
        }
        return false;
    }
    Snapshot snapshot() const noexcept { return {phase_, pending_, read_eof_}; }
    Reason reason() const noexcept { return reason_; }
};

// One-slot registry models a stable index + generation handle. Single-thread owner.
// dispatch has no user callback and never exports a borrowed Connection pointer.
class Owner {
    Actions& actions_;
    std::optional<Connection> connection_;
    std::uint64_t generation_;
    bool retired_ = false;
    Reason last_reason_ = Reason::none;
    std::size_t last_discarded_ = 0;
    bool matches(Handle handle) const noexcept {
        return handle.slot == 0 && handle.generation == generation_ && connection_.has_value();
    }
public:
    explicit Owner(Actions& actions, std::uint64_t generation = 1) noexcept
        : actions_(actions), generation_(generation) {}
    std::optional<Handle> create(int modeled_fd, Tick connect_deadline) {
        if (retired_ || connection_ || modeled_fd < 0) return std::nullopt;
        connection_.emplace(actions_, modeled_fd, connect_deadline);
        return Handle{0, generation_};
    }
    bool dispatch(Handle handle, Event event) {
        if (!matches(handle)) return false;
        const bool accepted = connection_->apply(event);
        if (connection_->snapshot().phase == Phase::closed) {
            last_reason_ = connection_->reason();
            last_discarded_ = connection_->snapshot().pending;
            connection_.reset(); // Reclaim only after apply returns.
            if (generation_ == std::numeric_limits<std::uint64_t>::max()) retired_ = true;
            else ++generation_;
        }
        return accepted;
    }
    std::optional<Snapshot> inspect(Handle handle) const noexcept {
        if (!matches(handle)) return std::nullopt;
        return connection_->snapshot();
    }
    Reason last_reason() const noexcept { return last_reason_; }
    std::size_t last_discarded() const noexcept { return last_discarded_; }
};

void tests() {
    Actions actions;
    Owner owner(actions);
    const auto old = *owner.create(42, 100);
    assert(!owner.dispatch(old, {Kind::enqueue, 1})); // Connecting cannot accept application output.
    assert(owner.dispatch(old, {Kind::connect_result})); // A checked SO_ERROR result of zero.
    assert(!owner.dispatch(old, {Kind::enqueue, 33}));
    assert(owner.dispatch(old, {Kind::enqueue, 8}));
    assert(owner.dispatch(old, {Kind::peer_eof}));
    assert(owner.inspect(old)->phase == Phase::open); // EOF does not close the send direction.
    assert(owner.dispatch(old, {Kind::enqueue, 2})); // This protocol allows a final response.
    assert(owner.dispatch(old, {Kind::begin_drain, 50}));
    assert(!owner.dispatch(old, {Kind::enqueue, 1}));
    assert(owner.dispatch(old, {Kind::sent, 4}));
    assert(owner.inspect(old)->pending == 6 && actions.shutdowns == 0);
    assert(!owner.dispatch(old, {Kind::sent, 7}));
    assert(owner.dispatch(old, {Kind::sent, 6}));
    assert(!owner.inspect(old) && owner.last_reason() == Reason::graceful);
    assert(actions.closes == 1 && actions.shutdowns == 1);
    assert(!owner.dispatch(old, {Kind::io_error}) && actions.closes == 1);

    const auto fresh = *owner.create(42, 100); // Deterministically reuse the modeled fd number.
    assert(fresh.generation != old.generation);
    assert(!owner.dispatch(old, {Kind::enqueue, 8})); // Delayed event or worker result is rejected.
    assert(!owner.dispatch({1, fresh.generation}, {Kind::io_error}));
    assert(owner.dispatch(fresh, {Kind::connect_result}));
    assert(owner.dispatch(fresh, {Kind::begin_drain, 50}));
    assert(owner.inspect(fresh)->phase == Phase::write_closed && actions.shutdowns == 2);
    assert(owner.dispatch(fresh, {Kind::begin_drain, 500}));
    assert(actions.shutdowns == 2);
    assert(owner.dispatch(fresh, {Kind::tick, 49}) && owner.inspect(fresh));
    assert(owner.dispatch(fresh, {Kind::tick, 50}));
    assert(owner.last_reason() == Reason::timeout && actions.closes == 2);

    const auto rejected = *owner.create(42, 100);
    assert(owner.dispatch(rejected, {Kind::connect_result, 0, ECONNREFUSED}));
    assert(owner.last_reason() == Reason::connect_error && actions.closes == 3);
    const auto connecting = *owner.create(42, 100);
    assert(owner.dispatch(connecting, {Kind::tick, 100}));
    assert(owner.last_reason() == Reason::timeout && actions.closes == 4);

    const auto stalled = *owner.create(42, 100);
    assert(owner.dispatch(stalled, {Kind::connect_result}));
    assert(owner.dispatch(stalled, {Kind::enqueue, 10}));
    assert(owner.dispatch(stalled, {Kind::sent, 3}));
    assert(owner.dispatch(stalled, {Kind::begin_drain, 20}));
    assert(owner.dispatch(stalled, {Kind::tick, 20}));
    assert(owner.last_discarded() == 7 && actions.closes == 5 && actions.shutdowns == 2);

    actions.fail_shutdown = true;
    const auto failure = *owner.create(42, 100);
    assert(owner.dispatch(failure, {Kind::connect_result}));
    assert(owner.dispatch(failure, {Kind::begin_drain, 20}));
    assert(owner.last_reason() == Reason::io_error && actions.closes == 6);
    actions.fail_shutdown = false;

    Owner exhausted(actions, std::numeric_limits<std::uint64_t>::max());
    const auto last = *exhausted.create(42, 100);
    assert(exhausted.dispatch(last, {Kind::io_error}));
    assert(!exhausted.create(42, 100)); // Retire rather than wrap to an old generation.
    assert(!exhausted.dispatch(last, {Kind::tick, 100}) && actions.closes == 7);

    Actions reverse_actions;
    Owner reverse(reverse_actions);
    const auto local_first = *reverse.create(9, 100);
    assert(reverse.dispatch(local_first, {Kind::connect_result}));
    assert(reverse.dispatch(local_first, {Kind::begin_drain, 50}));
    assert(reverse.inspect(local_first)->phase == Phase::write_closed);
    assert(reverse.dispatch(local_first, {Kind::peer_eof}));
    assert(reverse.last_reason() == Reason::graceful);
    assert(reverse_actions.shutdowns == 1 && reverse_actions.closes == 1);
}
} // namespace model

class Fd {
    int fd_;
public:
    explicit Fd(int fd) : fd_(fd) {
        if (fd < 0) throw std::system_error(errno, std::generic_category(), "socket");
    }
    Fd(const Fd&) = delete;
    Fd& operator=(const Fd&) = delete;
    ~Fd() { if (fd_ >= 0) static_cast<void>(::close(std::exchange(fd_, -1))); }
    int get() const noexcept { return fd_; }
};

void checked(int result, const char* operation) {
    if (result < 0) throw std::system_error(errno, std::generic_category(), operation);
}

// Real Linux syscalls: one nonblocking TCP connection to this process's loopback listener.
// No application messages, generation reuse, graceful drain, or failure injection here.
void loopback_connect_test() {
    Fd listener(::socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, 0));
    sockaddr_in address{};
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    address.sin_port = 0;
    checked(::bind(listener.get(), reinterpret_cast<const sockaddr*>(&address), sizeof(address)), "bind");
    checked(::listen(listener.get(), 1), "listen");
    socklen_t length = sizeof(address);
    checked(::getsockname(listener.get(), reinterpret_cast<sockaddr*>(&address), &length), "getsockname");
    Fd client(::socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, 0));
    const int result = ::connect(client.get(), reinterpret_cast<const sockaddr*>(&address), sizeof(address));
    if (result < 0) {
        if (errno != EINPROGRESS) throw std::system_error(errno, std::generic_category(), "connect");
        const auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(2);
        bool completion_hint = false;
        while (std::chrono::steady_clock::now() < deadline) {
            const auto remaining = std::chrono::ceil<std::chrono::milliseconds>(deadline - std::chrono::steady_clock::now());
            if (remaining.count() <= 0) break;
            pollfd event{client.get(), POLLOUT, 0};
            const int ready = ::poll(&event, 1, static_cast<int>(remaining.count()));
            if (ready < 0) {
                if (errno == EINTR) continue;
                throw std::system_error(errno, std::generic_category(), "poll");
            }
            if (event.revents & POLLNVAL) throw std::runtime_error("invalid client fd");
            if (ready > 0 && (event.revents & (POLLOUT | POLLERR | POLLHUP))) {
                completion_hint = true;
                break;
            }
        }
        if (!completion_hint) throw std::runtime_error("loopback connect deadline expired");
    }
    int error = -1;
    length = sizeof(error);
    checked(::getsockopt(client.get(), SOL_SOCKET, SO_ERROR, &error, &length), "getsockopt SO_ERROR");
    if (error != 0) throw std::system_error(error, std::generic_category(), "connect completion");
    assert(length == sizeof(error));
}

int main() {
    try {
        model::tests();
        loopback_connect_test();
        std::cout << "lifecycle checks passed; stale handles rejected; generation retired; SO_ERROR=0\n";
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        return 1;
    }
}
