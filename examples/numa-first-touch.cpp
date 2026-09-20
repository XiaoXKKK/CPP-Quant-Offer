#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <array>
#include <cassert>
#include <cerrno>
#include <cstddef>
#include <exception>
#include <iostream>
#include <memory>
#include <new>
#include <stdexcept>
#include <system_error>
#include <thread>

#include <sched.h>
#include <sys/mman.h>
#include <sys/syscall.h>
#include <unistd.h>

struct CpuSetDeleter {
    void operator()(cpu_set_t* set) const noexcept { CPU_FREE(set); }
};

struct CpuMask {
    std::size_t capacity;
    std::size_t bytes;
    std::unique_ptr<cpu_set_t, CpuSetDeleter> set;

    explicit CpuMask(std::size_t count)
        : capacity(count), bytes(CPU_ALLOC_SIZE(count)), set(CPU_ALLOC(count)) {
        if (!set) {
            throw std::bad_alloc{};
        }
        CPU_ZERO_S(bytes, set.get());
    }
};

CpuMask read_affinity() {
    // A bounded probe also handles kernel CPU masks larger than CPU_SETSIZE.
    for (std::size_t count = 128; count <= (1U << 20); count *= 2) {
        CpuMask mask(count);
        if (sched_getaffinity(0, mask.bytes, mask.set.get()) == 0) {
            return mask;
        }
        const int error = errno;
        if (error != EINVAL) {
            throw std::system_error(error, std::generic_category(), "get affinity");
        }
    }
    throw std::runtime_error("CPU affinity mask exceeds demo limit");
}

class ScopedAffinity {
    const CpuMask& saved_;
    CpuMask verification_;
    int& restore_error_;

public:
    ScopedAffinity(const CpuMask& saved, const CpuMask& target, int& error)
        : saved_(saved), verification_(saved.capacity), restore_error_(error) {
        if (sched_setaffinity(0, target.bytes, target.set.get()) == -1) {
            throw std::system_error(errno, std::generic_category(), "bind worker");
        }
    }

    ScopedAffinity(const ScopedAffinity&) = delete;
    ScopedAffinity& operator=(const ScopedAffinity&) = delete;

    ~ScopedAffinity() {
        if (sched_setaffinity(0, saved_.bytes, saved_.set.get()) == -1 ||
            sched_getaffinity(0, verification_.bytes, verification_.set.get()) == -1) {
            restore_error_ = errno;
        } else if (!CPU_EQUAL_S(saved_.bytes, saved_.set.get(),
                                verification_.set.get())) {
            // An external cpuset/CPU change can prevent an exact restoration.
            restore_error_ = EAGAIN;
        }
    }
};

class Mapping {
    void* address_;
    std::size_t length_;
    int& cleanup_error_;

public:
    Mapping(std::size_t length, int& error)
        : address_(mmap(nullptr, length, PROT_READ | PROT_WRITE,
                        MAP_PRIVATE | MAP_ANONYMOUS, -1, 0)),
          length_(length), cleanup_error_(error) {
        if (address_ == MAP_FAILED) {
            throw std::system_error(errno, std::generic_category(), "mmap");
        }
    }

    Mapping(const Mapping&) = delete;
    Mapping& operator=(const Mapping&) = delete;
    unsigned char* data() const noexcept {
        return static_cast<unsigned char*>(address_);
    }

    ~Mapping() {
        if (munmap(address_, length_) == -1) {
            cleanup_error_ = errno;
        }
    }
};

constexpr std::size_t sample_count = 32;

void query_nodes(unsigned char* data, std::size_t page_size) {
#ifdef SYS_move_pages
    std::array<void*, sample_count> pages{};
    std::array<int, sample_count> status{};
    for (std::size_t i = 0; i < sample_count; ++i) {
        pages[i] = data + i * page_size;
    }
    // nodes == nullptr requests status only. No page migration or policy change.
    const long result = syscall(SYS_move_pages, 0, static_cast<unsigned long>(sample_count),
                                pages.data(), static_cast<const int*>(nullptr),
                                status.data(), 0);
    if (result == -1) {
        const int error = errno;
        if (error != EPERM && error != EACCES && error != ENOSYS &&
            error != EOPNOTSUPP) {
            throw std::system_error(error, std::generic_category(), "query page nodes");
        }
        std::cout << "SKIP page-node query: errno=" << error << " ("
                  << std::error_code(error, std::generic_category()).message() << ")\n";
        return; // status entries are not evidence after a syscall failure.
    }
    std::cout << "page-node samples:";
    for (const int node : status) {
        if (node >= 0) {
            std::cout << ' ' << node;
        } else {
            std::cout << " error(" << node << ')';
        }
    }
    std::cout << '\n';
#else
    static_cast<void>(data);
    static_cast<void>(page_size);
    std::cout << "SKIP page-node query: SYS_move_pages is unavailable\n";
#endif
}

int main() {
    int cleanup_error = 0;
    try {
        const long page_size_result = sysconf(_SC_PAGESIZE);
        if (page_size_result <= 0) {
            throw std::runtime_error("could not determine page size");
        }
        const auto page_size = static_cast<std::size_t>(page_size_result);
        constexpr std::size_t max_bytes = 32U * 1024U * 1024U;
        if (page_size > max_bytes / sample_count) {
            throw std::runtime_error("mapping would exceed demo memory limit");
        }
        const auto length = page_size * sample_count;
        int restore_error = 0;
        std::exception_ptr worker_error;
        {
            Mapping memory(length, cleanup_error);
            if (madvise(memory.data(), length, MADV_NOHUGEPAGE) == 0) {
                std::cout << "MADV_NOHUGEPAGE applied to demo mapping\n";
            } else {
                const int error = errno;
                std::cout << "SKIP MADV_NOHUGEPAGE: errno=" << error << '\n';
            }
            std::cout << "page_size=" << page_size << " bytes=" << length
                      << " samples=" << sample_count << '\n';

            std::jthread worker([&] {
                try {
                    auto saved = read_affinity();
                    CpuMask target(saved.capacity);
                    std::size_t selected = saved.capacity;
                    for (std::size_t cpu = 0; cpu < saved.capacity; ++cpu) {
                        if (CPU_ISSET_S(cpu, saved.bytes, saved.set.get())) {
                            selected = cpu;
                            break;
                        }
                    }
                    if (selected == saved.capacity) {
                        throw std::runtime_error("no allowed CPU found");
                    }
                    CPU_SET_S(selected, target.bytes, target.set.get());
                    ScopedAffinity affinity(saved, target, restore_error);
                    const auto actual = read_affinity();
                    if (actual.bytes != target.bytes ||
                        !CPU_EQUAL_S(target.bytes, actual.set.get(), target.set.get())) {
                        throw std::runtime_error("worker affinity differs from request");
                    }
                    const int running_cpu = sched_getcpu();
                    if (running_cpu < 0 || static_cast<std::size_t>(running_cpu) != selected) {
                        throw std::runtime_error("worker CPU changed during binding check");
                    }
                    std::cout << "allowed_cpu_count="
                              << CPU_COUNT_S(saved.bytes, saved.set.get())
                              << " selected_cpu=" << selected << '\n';

                    // No earlier access to the mapping. Volatile preserves these stores;
                    // join, not volatile, synchronizes the later main-thread reads.
                    volatile unsigned char* bytes = memory.data();
                    for (std::size_t i = 0; i < sample_count; ++i) {
                        bytes[i * page_size] = static_cast<unsigned char>(i + 1);
                    }
                    query_nodes(memory.data(), page_size);
                } catch (...) {
                    worker_error = std::current_exception();
                }
            });
            worker.join();
            if (restore_error != 0) {
                throw std::system_error(restore_error, std::generic_category(),
                                        "restore worker affinity");
            }
            if (worker_error) {
                std::rethrow_exception(worker_error);
            }
            for (std::size_t i = 0; i < sample_count; ++i) {
                assert(memory.data()[i * page_size] == static_cast<unsigned char>(i + 1));
            }
            std::cout << "write/read checks passed; worker affinity restored\n";
        }
        if (cleanup_error != 0) {
            throw std::system_error(cleanup_error, std::generic_category(), "munmap");
        }
        std::cout << "mapping cleanup passed; no placement or speed guarantee asserted\n";
    } catch (const std::exception& error) {
        std::cerr << "demo failed: " << error.what() << '\n';
        if (cleanup_error != 0) {
            std::cerr << "munmap also failed: errno=" << cleanup_error << '\n';
        }
        return 1;
    }
}
