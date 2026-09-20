#include <cassert>
#include <cstddef>
#include <iostream>
#include <limits>
#include <span>
#include <stdexcept>
#include <vector>

namespace {

void reserve_and_append() {
    std::vector<int> values{10, 20};
    values.reserve(8);
    const auto capacity = values.capacity();
    auto first = values.begin();
    int* pointer = &values.front();
    int& reference = values.front();
    const auto old_end = values.end();

    values.reserve(capacity);
    values.reserve(0);
    assert(values.size() == 2 && values.capacity() == capacity);
    assert(first == values.begin() && pointer == &values.front());
    assert(reference == 10 && old_end == values.end());

    std::span<const int> prefix(values);
    values.push_back(30);
    // old_end is invalid now. Do not compare or dereference it.
    assert(values.capacity() == capacity);
    assert(*first == 10 && *pointer == 10 && reference == 10);
    assert(prefix.size() == 2 && prefix[1] == 20);
    assert(values.size() == 3 && values.back() == 30);
}

void forced_reallocation() {
    std::vector<int> values{11, 22, 33};
    const auto old_capacity = values.capacity();
    const std::size_t saved_index = 1;
    const int saved_value = values[saved_index];
    assert(old_capacity < values.max_size());

    // Save a value and index, not a borrowed pointer across reallocation.
    values.reserve(old_capacity + 1);
    assert(values.capacity() > old_capacity);
    assert(values.size() == 3 && values[saved_index] == saved_value);
    auto fresh = values.begin() + static_cast<std::ptrdiff_t>(saved_index);
    assert(*fresh == 22);
}

void resize_boundaries() {
    std::vector<int> values{1, 2, 3};
    values.reserve(8);
    const auto capacity = values.capacity();
    auto first = values.begin();
    int& reference = values.front();

    values.resize(1);
    assert(values.capacity() == capacity && *first == 1 && reference == 1);
    const auto unchanged_end = values.end();
    values.resize(values.size());
    assert(unchanged_end == values.end());

    values.resize(4);
    // unchanged_end is invalid after growth; first and reference remain valid.
    assert(values.capacity() == capacity && *first == 1 && reference == 1);
    assert((values == std::vector<int>{1, 0, 0, 0}));
    values.resize(0);
    assert(values.empty() && values.capacity() == capacity);
    // first and reference are no longer used.
}

void insert_and_erase() {
    std::vector<int> values{10, 20, 30};
    values.reserve(8);
    auto first = values.begin();
    int& reference = values.front();

    auto inserted = values.insert(values.begin() + 1, 15);
    assert(*first == 10 && reference == 10 && *inserted == 15);
    assert((values == std::vector<int>{10, 15, 20, 30}));

    auto next = values.erase(inserted);
    // inserted is invalid; erase returned a new iterator.
    assert(*first == 10 && reference == 10 && *next == 20);
    assert((values == std::vector<int>{10, 20, 30}));
    next = values.erase(values.end() - 1);
    assert(next == values.end());
    assert((values == std::vector<int>{10, 20}));

    auto empty_result = values.erase(values.begin(), values.begin());
    assert(empty_result == values.begin() && values.size() == 2);
    values.clear();
    assert(values.empty());
    assert(values.erase(values.begin(), values.end()) == values.end());
}

void erase_loop() {
    for (const auto& input : {std::vector<int>{}, std::vector<int>{2, 4},
                              std::vector<int>{1, 3}, std::vector<int>{1, 2, 4, 3}}) {
        auto values = input;
        for (auto it = values.begin(); it != values.end();) {
            if (*it % 2 == 0) {
                it = values.erase(it);
            } else {
                ++it;
            }
        }
        std::vector<int> expected;
        for (int value : input) {
            if (value % 2 != 0) expected.push_back(value);
        }
        assert(values == expected);
    }
}

void append_original_elements() {
    std::vector<int> values{1, 2, 3};
    const auto original_size = values.size();
    for (std::size_t i = 0; i < original_size; ++i) {
        const int doubled = values[i] * 2;
        values.push_back(doubled);
    }
    assert((values == std::vector<int>{1, 2, 3, 2, 4, 6}));
}

struct Item {
    int value;
    explicit Item(int input) : value(input) {
        if (input == 99) throw std::runtime_error("injected construction failure");
    }
};

void exception_no_effects() {
    std::vector<Item> values;
    values.reserve(4);
    values.emplace_back(7);
    const auto old_capacity = values.capacity();
    auto first = values.begin();
    const auto old_end = values.end();
    bool caught = false;
    try {
        values.emplace_back(99);
    } catch (const std::runtime_error&) {
        caught = true;
    }
    assert(caught && values.size() == 1 && values.capacity() == old_capacity);
    assert(first == values.begin() && first->value == 7 && old_end == values.end());

    std::vector<int> ints{3};
    if (ints.max_size() < std::numeric_limits<std::size_t>::max()) {
        const auto before = ints.capacity();
        caught = false;
        try {
            ints.reserve(ints.max_size() + 1);
        } catch (const std::length_error&) {
            caught = true;
        }
        assert(caught && ints.size() == 1 && ints.front() == 3);
        assert(ints.capacity() == before);
    }
}

void shrink_request() {
    std::vector<int> values{5, 6, 7};
    values.reserve(32);
    const auto capacity = values.capacity();
    values.shrink_to_fit();
    assert(values.capacity() >= values.size() && values.capacity() <= capacity);
    assert((values == std::vector<int>{5, 6, 7}));
    // Reacquire all handles; the request is allowed to keep the allocation.
    assert(*values.begin() == 5);

    values.clear();
    values.shrink_to_fit();
    assert(values.empty());
    assert(values.begin() == values.end());
}

} // namespace

int main() {
    reserve_and_append();
    forced_reallocation();
    resize_boundaries();
    insert_and_erase();
    erase_loop();
    append_original_elements();
    exception_no_effects();
    shrink_request();
    std::cout << "vector boundary checks passed\n";
}
