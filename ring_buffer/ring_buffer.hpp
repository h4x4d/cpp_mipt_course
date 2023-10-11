#include <cstddef>
#include <vector>

class RingBuffer {
 public:
  explicit RingBuffer(size_t capacity) : kCapacity(capacity) {
    queue_.reserve(kCapacity);
  }

  size_t Size() const { return queue_.size(); }

  bool Empty() const { return queue_.empty(); }

  bool TryPush(int element) {
    if (queue_.size() < kCapacity) {
      queue_.push_back(element);
      return true;
    }
    return false;
  }

  bool TryPop(int* element) {
    if (!queue_.empty()) {
      *element = queue_[0];
      queue_.erase(queue_.begin());
      return true;
    }
    return false;
  }

 private:
  std::vector<int> queue_;
  const size_t kCapacity;
};
