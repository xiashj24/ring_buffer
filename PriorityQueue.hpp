#pragma once
#include <cstddef>
#include <array>
#include "CircularBuffer.hpp"

/**
 * @ref This implementation is largely based on Data Structure and Algorithm
 * Analysis by Mark Weiss, code snippet:
 * https://users.cs.fiu.edu/~weiss/dsaa_c++4/code/BinaryHeap.h
 * @todo use template parameter to specify min heap or max heap
 * @todo const iterators and erase
 */

/**
 * @brief fixed-size heap based priority queue
 *
 * @tparam T element data type, must be comparable
 * @tparam N size of internal buffer, must be power of 2, while capacity is
 * N-1
 *
 */
template <typename T, size_t N>
class PriorityQueue {
  static_assert((N > 1) && ((N & (N - 1)) == 0), "N must be power of 2");

public:
  PriorityQueue() = default;

  template <size_t M>
  explicit PriorityQueue(const CircularBuffer<T, M>& items) {
    static_assert(M <= N, "can only be constructed from a smaller container");

    size_ = items.size();

    // copy elements to binary tree (represented as an array)
    for (size_t i = 0; i < size_; ++i) {
      binaryTree_[i + 1] = items[i];
    }

    // heapify in-place
    buildHeap();
  }

  size_t capacity() const { return capacity_; }
  size_t size() const { return size_; }
  bool full() const { return size_ == capacity_; }
  bool empty() const { return size_ == 0; }

  void clear() { size_ = 0; }

  /**
   * @brief insert item
   *
   * @param item
   * @return true
   * @return false
   */
  bool push(const T& val) {
    if (size_ == capacity_)
      return false;

    // percolate up
    size_t hole = ++size_;
    binaryTree_[0] = val;

    for (; val < binaryTree_[hole / 2]; hole /= 2) {
      binaryTree_[hole] = binaryTree_[hole / 2];
    }
    binaryTree_[hole] = binaryTree_[0];

    return true;
  }

  T pop() {
    if (empty())
      return T{};

    T result = binaryTree_[1];
    binaryTree_[1] = binaryTree_[size_--];
    // if (!empty())
    percolateDown(1);
    return result;
  }

  // UB if empty
  T top() const { return binaryTree_[1]; }

  // TODO:
  // void increaseKey(iterator it)
  // void decreaseKey(iterator it)
  // void erase(iterator it)

private:
  static constexpr size_t capacity_ = N - 1;
  size_t size_ = 0;
  std::array<T, N> binaryTree_;

  // size_t getParentIndex(size_t index) { return index / 2; }
  // size_t getLeftChildIndex(size_t index) { return index * 2; }
  // size_t getRightChildIndex(size_t index) { return index * 2 + 1; }

  void percolateDown(size_t hole) {
    size_t child;
    T temp = binaryTree_[hole];

    for (; hole * 2 <= size_; hole = child) {
      child = hole * 2;
      if (child != size_ && binaryTree_[child + 1] < binaryTree_[child])
        ++child;
      if (binaryTree_[child] < temp)
        binaryTree_[hole] = binaryTree_[child];
      else
        break;
    }
    binaryTree_[hole] = temp;
  }

  void buildHeap() {
    for (size_t i = size_ / 2; i > 0; --i)
      percolateDown(i);
  }
};
