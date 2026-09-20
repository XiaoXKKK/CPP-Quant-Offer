#include <algorithm>
#include <array>
#include <cstdint>
#include <iostream>
#include <limits>
#include <span>
#include <stdexcept>
#include <vector>

namespace {
constexpr std::size_t repeats = 64;
constexpr std::size_t domain = 256;
constexpr std::size_t count = domain * repeats;
using Values = std::span<const std::uint16_t>;

void require(bool condition, const char* message) {
    if (!condition) {
        throw std::runtime_error(message);
    }
}

// Preserve named inspection boundaries. This GNU attribute does not force
// conditional jumps, prohibit vectorization, or provide a timing guarantee.
[[gnu::noinline]] std::uint64_t sum_if(Values values, std::uint32_t threshold) {
    std::uint64_t sum = 0;
    for (const auto value : values) {
        if (value >= threshold) {
            sum += value;
        }
    }
    return sum;
}

[[gnu::noinline]] std::uint64_t sum_select(Values values, std::uint32_t threshold) {
    std::uint64_t sum = 0;
    for (const auto value : values) {
        sum += value >= threshold ? value : 0U;
    }
    return sum;
}

[[gnu::noinline]] std::uint64_t sum_mask(Values values, std::uint32_t threshold) {
    std::uint64_t sum = 0;
    for (const auto value : values) {
        const auto mask = std::uint64_t{0} - static_cast<std::uint64_t>(value >= threshold);
        sum += static_cast<std::uint64_t>(value) & mask;
    }
    return sum;
}

std::uint32_t next_random(std::uint32_t& state) {
    state ^= state << 13;
    state ^= state >> 17;
    state ^= state << 5;
    return state;
}

void deterministic_shuffle(std::vector<std::uint16_t>& values) {
    std::uint32_t state = 0x12345678U;
    // A reproducible permutation, not an unbiased random-sampling claim.
    for (std::size_t size = values.size(); size > 1; --size) {
        const auto index = static_cast<std::size_t>(next_random(state)) % size;
        std::swap(values[size - 1], values[index]);
    }
}

std::array<std::size_t, domain> histogram(Values values) {
    std::array<std::size_t, domain> result{};
    for (const auto value : values) {
        require(value < domain, "histogram value outside generated domain");
        ++result[value];
    }
    return result;
}

std::uint64_t generated_oracle(std::uint32_t threshold) {
    if (threshold >= domain) {
        return 0;
    }
    // Each integer [0,255] appears 64 times. Independent arithmetic-series oracle.
    const auto terms = static_cast<std::uint64_t>(domain) - threshold;
    return repeats * (static_cast<std::uint64_t>(threshold) + domain - 1U) * terms / 2U;
}

void verify(Values values, std::uint32_t threshold, std::uint64_t expected) {
    require(sum_if(values, threshold) == expected, "if result mismatch");
    require(sum_select(values, threshold) == expected, "select result mismatch");
    require(sum_mask(values, threshold) == expected, "mask result mismatch");
}

void test_boundaries() {
    const std::array<std::uint16_t, 0> empty{};
    verify(empty, 0, 0);
    verify(empty, 65536U, 0);
    const std::array<std::uint16_t, 1> one{65535};
    verify(one, 0, 65535);
    verify(one, 65535U, 65535);
    verify(one, 65536U, 0);
    const std::array<std::uint16_t, 5> edges{0, 1, 127, 128, 255};
    verify(edges, 0, 511);
    verify(edges, 128, 383);
    verify(edges, 256, 0);
    verify(edges, std::numeric_limits<std::uint32_t>::max(), 0);
    require(generated_oracle(0) == 2088960, "oracle total");
    require(generated_oracle(128) == 1568768, "oracle midpoint");
    require(generated_oracle(255) == 16320, "oracle upper element");
}
} // namespace

int main() {
    try {
        test_boundaries();
        std::vector<std::uint16_t> sorted;
        sorted.reserve(count);
        for (std::uint16_t value = 0; value < domain; ++value) {
            for (std::size_t repetition = 0; repetition < repeats; ++repetition) {
                sorted.push_back(value);
            }
        }
        auto shuffled = sorted;
        deterministic_shuffle(shuffled);
        auto second_shuffle = sorted;
        deterministic_shuffle(second_shuffle);
        require(shuffled == second_shuffle, "shuffle must reproduce");
        require(shuffled != sorted, "shuffle must change this input order");
        require(std::is_sorted(sorted.begin(), sorted.end()), "sorted input ordering");
        const auto expected_histogram = histogram(sorted);
        require(histogram(shuffled) == expected_histogram, "histogram changed");
        require(std::all_of(expected_histogram.begin(), expected_histogram.end(),
                            [](std::size_t n) { return n == repeats; }),
                "generated histogram count");
        for (std::uint32_t threshold = 0; threshold <= domain; ++threshold) {
            const auto expected = generated_oracle(threshold);
            verify(sorted, threshold, expected);
            verify(shuffled, threshold, expected);
        }
        verify(shuffled, std::numeric_limits<std::uint32_t>::max(), 0);
        std::cout << "values=" << count << " thresholds=257 implementations=3\n"
                  << "threshold=128 sum=" << sum_if(shuffled, 128)
                  << " histogram=equal correctness=OK\n"
                  << "No timing or branch-miss result is measured. Inspect optimized assembly.\n";
    } catch (const std::exception& error) {
        std::cerr << "ERROR: " << error.what() << '\n';
        return 1;
    }
}
