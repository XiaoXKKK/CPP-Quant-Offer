#include <array>
#include <cassert>
#include <chrono>
#include <condition_variable>
#include <cstddef>
#include <exception>
#include <iostream>
#include <mutex>
#include <stdexcept>
#include <thread>

using Clock = std::chrono::steady_clock;
enum class Status { success, closed, timeout };

template <std::size_t Capacity>
class BoundedQueue {
    static_assert(Capacity > 0);
    std::array<int, Capacity> values_{};
    std::size_t head_ = 0;
    std::size_t tail_ = 0;
    std::size_t size_ = 0;
    bool closed_ = false;
    std::mutex mutex_;
    std::condition_variable not_empty_;
    std::condition_variable not_full_;

public:
    Status push_until(int value, Clock::time_point deadline) {
        std::unique_lock lock(mutex_);
        if (!not_full_.wait_until(lock, deadline, [this] {
                return closed_ || size_ < Capacity;
            })) {
            return Status::timeout;
        }
        if (closed_) {
            return Status::closed;
        }
        values_[tail_] = value;
        tail_ = (tail_ + 1) % Capacity;
        ++size_;
        assert(size_ <= Capacity);
        lock.unlock();
        not_empty_.notify_one();
        return Status::success;
    }

    Status pop_until(int& result, Clock::time_point deadline) {
        std::unique_lock lock(mutex_);
        if (!not_empty_.wait_until(lock, deadline, [this] {
                return closed_ || size_ != 0;
            })) {
            return Status::timeout;
        }
        if (size_ == 0) {
            assert(closed_);
            return Status::closed;
        }
        result = values_[head_];
        head_ = (head_ + 1) % Capacity;
        --size_;
        lock.unlock();
        not_full_.notify_one();
        return Status::success;
    }

    void close() {
        {
            std::lock_guard lock(mutex_);
            closed_ = true;
        }
        not_empty_.notify_all();
        not_full_.notify_all();
    }
};

void check_boundaries() {
    BoundedQueue<2> queue;
    const auto expired = Clock::now();
    int value = -1;
    assert(queue.pop_until(value, expired) == Status::timeout && value == -1);
    // A true predicate permits progress even when the deadline has passed.
    assert(queue.push_until(10, expired) == Status::success);
    assert(queue.push_until(20, expired) == Status::success);
    assert(queue.push_until(30, expired) == Status::timeout);
    assert(queue.pop_until(value, expired) == Status::success && value == 10);
    assert(queue.push_until(30, expired) == Status::success); // Ring wraparound.
    queue.close();
    queue.close();
    assert(queue.push_until(40, expired) == Status::closed);
    assert(queue.pop_until(value, expired) == Status::success && value == 20);
    assert(queue.pop_until(value, expired) == Status::success && value == 30);
    value = -1;
    assert(queue.pop_until(value, expired) == Status::closed && value == -1);
}

void check_close_races(bool initially_full) {
    BoundedQueue<1> queue;
    const auto deadline = Clock::now() + std::chrono::seconds(30);
    if (initially_full) {
        assert(queue.push_until(7, deadline) == Status::success);
    }
    Status status = Status::timeout;
    std::exception_ptr worker_error;
    int output = -1;
    std::jthread worker([&] {
        try {
            status = initially_full ? queue.push_until(8, deadline)
                                    : queue.pop_until(output, deadline);
        } catch (...) {
            worker_error = std::current_exception();
        }
    });
    // Correct whether the worker begins waiting before or after this call.
    queue.close();
    worker.join();
    if (worker_error) {
        std::rethrow_exception(worker_error);
    }
    assert(status == Status::closed && output == -1);
    if (initially_full) {
        assert(queue.pop_until(output, deadline) == Status::success && output == 7);
    }
    assert(queue.pop_until(output, deadline) == Status::closed);
}

struct InjectedProducerFailure {};

void check_transfer(bool fail_producer) {
    constexpr std::size_t total = 128;
    constexpr std::size_t fail_after = 37;
    BoundedQueue<4> queue;
    std::array<int, total> received{};
    std::size_t received_count = 0; // Consumer-owned until join.
    std::size_t accepted_count = 0; // Main-thread-owned.
    std::exception_ptr consumer_error;
    std::exception_ptr producer_error;
    const auto deadline = Clock::now() + std::chrono::seconds(30);

    // All referenced state outlives consumer. Construction failure precedes input.
    std::jthread consumer([&] {
        try {
            for (;;) {
                int value = 0;
                const auto status = queue.pop_until(value, deadline);
                if (status == Status::closed) {
                    break;
                }
                if (status == Status::timeout) {
                    throw std::runtime_error("consumer watchdog expired");
                }
                if (received_count == received.size()) {
                    throw std::runtime_error("unexpected extra item");
                }
                received[received_count++] = value;
            }
        } catch (...) {
            consumer_error = std::current_exception();
            queue.close();
        }
    });

    try {
        for (std::size_t i = 0; i < total; ++i) {
            if (fail_producer && i == fail_after) {
                throw InjectedProducerFailure{};
            }
            const auto status = queue.push_until(static_cast<int>(i + 1), deadline);
            if (status != Status::success) {
                throw std::runtime_error("producer could not enqueue");
            }
            ++accepted_count;
        }
    } catch (...) {
        producer_error = std::current_exception();
    }
    queue.close();
    consumer.join();
    if (consumer_error) {
        std::rethrow_exception(consumer_error);
    }

    bool injected = false;
    if (producer_error) {
        try {
            std::rethrow_exception(producer_error);
        } catch (const InjectedProducerFailure&) {
            injected = true;
        }
    }
    assert(injected == fail_producer);
    assert(accepted_count == (fail_producer ? fail_after : total));
    assert(received_count == accepted_count);
    for (std::size_t i = 0; i < received_count; ++i) {
        assert(received[i] == static_cast<int>(i + 1));
    }
    std::cout << "accepted=" << accepted_count << " drained=" << received_count
              << " injected_failure=" << injected << '\n';
}

int main() {
    try {
        check_boundaries();
        check_close_races(false);
        check_close_races(true);
        check_transfer(false);
        check_transfer(true);
        std::cout << "empty, full, close, drain and failure checks passed\n";
    } catch (const std::exception& error) {
        std::cerr << "demo failed: " << error.what() << '\n';
        return 1;
    } catch (...) {
        std::cerr << "demo failed with an unexpected exception\n";
        return 1;
    }
}
