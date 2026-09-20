#include <cassert>
#include <iostream>
#include <memory>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

template <bool NothrowMove>
struct Trace {
    inline static int copies = 0;
    inline static int moves = 0;
    int value;

    explicit Trace(int initial) : value(initial) {}
    Trace(const Trace& other) : value(other.value) { ++copies; }
    Trace(Trace&& other) noexcept(NothrowMove)
        : value(std::exchange(other.value, -1)) {
        ++moves;
    }
    Trace& operator=(const Trace&) = default;
    Trace& operator=(Trace&&) = default;

    static void reset() noexcept { copies = moves = 0; }
};

using Safe = Trace<true>;
using Risky = Trace<false>; // Potentially throwing signature; this body does not throw.

enum class Category { mutable_lvalue, const_lvalue, rvalue };

constexpr Category inspect(Safe&) { return Category::mutable_lvalue; }
constexpr Category inspect(const Safe&) { return Category::const_lvalue; }
constexpr Category inspect(Safe&&) { return Category::rvalue; }

template <class T>
constexpr Category relay(T&& value) {
    return inspect(std::forward<T>(value));
}

struct CopyOnly {
    CopyOnly() = default;
    CopyOnly(const CopyOnly&) = default;
};

struct DeletedMove {
    DeletedMove() = default;
    DeletedMove(const DeletedMove&) = default;
    DeletedMove(DeletedMove&&) = delete;
};

struct Immovable {
    int value;
    explicit Immovable(int initial) : value(initial) {}
    Immovable(const Immovable&) = delete;
    Immovable(Immovable&&) = delete;
};

Immovable make_prvalue() { return Immovable{73}; }

Safe make_named() {
    Safe local{91};
    return local; // NRVO is optional. The fallback move is valid.
}

template <class T>
void observe_reallocation(const char* label) {
    std::vector<T> values;
    values.reserve(2);
    values.emplace_back(10);
    values.emplace_back(20);
    T::reset();
    // reserve(3) might fit an implementation's larger initial capacity.
    assert(values.capacity() < values.max_size());
    values.reserve(values.capacity() + 1);
    assert(values.size() == 2);
    assert(values[0].value == 10 && values[1].value == 20);
    std::cout << label << " reserve: copies=" << T::copies
              << " moves=" << T::moves << '\n';
    // No assertion about the library's choice of relocation strategy.
}

int main() {
    Safe source{42};
    Safe&& alias = std::move(source);
    static_assert(std::is_same_v<decltype(alias), Safe&&>);
    static_assert(std::is_same_v<decltype((alias)), Safe&>);
    static_assert(std::is_same_v<decltype(std::move(source)), Safe&&>);
    static_assert(std::is_same_v<decltype(Safe{1}), Safe>);
    assert(&alias == &source && source.value == 42);
    assert(inspect(alias) == Category::mutable_lvalue);
    assert(inspect(std::move(alias)) == Category::rvalue);
    assert(relay(source) == Category::mutable_lvalue);
    assert(relay(std::move(source)) == Category::rvalue);
    assert(source.value == 42); // The selected overload only inspects the category.

    Safe::reset();
    Safe moved{std::move(source)};
    assert(moved.value == 42 && source.value == -1);
    assert(Safe::copies == 0 && Safe::moves == 1);
    const Safe fixed{7};
    static_assert(std::is_same_v<decltype(std::move(fixed)), const Safe&&>);
    Safe copied{std::move(fixed)};
    assert(copied.value == 7 && fixed.value == 7);
    assert(Safe::copies == 1 && Safe::moves == 1);
    assert(relay(fixed) == Category::const_lvalue);
    std::cout << "explicit construction: copies=" << Safe::copies
              << " moves=" << Safe::moves << '\n';

    static_assert(std::is_move_constructible_v<CopyOnly>);
    static_assert(!std::is_move_constructible_v<DeletedMove>);
    static_assert(std::is_nothrow_move_constructible_v<Safe>);
    static_assert(!std::is_nothrow_move_constructible_v<Risky>);
    static_assert(std::is_same_v<decltype(std::move_if_noexcept(source)), Safe&&>);
    Risky risky{55};
    static_assert(std::is_same_v<decltype(std::move_if_noexcept(risky)), const Risky&>);
    Risky::reset();
    Risky preserved{std::move_if_noexcept(risky)};
    assert(preserved.value == 55 && risky.value == 55);
    assert(Risky::copies == 1 && Risky::moves == 0);

    auto owner = std::make_unique<int>(12);
    auto destination = std::move(owner);
    assert(!owner && destination && *destination == 12);
    owner = std::make_unique<int>(13);
    assert(*owner == 13);

    std::string text = "market-data-payload";
    std::string received = std::move(text);
    assert(received == "market-data-payload");
    // No assertion about text.empty(): its value after the move is unspecified.
    text.clear();
    text = "reused";
    assert(text == "reused");

    auto direct = make_prvalue();
    assert(direct.value == 73);
    Safe::reset();
    auto named = make_named();
    assert(named.value == 91);
    std::cout << "named return: copies=" << Safe::copies
              << " moves=" << Safe::moves << '\n';

    observe_reallocation<Safe>("nothrow");
    observe_reallocation<Risky>("potentially throwing");
    std::cout << "value categories, forwarding, ownership and return checks passed\n";
}
