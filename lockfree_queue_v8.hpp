#pragma once
#include <cstddef>
#include <array>
#include <atomic>
#include <new> // std::hardware_destructive_interference_size

/**
 * @brief static fixed size, lock-free, single consumer, single producer FIFO queue
 * @ref Lock-free Queues in the Multiverse of Madness - Dave Rowland - ADC 2025
 *      v8, on par with Moodycamel
 */

template <typename T, size_t N>
class lockfree_queue
{
    static_assert((N > 1) && ((N & (N - 1)) == 0), "N must be power of 2");

public:
    // producer thread
    bool try_push(const T &value) noexcept(std::is_nothrow_copy_assignable_v<T>)
    {
        size_t current_tail = tail_.load(std::memory_order_relaxed);
        size_t size = current_tail - cached_head_;

        if (size >= capacity_) // full?
        {
            cached_head_ = head_.load(std::memory_order_relaxed); // reload
            size = current_tail - cached_head_;

            if (size >= capacity_) // full
                return false;
        }

        data_[current_tail & capacity_] = value;

        std::atomic_thread_fence(std::memory_order_release);

        tail_.store(current_tail + 1, std::memory_order_relaxed);
        return true;
    }

    // consumer thread
    bool try_pop(T &value) noexcept(std::is_nothrow_copy_assignable_v<T>)
    {
        size_t current_head = head_.load(std::memory_order_relaxed);

        if (current_head == cached_tail_) // empty?
        {
            cached_tail_ = tail_.load(std::memory_order_relaxed); // reload
            std::atomic_thread_fence(std::memory_order_acquire);

            if (current_head == cached_tail_) // empty
                return false;
        }

        value = data_[current_head & capacity_];
        head_.store(current_head + 1, std::memory_order_relaxed);

        return true;
    }

    // producer thread
    bool full() const noexcept
    {
        size_t current_tail = tail_.load(std::memory_order_relaxed);
        size_t current_head = head_.load(std::memory_order_relaxed);
        return (current_tail - current_head) >= capacity_;
    }

    // consumer thread
    bool empty() const noexcept
    {
        size_t current_head = head_.load(std::memory_order_relaxed);
        size_t current_tail = tail_.load(std::memory_order_relaxed);
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
        size_t current_tail = tail_.load(std::memory_order_relaxed);
        head_.store(current_tail, std::memory_order_relaxed);
        cached_head_ = current_tail;
        cached_tail_ = current_tail;
    }

    size_t capacity() const noexcept { return capacity_; }

private:
    static constexpr size_t capacity_ = N - 1; // also used as bitmask for index
    std::array<T, N> data_{};

    // head and tail increase monotonically
    // Producer cache line: tail_ + cached_head_
    alignas(std::hardware_destructive_interference_size) std::atomic<size_t> tail_{0};
    size_t cached_head_{0};
    // Consumer cache line: head_ + cached_tail_
    alignas(std::hardware_destructive_interference_size) std::atomic<size_t> head_{0};
    size_t cached_tail_{0};
};