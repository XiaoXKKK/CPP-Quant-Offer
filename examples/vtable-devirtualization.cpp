#include <array>
#include <cassert>
#include <cstddef>
#include <iostream>
#include <memory>

namespace vtable_demo {

class Value {
public:
    virtual ~Value() = default;
    virtual int value() const noexcept = 0;
};

class Fixed final : public Value {
public:
    explicit Fixed(int value) noexcept : value_(value) {}
    int value() const noexcept override { return value_; }
private:
    int value_;
};

class Flexible : public Value {
public:
    explicit Flexible(int value) noexcept : value_(value) {}
    int value() const noexcept override { return value_; }
private:
    int value_;
};

// External linkage keeps these independently inspectable in compiler output.
int dispatch_unknown(const Value& object) noexcept { return object.value(); }
int dispatch_final(const Fixed& object) noexcept { return object.value(); }

int dispatch_known_local() noexcept {
    Flexible object{7};
    const Value& base = object;
    return base.value();
}

class Label {
public:
    virtual ~Label() = default;
    virtual int label() const noexcept = 0;
};

class Combined final : public Value, public Label {
public:
    Combined(int value, int label) noexcept : value_(value), label_(label) {}
    int value() const noexcept override { return value_; }
    int label() const noexcept override { return label_; }
private:
    int value_;
    int label_;
};

int dispatch_label(const Label& object) noexcept { return object.label(); }

struct Trace {
    std::array<int, 4> events{};
    std::size_t size = 0;

    void record(int event) noexcept {
        assert(size < events.size());
        events[size++] = event;
    }
};

class LifecycleBase {
public:
    explicit LifecycleBase(Trace& trace) noexcept : trace_(trace) {
        trace_.record(100 + kind());
    }
    virtual ~LifecycleBase() { trace_.record(400 + kind()); }
    virtual int kind() const noexcept { return 1; }
protected:
    Trace& trace_;
};

class LifecycleDerived final : public LifecycleBase {
public:
    explicit LifecycleDerived(Trace& trace) noexcept : LifecycleBase(trace) {
        trace_.record(200 + kind());
    }
    ~LifecycleDerived() override { trace_.record(300 + kind()); }
    int kind() const noexcept override { return 2; }
};

} // namespace vtable_demo

int main() {
    using namespace vtable_demo;
    Fixed fixed{42};
    Flexible flexible{-5};
    assert(dispatch_unknown(fixed) == 42);
    assert(dispatch_unknown(flexible) == -5);
    assert(dispatch_final(fixed) == 42);
    assert(dispatch_known_local() == 7);

    Combined combined{11, 22};
    const Value& value_view = combined;
    const Label& label_view = combined;
    assert(dispatch_unknown(value_view) == 11);
    assert(dispatch_label(label_view) == 22);
    assert(dynamic_cast<const Combined*>(&label_view) == &combined);
    assert(dynamic_cast<const void*>(&label_view) == static_cast<const void*>(&combined));
    assert(dynamic_cast<const Fixed*>(&label_view) == nullptr);

    Trace trace;
    {
        std::unique_ptr<LifecycleBase> object = std::make_unique<LifecycleDerived>(trace);
        assert(object->kind() == 2);
        assert(object->LifecycleBase::kind() == 1);
        assert(trace.size == 2);
        assert(trace.events[0] == 101 && trace.events[1] == 202);
    }
    assert(trace.size == 4);
    assert((trace.events == std::array<int, 4>{101, 202, 302, 401}));
    std::cout << "dispatch checks passed; lifecycle=101,202,302,401\n";
}
