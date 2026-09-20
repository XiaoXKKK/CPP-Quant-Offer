#include <cassert>
#include <iostream>

#if defined(ODR_MULTI_TU)
#if (defined(ODR_TU_A) + defined(ODR_TU_B) + defined(ODR_TU_MAIN)) != 1
#error "Select exactly one of ODR_TU_A, ODR_TU_B, ODR_TU_MAIN"
#endif
#endif

// Header-like region: the definitions here have identical tokens in every TU.
namespace odr_demo {
extern int external_value;
extern int declared_only;  // No definition: the sizeof below does not odr-use it.
static_assert(sizeof(declared_only) == sizeof(int));

inline int shared_visits = 0;

inline int& function_counter() {
    static int count = 0;
    return count;
}

inline void touch_shared() {
    ++shared_visits;
    ++function_counter();
}

[[maybe_unused]] static int internal_value = 0;

struct View {
    int* external;
    int* shared;
    int* function_local;
    int* internal;
};

View view_a();
View view_b();
void touch_a();
void touch_b();
}  // namespace odr_demo

#if !defined(ODR_MULTI_TU) || defined(ODR_TU_A)
namespace odr_demo {
#ifndef ODR_BAD_MISSING
int external_value = 7;
#endif

#ifdef ODR_BAD_SAME_TU
int external_value = 11;  // Intentional compile failure; never run this variant.
#endif

View view_a() {
    return {&external_value, &shared_visits, &function_counter(), &internal_value};
}

void touch_a() {
    ++external_value;
    ++internal_value;
    touch_shared();
}
}  // namespace odr_demo
#endif

#if !defined(ODR_MULTI_TU) || defined(ODR_TU_B)
namespace odr_demo {
#ifdef ODR_BAD_DUPLICATE
int external_value = 11;  // Cross-TU violation: expected GNU linker failure.
#endif

View view_b() {
    return {&external_value, &shared_visits, &function_counter(), &internal_value};
}

void touch_b() {
    ++external_value;
    ++internal_value;
    touch_shared();
}
}  // namespace odr_demo
#endif

#if !defined(ODR_MULTI_TU) || defined(ODR_TU_MAIN)
int main() {
    using namespace odr_demo;
    const View a = view_a();
    const View b = view_b();
    assert(a.external == b.external && *a.external == 7);
    assert(a.shared == b.shared && *a.shared == 0);
    assert(a.function_local == b.function_local && *a.function_local == 0);
#ifdef ODR_MULTI_TU
    assert(a.internal != b.internal);
#else
    assert(a.internal == b.internal);
#endif
    touch_a();
    touch_b();
    assert(*a.external == 9);
    assert(*a.shared == 2);
    assert(*a.function_local == 2);
#ifdef ODR_MULTI_TU
    assert(*a.internal == 1 && *b.internal == 1);
    std::cout << "multi-TU: shared entities and separate internal state OK\n";
#else
    assert(*a.internal == 2 && *b.internal == 2);
    std::cout << "single-TU: entity identity and bounded updates OK\n";
#endif
}
#endif
