#include <array>
#include <cassert>
#include <cerrno>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <stdexcept>
#include <string>
#include <system_error>
#include <thread>

#include <linux/io_uring.h>
#include <sys/mman.h>
#include <sys/syscall.h>
#include <unistd.h>

#if defined(SYS_io_uring_setup) && defined(SYS_io_uring_enter)

struct RingResources {
    int fd = -1;
    struct Map { void* address = MAP_FAILED; std::size_t length = 0; };
    std::array<Map, 3> mappings{};
    int& cleanup_error;

    explicit RingResources(int& error) : cleanup_error(error) {}
    RingResources(const RingResources&) = delete;
    RingResources& operator=(const RingResources&) = delete;

    void* map(std::size_t index, std::size_t length, off_t offset) {
        auto& entry = mappings.at(index);
        entry.address = mmap(nullptr, length, PROT_READ | PROT_WRITE,
                             MAP_SHARED, fd, offset);
        if (entry.address == MAP_FAILED) {
            throw std::system_error(errno, std::generic_category(), "mmap ring");
        }
        entry.length = length;
        return entry.address;
    }

    ~RingResources() {
        for (auto& entry : mappings) {
            if (entry.address != MAP_FAILED &&
                munmap(entry.address, entry.length) == -1 && cleanup_error == 0) {
                cleanup_error = errno;
            }
        }
        // Only a NOP can be outstanding; no external buffer or application fd.
        if (fd != -1 && close(fd) == -1 && cleanup_error == 0) {
            cleanup_error = errno; // Do not retry close after EINTR on Linux.
        }
    }
};

std::size_t checked_length(std::size_t offset, std::size_t count,
                           std::size_t stride) {
    constexpr std::size_t max_mapping = 16U * 1024U * 1024U;
    if (offset > max_mapping || count > (max_mapping - offset) / stride) {
        throw std::runtime_error("ring mapping exceeds demo bounds");
    }
    return offset + count * stride;
}

template <class T>
T* ring_at(void* base, std::size_t length, std::size_t offset,
           std::size_t count = 1) {
    if (offset > length || count > (length - offset) / sizeof(T) ||
        offset % alignof(T) != 0) {
        throw std::runtime_error("invalid ring offset or alignment");
    }
    return reinterpret_cast<T*>(static_cast<unsigned char*>(base) + offset);
}

__u32 load_acquire(const __u32* value) {
    static_assert(__atomic_always_lock_free(sizeof(__u32), nullptr));
    return __atomic_load_n(value, __ATOMIC_ACQUIRE);
}

void store_release(__u32* target, __u32 value) {
    __atomic_store_n(target, value, __ATOMIC_RELEASE);
}

bool unavailable(int error) {
    return error == ENOSYS || error == EPERM || error == EACCES || error == EOPNOTSUPP;
}

void run_nop(RingResources& resources) {
    io_uring_params params{}; // No SQPOLL, IOPOLL, enlarged SQE/CQE, or deferred taskrun.
    const long setup = syscall(SYS_io_uring_setup, 2U, &params);
    if (setup == -1) {
        const int error = errno;
        if (unavailable(error)) {
            std::cout << "SKIP io_uring setup: errno=" << error << " ("
                      << std::error_code(error, std::generic_category()).message() << ")\n";
            return;
        }
        throw std::system_error(error, std::generic_category(), "io_uring_setup");
    }
    resources.fd = static_cast<int>(setup);
    if (params.sq_entries == 0 || params.cq_entries == 0 ||
        (params.sq_entries & (params.sq_entries - 1)) != 0 ||
        (params.cq_entries & (params.cq_entries - 1)) != 0) {
        throw std::runtime_error("unexpected ring capacities");
    }

    std::size_t sq_length = checked_length(params.sq_off.array, params.sq_entries,
                                           sizeof(__u32));
    const auto cq_length = checked_length(params.cq_off.cqes, params.cq_entries,
                                          sizeof(io_uring_cqe));
    const bool single_mapping = (params.features & IORING_FEAT_SINGLE_MMAP) != 0;
    if (single_mapping && sq_length < cq_length) {
        sq_length = cq_length;
    }
    void* sq = resources.map(0, sq_length, IORING_OFF_SQ_RING);
    void* cq = single_mapping ? sq : resources.map(1, cq_length, IORING_OFF_CQ_RING);
    const auto actual_cq_length = single_mapping ? sq_length : cq_length;
    const auto sqes_length = checked_length(0, params.sq_entries, sizeof(io_uring_sqe));
    void* sqes_mapping = resources.map(2, sqes_length, IORING_OFF_SQES);

    auto* sq_head = ring_at<__u32>(sq, sq_length, params.sq_off.head);
    auto* sq_tail = ring_at<__u32>(sq, sq_length, params.sq_off.tail);
    auto* sq_mask = ring_at<__u32>(sq, sq_length, params.sq_off.ring_mask);
    auto* sq_array = ring_at<__u32>(sq, sq_length, params.sq_off.array, params.sq_entries);
    auto* cq_head = ring_at<__u32>(cq, actual_cq_length, params.cq_off.head);
    auto* cq_tail = ring_at<__u32>(cq, actual_cq_length, params.cq_off.tail);
    auto* cq_mask = ring_at<__u32>(cq, actual_cq_length, params.cq_off.ring_mask);
    auto* cq_overflow = ring_at<__u32>(cq, actual_cq_length, params.cq_off.overflow);
    auto* cqes = ring_at<io_uring_cqe>(cq, actual_cq_length, params.cq_off.cqes,
                                      params.cq_entries);
    auto* sqes = ring_at<io_uring_sqe>(sqes_mapping, sqes_length, 0, params.sq_entries);
    if (*sq_mask != params.sq_entries - 1 || *cq_mask != params.cq_entries - 1) {
        throw std::runtime_error("unexpected ring masks");
    }
    const __u32 tail = load_acquire(sq_tail);
    const __u32 head = load_acquire(cq_head);
    if (load_acquire(sq_head) != tail || load_acquire(cq_tail) != head) {
        throw std::runtime_error("new ring is not empty");
    }
    std::cout << "sq_entries=" << params.sq_entries << " cq_entries=" << params.cq_entries
              << " single_mmap=" << single_mapping
              << " nodrop=" << ((params.features & IORING_FEAT_NODROP) != 0) << '\n';

    constexpr __u64 token = 0x4e4f5001ULL;
    sqes[0] = {};
    sqes[0].opcode = IORING_OP_NOP;
    sqes[0].user_data = token;
    sq_array[tail & *sq_mask] = 0;
    store_release(sq_tail, tail + 1U); // Publishes both SQE and SQ-array entry.

    const auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(2);
    for (std::size_t attempt = 0; attempt < 100000; ++attempt) {
        const __u32 completed_tail = load_acquire(cq_tail);
        if (completed_tail != head) {
            if (completed_tail - head != 1U) {
                throw std::runtime_error("unexpected number of completions");
            }
            const io_uring_cqe completion = cqes[head & *cq_mask];
            store_release(cq_head, head + 1U); // Local copy survives CQ-slot reuse.
            if (completion.user_data != token || completion.flags != 0) {
                throw std::runtime_error("unexpected completion identity or flags");
            }
            if (completion.res < 0) {
                throw std::runtime_error("NOP CQE res=" + std::to_string(completion.res));
            }
            assert(completion.res == 0);
            assert(load_acquire(cq_overflow) == 0);
            std::cout << "NOP completed: user_data matched, res=0, cq_overflow=0\n";
            return;
        }
        if (std::chrono::steady_clock::now() >= deadline) {
            break;
        }
        const __u32 pending = (tail + 1U) - load_acquire(sq_head);
        if (pending > 1U) {
            throw std::runtime_error("unexpected SQ consumption");
        }
        // min_complete=0 and flags=0: never request a blocking GETEVENTS wait.
        const long submitted = syscall(SYS_io_uring_enter, resources.fd, pending, 0U,
                                       0U, nullptr, 0U);
        if (submitted == -1 && errno != EINTR && errno != EAGAIN) {
            throw std::system_error(errno, std::generic_category(), "io_uring_enter");
        }
        if (submitted > 1) {
            throw std::runtime_error("unexpected submission count");
        }
        std::this_thread::yield();
    }
    throw std::runtime_error("NOP completion watchdog expired");
}

int main() {
    int cleanup_error = 0;
    try {
        {
            RingResources resources(cleanup_error);
            run_nop(resources);
        }
        if (cleanup_error != 0) {
            throw std::system_error(cleanup_error, std::generic_category(), "ring cleanup");
        }
        std::cout << "ring resource cleanup passed\n";
    } catch (const std::exception& error) {
        std::cerr << "demo failed: " << error.what() << '\n';
        if (cleanup_error != 0) {
            std::cerr << "cleanup also failed: errno=" << cleanup_error << '\n';
        }
        return 1;
    }
}

#else
int main() {
    std::cout << "SKIP io_uring: syscall numbers unavailable in build headers\n";
}
#endif
