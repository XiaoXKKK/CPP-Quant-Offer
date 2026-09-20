#include <array>
#include <cassert>
#include <concepts>
#include <cstdint>
#include <iostream>
#include <optional>
#include <type_traits>
#include <utility>

namespace {

template<class T>
concept HasBid = requires(const T& quote) { quote.bid_ticks(); };

template<class T>
concept QuoteLike = requires(const T& quote) {
    typename T::price_type;
    requires std::same_as<typename T::price_type, std::int64_t>;
    { quote.bid_ticks() } -> std::same_as<std::int64_t>;
    { quote.ask_ticks() } -> std::same_as<std::int64_t>;
};

template<class T>
concept NoThrowQuote = QuoteLike<T> && requires(const T& quote) {
    { quote.bid_ticks() } noexcept;
    { quote.ask_ticks() } noexcept;
};

struct Quote {
    using price_type = std::int64_t;
    std::int64_t bid;
    std::int64_t ask;
    constexpr std::int64_t bid_ticks() const noexcept { return bid; }
    constexpr std::int64_t ask_ticks() const noexcept { return ask; }
};

struct PotentiallyThrowingQuote {
    using price_type = std::int64_t;
    std::int64_t bid_ticks() const { return 10; }
    std::int64_t ask_ticks() const { return 12; }
};

struct MissingAsk {
    using price_type = std::int64_t;
    std::int64_t bid_ticks() const { return 10; }
};

struct WrongPrice {
    using price_type = double;
    double bid_ticks() const { return 1.0; }
    double ask_ticks() const { return 2.0; }
};

struct MutableOnly {
    using price_type = std::int64_t;
    std::int64_t bid_ticks() { return 10; }
    std::int64_t ask_ticks() { return 12; }
};

static_assert(HasBid<Quote> && HasBid<MissingAsk>);
static_assert(!HasBid<int>);
static_assert(QuoteLike<Quote> && NoThrowQuote<Quote>);
static_assert(QuoteLike<PotentiallyThrowingQuote>);
static_assert(!NoThrowQuote<PotentiallyThrowingQuote>);
static_assert(!QuoteLike<MissingAsk> && !QuoteLike<WrongPrice>);
static_assert(!QuoteLike<MutableOnly> && !QuoteLike<int>);

// A simple requirement only asks whether this expression is well-formed.
template<class T>
concept IntegralSpelling = requires { std::is_integral_v<T>; };
template<class T>
concept ReallyIntegral = requires { requires std::is_integral_v<T>; };
static_assert(IntegralSpelling<double>);
static_assert(!ReallyIntegral<double> && ReallyIntegral<int>);

template<class T>
concept HasPriceType = requires { typename T::price_type; };
template<class T>
concept IntegralPrice = HasPriceType<T> && std::integral<typename T::price_type>;
static_assert(IntegralPrice<Quote>);
static_assert(!IntegralPrice<int> && !IntegralPrice<WrongPrice>);

template<class T, class = void>
struct LegacyHasBid : std::false_type {};
template<class T>
struct LegacyHasBid<T, std::void_t<decltype(std::declval<const T&>().bid_ticks())>>
    : std::true_type {};
static_assert(LegacyHasBid<Quote>::value && !LegacyHasBid<int>::value);

template<QuoteLike T>
constexpr int route(const T&) { return 1; }
template<NoThrowQuote T>
constexpr int route(const T&) { return 2; }
static_assert(route(Quote{10, 12}) == 2);
static_assert(route(PotentiallyThrowingQuote{}) == 1);

// This teaching model accepts nonnegative ticks up to this local limit.
constexpr std::int64_t max_demo_ticks = 1'000'000'000;

template<QuoteLike T>
std::optional<std::int64_t> checked_spread(const T& quote) {
    const auto bid = quote.bid_ticks();
    const auto ask = quote.ask_ticks();
    if (bid < 0 || ask < 0 || bid > max_demo_ticks || ask > max_demo_ticks || ask < bid) {
        return std::nullopt;
    }
    return ask - bid;
}

// An explicit return type lets callers form this expression without its body.
template<class T>
int body_checked_later(T value) { return value.missing(); }
template<class T>
concept DeclarationCallable = requires(T value) { body_checked_later(value); };
static_assert(DeclarationCallable<int>);

// These modes are intentionally ill-formed and are never enabled for the demo.
#if defined(CONCEPTS_FAIL_SUBSUMPTION)
template<class T> requires std::is_integral_v<T>
constexpr int ambiguous(T) { return 1; }
template<class T> requires (std::is_integral_v<T> && std::is_signed_v<T>)
constexpr int ambiguous(T) { return 2; }
static_assert(ambiguous(1) == 2);
#endif

#if defined(CONCEPTS_FAIL_NON_TEMPLATE)
constexpr bool outside_template = requires(int value) { value.bid_ticks(); };
#endif

} // namespace

int main() {
#if defined(CONCEPTS_FAIL_BODY)
    (void)body_checked_later(1);
#endif
#if defined(CONCEPTS_FAIL_CONSTRAINT)
    (void)checked_spread(MissingAsk{});
#endif

    assert(checked_spread(Quote{10, 12}) == 2);
    assert(checked_spread(PotentiallyThrowingQuote{}) == 2);
    assert(checked_spread(Quote{0, 0}) == 0);
    assert(checked_spread(Quote{0, max_demo_ticks}) == max_demo_ticks);
    assert(!checked_spread(Quote{12, 10}));
    assert(!checked_spread(Quote{-1, 10}));
    assert(!checked_spread(Quote{0, max_demo_ticks + 1}));

    const std::array<Quote, 3> batch{{{10, 12}, {20, 21}, {0, 0}}};
    std::int64_t total = 0;
    for (const auto& quote : batch) {
        const auto spread = checked_spread(quote);
        assert(spread.has_value());
        total += *spread;
    }
    assert(total == 3);
    std::cout << "concept checks passed; total spread=" << total << '\n';
}
