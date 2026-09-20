#include <array>
#include <cassert>
#include <cstddef>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <memory>
#include <new>
#include <stdexcept>
#include <type_traits>

struct Packet {
    char kind;
    double price;
    unsigned quantity;
};

struct ReorderedPacket {
    double price;
    unsigned quantity;
    char kind;
};

struct StandardLayoutOnly {
    int value;
    ~StandardLayoutOnly() {} // A non-trivial destructor prevents trivial copyability.
};

class TriviallyCopyableOnly {
public:
    int visible;
    TriviallyCopyableOnly(int first, int second) : visible(first), hidden_(second) {}
    int hidden() const noexcept { return hidden_; }

private:
    int hidden_;
};

static_assert(std::is_standard_layout_v<Packet>);
static_assert(std::is_trivially_copyable_v<Packet>);
static_assert(std::is_standard_layout_v<StandardLayoutOnly>);
static_assert(!std::is_trivially_copyable_v<StandardLayoutOnly>);
static_assert(std::is_trivially_copyable_v<TriviallyCopyableOnly>);
static_assert(!std::is_standard_layout_v<TriviallyCopyableOnly>);
static_assert(offsetof(Packet, kind) == 0);
static_assert(offsetof(Packet, kind) < offsetof(Packet, price));
static_assert(offsetof(Packet, price) < offsetof(Packet, quantity));
static_assert(offsetof(Packet, quantity) + sizeof(unsigned) <= sizeof(Packet));
static_assert(sizeof(Packet) % alignof(Packet) == 0);
static_assert(alignof(Packet) >= alignof(double));

struct alignas(std::max_align_t) Tracked {
    inline static int attempts = 0;
    inline static int constructed = 0;
    inline static int destroyed = 0;
    inline static int live = 0;
    int value;

    explicit Tracked(int initial, bool fail = false) : value(initial) {
        ++attempts;
        if (fail) {
            throw std::runtime_error("injected construction failure");
        }
        ++constructed;
        ++live;
    }

    Tracked(const Tracked&) = delete;
    Tracked& operator=(const Tracked&) = delete;

    ~Tracked() noexcept {
        assert(live > 0);
        --live;
        ++destroyed;
    }
};

class OneSlot {
public:
    OneSlot() = default;
    OneSlot(const OneSlot&) = delete;
    OneSlot& operator=(const OneSlot&) = delete;

    ~OneSlot() { reset(); }

    Tracked* emplace(int value, bool fail = false) {
        if (object_ != nullptr) {
            throw std::logic_error("slot occupied");
        }
        // The cast locates storage. construct_at creates and initializes Tracked.
        object_ = std::construct_at(reinterpret_cast<Tracked*>(storage_), value, fail);
        return object_;
    }

    void reset() noexcept {
        if (object_ != nullptr) {
            std::destroy_at(object_);
            object_ = nullptr;
        }
    }

    Tracked* get() noexcept { return object_; }

private:
    alignas(Tracked) std::byte storage_[sizeof(Tracked)];
    Tracked* object_ = nullptr;
};

static_assert(alignof(OneSlot) >= alignof(Tracked));
static_assert(!std::is_copy_constructible_v<OneSlot>);
static_assert(!std::is_move_constructible_v<OneSlot>);

void check_layout_and_bytes() {
    std::cout << "Packet: size=" << sizeof(Packet) << " align=" << alignof(Packet)
              << " offsets=" << offsetof(Packet, kind) << ',' << offsetof(Packet, price)
              << ',' << offsetof(Packet, quantity) << '\n';
    std::cout << "ReorderedPacket: size=" << sizeof(ReorderedPacket)
              << " align=" << alignof(ReorderedPacket) << '\n';

    Packet original{'B', 100.25, 7};
    std::array<std::byte, sizeof(Packet)> bytes{};
    std::memcpy(bytes.data(), &original, sizeof(original));
    Packet restored{};
    std::memcpy(&restored, bytes.data(), sizeof(restored));
    assert(restored.kind == original.kind);
    assert(restored.price == original.price);
    assert(restored.quantity == original.quantity);
    // Compare members, never interpret padding or use memcmp as value equality.
}

void check_implicit_creation() {
    struct Pair { int bid; int ask; };
    void* storage = std::malloc(sizeof(Pair));
    if (storage == nullptr) {
        throw std::bad_alloc{};
    }
    // C++20 malloc may implicitly create this implicit-lifetime Pair and its ints.
    // No user-defined constructor is being bypassed, and every field is assigned.
    auto* pair = static_cast<Pair*>(storage);
    pair->bid = 100;
    pair->ask = 101;
    assert(pair->ask - pair->bid == 1);
    std::free(storage);
}

void check_explicit_lifetimes() {
    {
        OneSlot slot;
        assert(slot.get() == nullptr && Tracked::live == 0);
        auto* first = slot.emplace(11);
        assert(first->value == 11 && Tracked::live == 1);

        bool rejected = false;
        try {
            slot.emplace(12);
        } catch (const std::logic_error&) {
            rejected = true;
        }
        assert(rejected && slot.get() == first && first->value == 11);
        assert(Tracked::attempts == 1);

        slot.reset();
        slot.reset(); // Empty reset must not destroy twice.
        assert(slot.get() == nullptr && Tracked::live == 0);
        // Do not access *first in this interval: no live Tracked occupies the slot.
        auto* second = slot.emplace(22);
        assert(second->value == 22);
        // Same non-const complete type, exactly overlapping storage: transparent replacement.
        assert(first == second && first->value == 22);
        slot.reset();

        bool failed = false;
        try {
            slot.emplace(33, true);
        } catch (const std::runtime_error&) {
            failed = true;
        }
        assert(failed && slot.get() == nullptr && Tracked::live == 0);
        assert(Tracked::constructed == 2 && Tracked::destroyed == 2);
        auto* final = slot.emplace(44);
        assert(final->value == 44 && Tracked::live == 1);
    } // OneSlot destroys the final Tracked; the byte array itself does not do so.
    assert(Tracked::attempts == 4);
    assert(Tracked::constructed == 3 && Tracked::destroyed == 3 && Tracked::live == 0);
    std::cout << "attempts=" << Tracked::attempts << " constructed=" << Tracked::constructed
              << " destroyed=" << Tracked::destroyed << " live=" << Tracked::live << '\n';
}

int main() {
    check_layout_and_bytes();
    check_implicit_creation();
    check_explicit_lifetimes();
    std::cout << "layout, representation and lifetime checks passed\n";
}
