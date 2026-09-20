#include <array>
#include <cerrno>
#include <charconv>
#include <cstdint>
#include <iostream>
#include <limits>
#include <optional>
#include <stdexcept>
#include <string_view>
#include <system_error>

#include <linux/perf_event.h>
#include <sys/ioctl.h>
#include <sys/syscall.h>
#include <unistd.h>

namespace {
constexpr std::size_t input_size = 4096;
constexpr unsigned maximum_rounds = 2048;
using Input = std::array<std::uint32_t, input_size>;
constexpr std::array<std::uint64_t, 4> expected_pass{
    17396735839208ULL, 17684914805289ULL,
    17535516519958ULL, 17574288582265ULL};

void require(bool condition, const char* message) {
    if (!condition) {
        throw std::runtime_error(message);
    }
}

unsigned parse_rounds(std::string_view value) {
    unsigned rounds = 0;
    const auto result = std::from_chars(value.data(), value.data() + value.size(), rounds);
    if (result.ec != std::errc{} || result.ptr != value.data() + value.size() ||
        rounds == 0 || rounds > maximum_rounds) {
        throw std::invalid_argument("rounds must be an integer in [1, 2048]");
    }
    return rounds;
}

Input make_input() {
    Input input{};
    for (std::size_t i = 0; i < input.size(); ++i) {
        input[i] = (static_cast<std::uint32_t>(i) * 2654435761U) ^ 0xa5a5a5a5U;
    }
    return input;
}

// GCC/Clang extension: retain useful function boundaries for the example.
// This does not promise complete stacks in a profile of arbitrary libraries.
[[gnu::noinline]] std::uint64_t arithmetic_phase(const Input& input, unsigned salt) {
    std::uint64_t sum = 0;
    for (const auto value : input) {
        std::uint32_t x = value ^ salt;
        for (unsigned step = 0; step < 24; ++step) {
            x = (x ^ (x >> 13)) * 1664525U + 1013904223U;
        }
        sum += x;
    }
    return sum;
}

[[gnu::noinline]] std::uint64_t gather_phase(const Input& input, unsigned salt) {
    std::uint64_t sum = 0;
    for (std::size_t i = 0; i < input.size(); ++i) {
        const auto index = (i * 73U) & (input_size - 1U);
        sum += input[index] ^ (salt + static_cast<std::uint32_t>(i));
    }
    return sum;
}

[[gnu::noinline]] std::uint64_t run_workload(const Input& input, unsigned rounds) {
    std::uint64_t checksum = 0;
    for (unsigned round = 0; round < rounds; ++round) {
        const unsigned salt = round % 4U;
        const auto value = arithmetic_phase(input, salt) + gather_phase(input, salt);
        require(value == expected_pass[salt], "workload checksum mismatch");
        checksum += value;
    }
    return checksum;
}

struct CounterRead {
    std::uint64_t value;
    std::uint64_t time_enabled;
    std::uint64_t time_running;
};
static_assert(sizeof(CounterRead) == 3 * sizeof(std::uint64_t));

std::optional<long double> scaled_count(const CounterRead& count) {
    require(count.time_running <= count.time_enabled, "invalid counter time ordering");
    if (count.time_running == 0) {
        return std::nullopt;
    }
    // Convert before multiplying: the integer product could overflow.
    return static_cast<long double>(count.value) *
           static_cast<long double>(count.time_enabled) /
           static_cast<long double>(count.time_running);
}

bool unavailable_configuration(int error) {
    // EINVAL can also indicate bad parameters; it does not prove missing hardware.
    return error == EACCES || error == EPERM || error == ENOSYS ||
           error == ENOENT || error == ENODEV || error == EOPNOTSUPP || error == EINVAL;
}

class TaskClock {
public:
    TaskClock() {
        perf_event_attr attr{};
        attr.type = PERF_TYPE_SOFTWARE;
        attr.size = sizeof(attr);
        attr.config = PERF_COUNT_SW_TASK_CLOCK;
        attr.disabled = 1;
        // Request privilege filters, but software task-clock implementations do
        // not necessarily split their accumulated clock by privilege level.
        attr.exclude_kernel = 1;
        attr.exclude_hv = 1;
        attr.read_format = PERF_FORMAT_TOTAL_TIME_ENABLED | PERF_FORMAT_TOTAL_TIME_RUNNING;
        const long result = ::syscall(SYS_perf_event_open, &attr, 0, -1, -1,
                                      PERF_FLAG_FD_CLOEXEC);
        if (result < 0) {
            const int error = errno;
            if (unavailable_configuration(error)) {
                std::cout << "SKIP task-clock configuration: perf_event_open errno=" << error << " ("
                          << std::generic_category().message(error) << ")\n";
                return;
            }
            throw std::system_error(error, std::generic_category(), "perf_event_open");
        }
        // A successful Linux fd is an int, despite syscall's long return type.
        fd_ = static_cast<int>(result);
    }

    TaskClock(const TaskClock&) = delete;
    TaskClock& operator=(const TaskClock&) = delete;
    ~TaskClock() {
        if (fd_ >= 0) {
            ::close(fd_); // Linux: do not retry close on EINTR.
        }
    }

    void start() const {
        if (fd_ >= 0) {
            control(PERF_EVENT_IOC_RESET);
            control(PERF_EVENT_IOC_ENABLE);
        }
    }

    void finish() const {
        if (fd_ < 0) {
            return;
        }
        control(PERF_EVENT_IOC_DISABLE);
        CounterRead count{};
        const auto bytes = ::read(fd_, &count, sizeof(count));
        if (bytes < 0) {
            throw std::system_error(errno, std::generic_category(), "read counter");
        }
        require(bytes == static_cast<ssize_t>(sizeof(count)), "short counter read");
        const auto scaled = scaled_count(count);
        std::cout << "task-clock raw=" << count.value
                  << " enabled_ns=" << count.time_enabled
                  << " running_ns=" << count.time_running;
        if (scaled) {
            std::cout << " scaled_estimate_ns=" << *scaled << '\n';
        } else {
            std::cout << " SKIP scaling: counter never ran\n";
        }
    }

private:
    void control(unsigned long request) const {
        if (::ioctl(fd_, request, 0) < 0) {
            throw std::system_error(errno, std::generic_category(), "counter ioctl");
        }
    }
    int fd_ = -1;
};

void test_helpers() {
    require(parse_rounds("1") == 1 && parse_rounds("2048") == maximum_rounds,
            "round parsing bounds");
    for (const auto bad : {"", "0", "2049", "-1", "1x", " 1", "999999999999999999999"}) {
        bool rejected = false;
        try {
            static_cast<void>(parse_rounds(bad));
        } catch (const std::invalid_argument&) {
            rejected = true;
        }
        require(rejected, "invalid rounds accepted");
    }
    require(!scaled_count({0, 100, 0}), "zero running time must not divide");
    require(scaled_count({10, 100, 50}) == 20.0L, "scaling formula");
    require(scaled_count({10, 50, 50}) == 10.0L, "unscaled counter");
    require(scaled_count({std::numeric_limits<std::uint64_t>::max(), 2, 1}).value() >
                static_cast<long double>(std::numeric_limits<std::uint64_t>::max()),
            "scale before integer overflow");
    bool rejected = false;
    try {
        static_cast<void>(scaled_count({1, 1, 2}));
    } catch (const std::runtime_error&) {
        rejected = true;
    }
    require(rejected, "invalid counter times accepted");
}
} // namespace

int main(int argc, char** argv) {
    try {
        require(argc <= 2, "usage: perf-flamegraph [rounds: 1..2048]");
        const unsigned rounds = argc == 2 ? parse_rounds(argv[1]) : 32U;
        test_helpers();
        const Input input = make_input();
        // Warm and verify all four salts before the measured interval.
        static_cast<void>(run_workload(input, 4));
        TaskClock counter;
        counter.start();
        const auto checksum = run_workload(input, rounds);
        counter.finish();
        std::cout << "rounds=" << rounds << " checksum=" << checksum
                  << " correctness=OK\n";
    } catch (const std::exception& error) {
        std::cerr << "ERROR: " << error.what() << '\n';
        return 1;
    }
}
