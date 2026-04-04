#pragma once
#include <array>
#include <cstddef>
#include <cstdint>
#include <utility>
#include "RandomGenerator.hpp"

/**
 *
 * @brief fixed-size single-producer, single-consumer lock-free circular buffer
 * (aka FIFO queue)
 * @author Shijie Xia
 * @date 2025/8/26
 *
 * @note adapted from code snippets of Making Embedded Systems by Elecia White
 * if you have access to c++17, consider using std::pmr::vector
 *
 * @todo
 * - unit test for push_back_overwrite_oldest()
 * - constructor with initializer list {}
 * - rvalue move specialization (&&)
 */

/**
 * @brief fixed-size single-producer, single-consumer lock-free circular buffer
 * (aka FIFO queue)
 *
 * @tparam T data type for the internal buffer
 * @tparam N size of the internal buffer, has to be power of 2
 *
 * @note "waste" one slot to distinguish between full and empty, so capacity()
 * is actually N-1.
 *
 * By default, push_back() does not overwrite the earliest element when the
 * buffer is full; use push_back_overwrite_oldest() to overwrite the oldest
 * element when full.
 *
 */
template <typename T, size_t N>
class CircularBuffer {
  static_assert((N > 1) && ((N & (N - 1)) == 0), "N must be power of 2");

public:
  CircularBuffer() noexcept : head_{0}, tail_{0} {}

  size_t size() const noexcept { return ((head_ - tail_) & capacity_); }
  size_t capacity() const noexcept { return capacity_; }

  bool full() const noexcept { return size() == capacity_; }
  bool empty() const noexcept { return head_ == tail_; }

  void clear() noexcept {
    head_ = 0;
    tail_ = 0;
    // head_ = tail_;
  }

  // swap by moving individual elements
  void swap_with(CircularBuffer& other) noexcept {
    if (this == &other)
      return;  // self-swap guard

    size_t size_this = this->size();
    size_t size_other = other.size();

    while (true) {
      T temp;
      bool success = true;
      if (size_this > 0) {
        temp = this->pop_front();
        success = other.push_back(temp);
        --size_this;
      }
      if (size_other > 0) {
        this->push_back(other.pop_front());  // safe, this has been popped once
        --size_other;
      }

      if (!success) {  // try again
        success = other.push_back(temp);
      }

      // jassert(success);

      if (size_this == 0 && size_other == 0)
        break;
    }
  }

  friend void swap(CircularBuffer& a, CircularBuffer& b) noexcept {
    a.swap_with(b);
  }

  // return false if no available slot, do not overwrite oldest element
  bool push_back(const T& val) noexcept {
    if (full()) {
      return false;
    }

    buffer_[head_] = val;
    head_ = (head_ + 1) & capacity_;
    return true;
  }

  bool push_back(T&& val) noexcept {
    if (full()) {
      return false;
    }

    buffer_[head_] = std::move(val);
    head_ = (head_ + 1) & capacity_;
    return true;
  }

  // push back version that overwrite the oldest element when the buffer is
  // full
  // warning: unsafe to use in multithread program
  void push_back_overwrite_oldest(const T& val) noexcept {
    buffer_[head_] = val;

    size_t next = (head_ + 1) & capacity_;

    // if full overwrite oldest
    if (next == tail_) {
      tail_ = (tail_ + 1) & capacity_;
    }

    head_ = next;
  }

  T pop_front() noexcept {
    if (empty()) {
      return T{};
    }

    auto val = buffer_[tail_];
    tail_ = (tail_ + 1) & capacity_;

    return val;
  }

  T& front() { return buffer_[tail_]; }
  const T& front() const { return buffer_[tail_]; }
  T& back() { return buffer_[(head_ - 1) & capacity_]; }
  const T& back() const { return buffer_[(head_ - 1) & capacity_]; }

  // note: no bounds checking, idx must < size()
  T& operator[](size_t idx) noexcept {
    return buffer_[(tail_ + idx) & capacity_];
  }

  const T& operator[](size_t idx) const noexcept {
    return buffer_[(tail_ + idx) & capacity_];
  }

  // range-for loop will loop from tail to head (earliest to latest)
  class iterator {
  public:
    using iterator_category = std::bidirectional_iterator_tag;
    using value_type = T;
    using difference_type = std::ptrdiff_t;
    using pointer = T*;
    using reference = T&;

    iterator() noexcept : cbuf_(nullptr), pos_(0) {}
    iterator(CircularBuffer* cbuf, size_t pos) noexcept
        : cbuf_(cbuf), pos_(pos) {}

    T& operator*() noexcept { return cbuf_->buffer_[pos_]; }
    const T& operator*() const noexcept { return cbuf_->buffer_[pos_]; }

    T* operator->() noexcept { return &cbuf_->buffer_[pos_]; }
    const T* operator->() const noexcept { return &cbuf_->buffer_[pos_]; }

    iterator& operator++() noexcept {
      pos_ = (pos_ + 1) & capacity_;
      return *this;
    }

    iterator& operator--() noexcept {
      pos_ = (pos_ - 1) & capacity_;
      return *this;
    }

    bool operator!=(const iterator& other) const noexcept {
      return cbuf_ != other.cbuf_ || pos_ != other.pos_;
    }
    bool operator==(const iterator& other) const noexcept {
      return cbuf_ == other.cbuf_ && pos_ == other.pos_;
    }

  private:
    CircularBuffer* cbuf_;
    size_t pos_;
  };

  class const_iterator {
  public:
    using iterator_category = std::bidirectional_iterator_tag;
    using value_type = T;
    using difference_type = std::ptrdiff_t;
    using pointer = const T*;
    using reference = const T&;

    const_iterator() noexcept : cbuf_(nullptr), pos_(0) {}
    const_iterator(const CircularBuffer* cbuf, size_t pos) noexcept
        : cbuf_(cbuf), pos_(pos) {}

    const T& operator*() const noexcept { return cbuf_->buffer_[pos_]; }

    const T* operator->() const noexcept { return &cbuf_->buffer_[pos_]; }

    const_iterator& operator++() noexcept {
      pos_ = (pos_ + 1) & capacity_;
      return *this;
    }

    const_iterator& operator--() noexcept {
      pos_ = (pos_ - 1) & capacity_;
      return *this;
    }

    bool operator!=(const const_iterator& other) const noexcept {
      return cbuf_ != other.cbuf_ || pos_ != other.pos_;
    }
    bool operator==(const const_iterator& other) const noexcept {
      return cbuf_ == other.cbuf_ && pos_ == other.pos_;
    }

  private:
    const CircularBuffer* cbuf_;
    size_t pos_;
  };

  iterator begin() noexcept { return iterator(this, tail_); }
  iterator end() noexcept { return iterator(this, head_); }
  const_iterator begin() const noexcept { return const_iterator(this, tail_); }
  const_iterator end() const noexcept { return const_iterator(this, head_); }
  const_iterator cbegin() const noexcept { return begin(); }
  const_iterator cend() const noexcept { return end(); }

  /**
   * @brief erase one element in the buffer, shifting subsequencet elements
   *
   * @param pos
   * @return If pos refers to the last element, then the end() iterator is
   * returned. If *pos == back() prior to removal, then the updated end()
   * iterator is returned. Else return the element that followed the erased one
   */
  iterator erase(iterator pos) {
    if (pos == end())
      return end();

    iterator result = pos;
    iterator next = pos;
    ++next;

    while (next != end()) {
      *pos = *next;
      ++pos;
      ++next;
    }

    head_ = (head_ - 1) & capacity_;
    return result;
  }

  /**
   * @brief insert one element in the buffer, shifting subsequencet elements
   *
   * @param pos
   * @param val
   * @return end() if full, else return the iterator of inserted element
   * @note this method works but is not recommeded, it you use it heavily you
   * should consider other data structures
   */
  // iterator insert(iterator pos, const T& val) {
  //   if (full())
  //     return end();

  //   if (pos == end()) {
  //     push_back(val);
  //     return pos;
  //   }

  //   push_back(val);

  //   iterator it = end();
  //   --it;  // back()
  //   iterator prev = it;
  //   --prev;

  //   while (prev != pos) {
  //     *it = *prev;
  //     --it;
  //     --prev;
  //   }

  //   *pos = val;
  //   return pos;
  // }

  void shuffle(RandomGeneratorBase& rng) noexcept {
    for (size_t i = size() - 1; i > 0; --i) {
      size_t j = static_cast<size_t>(
          rng.nextInt(static_cast<uint32_t>(i + 1)));  // 0 ≤ j ≤ i
      std::swap((*this)[i], (*this)[j]);
    }
  }

private:
  static constexpr size_t capacity_ = N - 1;
  std::array<T, N> buffer_;

  size_t head_;
  size_t tail_;
};
