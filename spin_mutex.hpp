#include <atomic>

/**
 * @brief spin lock, can be used as a drop-in replacement for std::mutex
 * @ref https://timur.audio/using-locks-in-real-time-audio-processing-safely
 */

class spin_mutex
{
    void lock() noexcept
    {
        if (!try_lock())
            /* spin */;
    }

    bool try_lock() noexcept
    {
        return !flag.test_and_set(std::memory_order_acquire);
    }

    void unlock() noexcept
    {
        flag.clear(std::memory_order_release);
    }

private:
    std::atomic_flag flag = ATOMIC_FLAG_INIT;
};