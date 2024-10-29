#include <cstddef>

template <typename T>
class RingBuffer {
 public:
  explicit RingBuffer(size_t capacity)
      : buffer_(new T[capacity]), kCapacity(capacity) {}

  size_t Size() const { return size_; }

  bool Empty() const { return size_ == 0; }

  bool TryPush(const T& element) {
    if (size_ < kCapacity) {
      buffer_[(shift_ + size_) % kCapacity] = element;
      ++size_;
      return true;
    }
    return false;
  }

  bool TryPop(T* element) {
    if (!Empty()) {
      *element = buffer_[shift_];
      shift_ = (shift_ + 1) % kCapacity;
      --size_;
      return true;
    }
    return false;
  }

  ~RingBuffer() {
    delete[] buffer_;
  }

 private:
  T* buffer_;
  size_t shift_ = 0;
  size_t size_ = 0;
  const size_t kCapacity;
};
