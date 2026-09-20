#include <cassert>
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <limits>
#include <type_traits>

#if !defined(__linux__) || !defined(__x86_64__) || !defined(__GNUC__)
#error "This ABI inspection demo targets Linux x86-64 with a GNU-compatible compiler."
#endif

#define ABI_EXPORT __attribute__((visibility("default")))
#define ABI_HIDDEN __attribute__((visibility("hidden")))

namespace abi_demo {
unsigned scale(unsigned value) noexcept { return value * 2U; }
double scale(double value) noexcept { return value * 2.0; }
} // namespace abi_demo

namespace {
unsigned local_adjust(unsigned value) noexcept { return value ^ 0x5aU; }
} // namespace

extern "C" ABI_HIDDEN unsigned abi_hidden_probe(unsigned value) noexcept {
    return local_adjust(value);
}

struct AbiRequestV1 {
    std::uint32_t abi_version;
    std::uint32_t struct_size;
    std::int32_t left;
    std::int32_t right;
};

static_assert(std::is_standard_layout_v<AbiRequestV1>);
static_assert(std::is_trivially_copyable_v<AbiRequestV1>);
static_assert(sizeof(AbiRequestV1) <= std::numeric_limits<std::uint32_t>::max());

enum AbiStatus {
    abi_ok = 0,
    abi_bad_argument = 1,
    abi_bad_version = 2,
    abi_bad_size = 3,
    abi_out_of_range = 4
};

// Non-null arguments must point to live, accessible objects of the declared types.
// struct_size is a version contract, not a check for arbitrary pointer validity.
// No ownership is transferred. On failure, *result is unchanged.
extern "C" ABI_EXPORT int abi_sum_v1(const AbiRequestV1* request,
                                     std::int32_t* result) noexcept {
    if (request == nullptr || result == nullptr) {
        return abi_bad_argument;
    }
    if (request->abi_version != 1U) {
        return abi_bad_version;
    }
    if (request->struct_size != sizeof(AbiRequestV1)) {
        return abi_bad_size;
    }
    const auto sum = static_cast<std::int64_t>(request->left) + request->right;
    if (sum < std::numeric_limits<std::int32_t>::min() ||
        sum > std::numeric_limits<std::int32_t>::max()) {
        return abi_out_of_range;
    }
    *result = static_cast<std::int32_t>(sum);
    return abi_ok;
}

int main() {
    assert(abi_demo::scale(6U) == 12U);
    assert(abi_demo::scale(1.25) == 2.5);
    assert(abi_hidden_probe(0U) == 0x5aU);

    AbiRequestV1 request{1U, static_cast<std::uint32_t>(sizeof(AbiRequestV1)), 20, 22};
    std::int32_t result = -77;
    assert(abi_sum_v1(&request, &result) == abi_ok && result == 42);

    result = -77;
    assert(abi_sum_v1(nullptr, &result) == abi_bad_argument && result == -77);
    assert(abi_sum_v1(&request, nullptr) == abi_bad_argument);

    request.abi_version = 2U;
    assert(abi_sum_v1(&request, &result) == abi_bad_version && result == -77);
    request.abi_version = 1U;
    request.struct_size = 0U;
    assert(abi_sum_v1(&request, &result) == abi_bad_size && result == -77);
    request.struct_size = static_cast<std::uint32_t>(sizeof(AbiRequestV1));

    request.left = std::numeric_limits<std::int32_t>::max();
    request.right = 1;
    assert(abi_sum_v1(&request, &result) == abi_out_of_range && result == -77);
    request.left = std::numeric_limits<std::int32_t>::min();
    request.right = -1;
    assert(abi_sum_v1(&request, &result) == abi_out_of_range && result == -77);
    request.left = -20;
    request.right = 22;
    assert(abi_sum_v1(&request, &result) == abi_ok && result == 2);

    std::cout << "request layout: size=" << sizeof(AbiRequestV1)
              << " align=" << alignof(AbiRequestV1)
              << " left-offset=" << offsetof(AbiRequestV1, left) << '\n';
    std::cout << "overloads, linkage and versioned boundary checks passed\n";
}
