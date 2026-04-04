#pragma once
#include <cstddef>
#include <array>
#include <atomic>

/**
 * @brief static fixed size, lock-free, single consumer, single producer FIFO queue
 * @ref Lock-free Queues in the Multiverse of Madness - Dave Rowland - ADC 2025
 *      v4
 */

template <typename T, size_t N>
class ring_buffer
{
    static_assert((N > 1) && ((N & (N - 1)) == 0), "N must be power of 2");

public:
    // producer thread
    bool try_push(const T &value) noexcept(std::is_nothrow_copy_assignable_v<T>)
    {
        size_t current_tail = tail_.load(std::memory_order_relaxed);
        size_t current_head = head_.load(std::memory_order_acquire);

        size_t size = current_tail - current_head;

        if (size >= capacity_) // full
            return false;

        data_[current_tail & capacity_] = value;
        tail_.store(current_tail + 1, std::memory_order_release);

        return true;
    }

    // producer thread
    bool full() const noexcept
    {
        size_t current_tail = tail_.load(std::memory_order_relaxed);
        size_t current_head = head_.load(std::memory_order_acquire);
        return (current_tail - current_head) >= capacity_;
    }

    // consumer thread
    bool try_pop(T &value) noexcept(std::is_nothrow_copy_assignable_v<T>)
    {
        size_t current_head = head_.load(std::memory_order_relaxed);
        size_t current_tail = tail_.load(std::memory_order_acquire);

        if (current_head == current_tail) // empty
            return false;

        value = data_[current_head & capacity_];
        head_.store(current_head + 1, std::memory_order_release);

        return true;
    }

    // consumer thread
    bool empty() const noexcept
    {
        size_t current_head = head_.load(std::memory_order_relaxed);
        size_t current_tail = tail_.load(std::memory_order_acquire);
        return current_head == current_tail;
    }

    // both producer and consumer thread
    size_t size() const noexcept
    {
        size_t current_head = head_.load(std::memory_order_acquire);
        size_t current_tail = tail_.load(std::memory_order_acquire);

        return current_tail - current_head;
    }

    // should only be called when no other thread is using the queue
    void clear() noexcept
    {
        head_.store(tail_.load(std::memory_order_relaxed), std::memory_order_relaxed);
    }

    size_t capacity() const noexcept { return capacity_; }

private:
    static constexpr size_t capacity_ = N - 1; // also used as bitmask for index
    std::array<T, N> data_{};

    // head and tail increase monotonically
    std::atomic<size_t> head_{0};
    std::atomic<size_t> tail_{0};
};