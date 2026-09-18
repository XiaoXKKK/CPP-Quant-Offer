#include <sys/epoll.h>
#include <sys/socket.h>
#include <unistd.h>
#include <cerrno>
#include <cstdlib>
#include <iostream>
#include <stdexcept>
#include <string>

struct Fd {
    int value;
    explicit Fd(int fd) : value(fd) { if (fd < 0) throw std::runtime_error("fd creation failed"); }
    ~Fd() { if (value >= 0) ::close(value); }
    Fd(const Fd&) = delete;
    Fd& operator=(const Fd&) = delete;
};
void require(bool condition, const char* message) {
    if (!condition) throw std::runtime_error(message);
}
int wait_event(int epoll_fd, int timeout) {
    epoll_event event{};
    int count;
    do { count = ::epoll_wait(epoll_fd, &event, 1, timeout); } while (count < 0 && errno == EINTR);
    require(count >= 0, "epoll_wait failed");
    return count;
}
void demonstrate(bool edge) {
    int pair[2];
    require(::socketpair(AF_UNIX, SOCK_STREAM | SOCK_NONBLOCK, 0, pair) == 0, "socketpair failed");
    Fd reader(pair[0]), writer(pair[1]), ep(::epoll_create1(EPOLL_CLOEXEC));
    epoll_event event{};
    event.events = EPOLLIN | (edge ? static_cast<unsigned>(EPOLLET) : 0U);
    event.data.fd = reader.value;
    require(::epoll_ctl(ep.value, EPOLL_CTL_ADD, reader.value, &event) == 0, "epoll_ctl failed");
    require(::write(writer.value, "abcdef", 6) == 6, "write failed");
    require(wait_event(ep.value, 1000) == 1, "missing first event");
    char buffer[16];
    require(::read(reader.value, buffer, 3) == 3, "partial read failed");
    const int again = wait_event(ep.value, 0);
    require(again == (edge ? 0 : 1), "unexpected readiness in this controlled experiment");
    std::string remaining;
    for (;;) {
        const auto n = ::read(reader.value, buffer, sizeof(buffer));
        if (n > 0) { remaining.append(buffer, static_cast<std::size_t>(n)); continue; }
        if (n == 0) throw std::runtime_error("unexpected EOF before shutdown");
        if (errno == EINTR) continue;
        require(errno == EAGAIN || errno == EWOULDBLOCK, "read failed");
        break;
    }
    require(remaining == "def", "drain lost bytes");
    require(::shutdown(writer.value, SHUT_WR) == 0, "shutdown failed");
    require(wait_event(ep.value, 1000) == 1, "missing EOF readiness");
    require(::read(reader.value, buffer, sizeof(buffer)) == 0, "expected EOF");
    std::cout << (edge ? "ET" : "LT") << ": after partial read events=" << again
              << ", drained=" << remaining << ", EOF handled\n";
}
int main() {
    demonstrate(false);
    demonstrate(true);
}
