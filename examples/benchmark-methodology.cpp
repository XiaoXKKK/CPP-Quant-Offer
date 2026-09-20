#include <algorithm>
#include <array>
#include <cassert>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <iomanip>
#include <iostream>
#include <limits>
#include <span>
#include <stdexcept>
#include <vector>

#if defined(__GNUC__) || defined(__clang__)
#define DEMO_NOINLINE __attribute__((noinline))
#else
#define DEMO_NOINLINE
#endif

using Input = std::span<const std::uint32_t>;
using Kernel = std::uint64_t (*)(Input);
using Clock = std::chrono::steady_clock;

DEMO_NOINLINE std::uint64_t sum_a(Input input) {
    std::uint64_t total = 0;
    for (const auto value : input) {
        total += value;
    }
    return total;
}

DEMO_NOINLINE std::uint64_t sum_b(Input input) {
    std::uint64_t s0 = 0, s1 = 0, s2 = 0, s3 = 0;
    std::size_t i = 0;
    for (; input.size() - i >= 4; i += 4) {
        s0 += input[i];
        s1 += input[i + 1];
        s2 += input[i + 2];
        s3 += input[i + 3];
    }
    for (; i < input.size(); ++i) {
        s0 += input[i];
    }
    return s0 + s1 + s2 + s3;
}

void verify_boundaries() {
    constexpr std::array<std::uint32_t, 5> values{
        0, 1, std::numeric_limits<std::uint32_t>::max(), 7, 9};
    constexpr std::array<std::uint64_t, 6> expected{
        0, 0, 1, 4294967296ULL, 4294967303ULL, 4294967312ULL};
    for (std::size_t count = 0; count <= values.size(); ++count) {
        const Input input(values.data(), count);
        if (sum_a(input) != expected[count] || sum_b(input) != expected[count]) {
            throw std::runtime_error("boundary correctness failure");
        }
    }
}

// Nearest rank: sorted[ceil(n * numerator / denominator) - 1].
// This helper deliberately accepts only the small, bounded sample sets below.
double nearest_rank(std::vector<double> values, std::size_t numerator,
                    std::size_t denominator) {
    if (values.empty() || values.size() > 1000 || denominator == 0 ||
        denominator > 1000 || numerator == 0 || numerator > denominator) {
        throw std::runtime_error("invalid bounded quantile request");
    }
    std::sort(values.begin(), values.end());
    const auto rank = (values.size() * numerator + denominator - 1) / denominator;
    return values[rank - 1];
}

#if defined(__GNUC__) || defined(__clang__)
void compiler_input_barrier(const std::uint32_t* input) {
    // Compiler memory barrier only: no cache flush, CPU fence, or thread sync.
    asm volatile("" : : "r"(input) : "memory");
}

struct Sample {
    double total_ns;
    std::uint64_t checksum;
};

DEMO_NOINLINE Sample measure(Kernel kernel, Input input, std::size_t passes) {
    std::uint64_t checksum = 0;
    const auto start = Clock::now();
    for (std::size_t pass = 0; pass < passes; ++pass) {
        compiler_input_barrier(input.data());
        checksum += kernel(input);
    }
    // Keep the accumulated result available before the ending timestamp.
    asm volatile("" : "+r"(checksum) : : "memory");
    const auto stop = Clock::now();
    return {std::chrono::duration<double, std::nano>(stop - start).count(), checksum};
}
#endif

int main() {
    try {
        static_assert(Clock::is_steady);
        verify_boundaries();
        assert(nearest_rank({4, 1, 3, 2}, 50, 100) == 2);
        assert(nearest_rank({4, 1, 3, 2}, 99, 100) == 4);

        constexpr std::size_t count = 65536;
        constexpr std::size_t passes = 32;
        constexpr std::size_t warmup_rounds = 4;
        constexpr std::size_t rounds = 20;
        std::vector<std::uint32_t> data(count);
        for (std::size_t i = 0; i < data.size(); ++i) {
            data[i] = static_cast<std::uint32_t>((i * 17U + 11U) % 1024U);
        }
        // 17 is coprime to 1024: each 1024-value block is a permutation of 0..1023.
        constexpr std::uint64_t expected = 64ULL * (1023ULL * 1024ULL / 2ULL);
        if (sum_a(data) != expected || sum_b(data) != expected) {
            throw std::runtime_error("closed-form correctness failure");
        }
        std::cout << "correctness passed; expected_sum=" << expected << '\n';

#if defined(__GNUC__) || defined(__clang__)
        std::array<double, 20> timer_pairs{};
        for (auto& duration : timer_pairs) {
            const auto start = Clock::now();
            const auto stop = Clock::now();
            duration = std::chrono::duration<double, std::nano>(stop - start).count();
        }
        const auto checked_measure = [&](Kernel kernel) {
            const auto sample = measure(kernel, data, passes);
            if (sample.checksum != expected * passes || sample.total_ns < 0) {
                throw std::runtime_error("measured batch correctness failure");
            }
            return sample;
        };
        for (std::size_t i = 0; i < warmup_rounds; ++i) {
            (void)checked_measure((i % 2 == 0) ? sum_a : sum_b);
            (void)checked_measure((i % 2 == 0) ? sum_b : sum_a);
        }
        struct Pair { Sample a; Sample b; };
        std::array<Pair, rounds> samples{};
        for (std::size_t round = 0; round < rounds; ++round) {
            if (round % 2 == 0) {
                samples[round].a = checked_measure(sum_a);
                samples[round].b = checked_measure(sum_b);
            } else {
                samples[round].b = checked_measure(sum_b);
                samples[round].a = checked_measure(sum_a);
            }
        }

        std::cout << "elements=" << count << " bytes=" << count * sizeof(data[0])
                  << " passes_per_batch=" << passes << " warmup_pairs=" << warmup_rounds
                  << " measured_pairs=" << rounds << '\n';
        std::cout << "compiler=" << __VERSION__ << '\n';
        std::cout << std::fixed << std::setprecision(3);
        std::cout << "timer_pair_ns";
        for (const auto duration : timer_pairs) std::cout << ',' << duration;
        std::cout << "\nround,order,a_batch_ns,b_batch_ns,a_checksum,b_checksum\n";
        std::vector<double> a_means, b_means;
        a_means.reserve(rounds);
        b_means.reserve(rounds);
        for (std::size_t round = 0; round < rounds; ++round) {
            const auto& pair = samples[round];
            std::cout << round << ',' << ((round % 2 == 0) ? "AB" : "BA") << ','
                      << pair.a.total_ns << ',' << pair.b.total_ns << ','
                      << pair.a.checksum << ',' << pair.b.checksum << '\n';
            a_means.push_back(pair.a.total_ns / static_cast<double>(passes));
            b_means.push_back(pair.b.total_ns / static_cast<double>(passes));
        }
        const auto report = [](const char* name, const std::vector<double>& means) {
            std::cout << name << " batch_mean_ns_per_sum nearest_rank n=" << means.size()
                      << " p50=" << nearest_rank(means, 50, 100)
                      << " p99=" << nearest_rank(means, 99, 100)
                      << " p99.9=" << nearest_rank(means, 999, 1000) << '\n';
        };
        report("A", a_means);
        report("B", b_means);
        std::cout << "n=20: p99 and p99.9 both select max; no reliable tail estimate\n"
                  << "batch means are not per-request latencies; no winner asserted\n";
#else
        (void)passes;
        (void)warmup_rounds;
        (void)rounds;
        std::cout << "SKIP timing: demo compiler barrier requires GCC or Clang\n";
#endif
    } catch (const std::exception& error) {
        std::cerr << "demo failed: " << error.what() << '\n';
        return 1;
    }
}
