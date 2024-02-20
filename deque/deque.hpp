#include <array>
#include <cmath>
#include <exception>
#include <iostream>
#include <iterator>
#include <string>
#include <vector>

template <typename T>
class Deque {
 public:
  template <bool IsConst>
  class BasicIterator;

  using iterator = BasicIterator<false>;
  using const_iterator = BasicIterator<true>;
  using reverse_iterator = std::reverse_iterator<iterator>;
  using const_reverse_iterator = std::reverse_iterator<const_iterator>;

  Deque() = default;

  explicit Deque(size_t count) : Deque(count, T()){};

  Deque(size_t count, const T& value);

  Deque(const Deque& other);

  Deque& operator=(const Deque& other);

  size_t size() const { return size_; };

  bool empty() const noexcept { return size_ == 0; };

  T& operator[](size_t index);

  const T& operator[](size_t index) const;

  T& at(size_t index);

  const T& at(size_t index) const;

  iterator begin() { return iterator(*this, start_shift_); }

  const_iterator cbegin() { return const_iterator((*this), start_shift_); }

  reverse_iterator rbegin() { return reverse_iterator(end()); }

  const_reverse_iterator crbegin() { return const_reverse_iterator(cend()); }

  iterator end() {
    return iterator(
        (*this), (start_shift_ + size_) % (kBucketSize * buckets_amount_ + 1));
  }

  const_iterator cend() {
    return const_iterator(
        (*this), (start_shift_ + size_) % (kBucketSize * buckets_amount_ + 1));
  }

  reverse_iterator rend() { return reverse_iterator(begin()); }

  const_reverse_iterator crend() { return const_reverse_iterator(cbegin()); }

  void push_back(const T& value);

  void push_front(const T& value);

  void pop_back();

  void pop_front();

  void insert(iterator iter, const T& value);

  void erase(iterator iter);

  ~Deque() { clear_buckets(); }

 private:
  constexpr static size_t kBucketSize = 8;
  using bucket = T*;

  void clear_buckets();

  void swap(Deque<T>& other);

  void reallocate_buckets();

  void init_bucket(size_t bucket_index) {
    return init_any_bucket(bucket_index, buckets_);
  };

  void init_any_bucket(size_t bucket_index, std::vector<bucket>& buckets) const;

  size_t get_absolute(size_t index) const {
    return (start_shift_ + index + kBucketSize * buckets_amount_) %
           (kBucketSize * buckets_amount_);
  };

  std::vector<bucket> buckets_;
  size_t buckets_amount_ = 0;
  size_t size_ = 0;
  size_t start_shift_ = 0;
};

template <typename T>
void Deque<T>::pop_back() {
  operator[](size() - 1).~T();
  --size_;
}

template <typename T>
void Deque<T>::erase(Deque::iterator iter) {
  for (; iter + 1 != end(); ++iter) {
    *iter = *(iter + 1);
  }
  pop_back();
}

template <typename T>
void Deque<T>::insert(Deque::iterator iter, const T& value) {
  T last_value = value;
  while (iter != end()) {
    std::swap(last_value, *iter);
    iter++;
  }
  push_back(last_value);
}

template <typename T>
void Deque<T>::pop_front() {
  operator[](start_shift_).~T();
  start_shift_ = (start_shift_ + 1) % (kBucketSize * buckets_amount_);
  --size_;
}

template <typename T>
void Deque<T>::push_front(const T& value) {
  if (size_ == kBucketSize * buckets_amount_) {
    reallocate_buckets();
  }

  size_t absolute_index = get_absolute(-1);
  size_t bucket = absolute_index / kBucketSize;

  if (buckets_[bucket] == nullptr) {
    init_bucket(bucket);
  }
  new (&operator[](-1)) T(value);
  start_shift_ = absolute_index;
  size_++;
}

template <typename T>
void Deque<T>::push_back(const T& value) {
  if (size_ >= kBucketSize * buckets_amount_) {
    reallocate_buckets();
  }

  size_t absolute_index = get_absolute(size_);
  size_t bucket = absolute_index / kBucketSize;

  if (buckets_[bucket] == nullptr) {
    init_bucket(bucket);
  }

  new (&operator[](size())) T(value);
  size_++;
}

template <typename T>
void Deque<T>::reallocate_buckets() {
  size_t new_buckets_amount = buckets_amount_ * 2;
  if (buckets_amount_ == 0) {
    new_buckets_amount = 1;
  }

  std::vector<bucket> new_buckets(new_buckets_amount, nullptr);
  size_t now = 0;
  size_t start_bucket = start_shift_ / kBucketSize;
  if (start_shift_ % kBucketSize != 0 &&
      size_ == kBucketSize * buckets_amount_) {
    try {
      init_any_bucket(now, new_buckets);
      std::uninitialized_copy(buckets_[start_bucket] + start_shift_ % kBucketSize,
                              buckets_[start_bucket] + kBucketSize,
                              new_buckets[now++] + start_shift_ % kBucketSize);
    } catch (...) {
      delete[] new_buckets[now - 1];
      throw;
    }
    new_buckets[buckets_amount_] = buckets_[start_bucket];
    start_shift_ %= kBucketSize;
  } else if (buckets_amount_ != 0) {
    new_buckets[now++] = buckets_[start_bucket];
    start_shift_ %= kBucketSize;
  }

  for (size_t i = start_bucket + 1; i < buckets_amount_; ++i) {
    new_buckets[now++] = buckets_[i];
  }
  for (size_t i = 0; i < start_bucket; ++i) {
    new_buckets[now++] = buckets_[i];
  }
  buckets_amount_ = new_buckets_amount;
  buckets_ = new_buckets;
}

template <typename T>
void Deque<T>::init_any_bucket(size_t bucket_index,
                               std::vector<bucket>& buckets) const {
  if (buckets[bucket_index] == nullptr) {
    buckets[bucket_index] =
        reinterpret_cast<T*>(new std::byte[sizeof(T) * kBucketSize]);
  }
}

template <typename T>
void Deque<T>::clear_buckets() {
  for (size_t i = 0; i < buckets_amount_; ++i) {
    if (buckets_[i] != nullptr) {
      auto* source = reinterpret_cast<std::byte*>(buckets_[i]);
      delete[] source;
      buckets_[i] = nullptr;
    }
  }
}

template <typename T>
void Deque<T>::swap(Deque& other) {
  std::swap(size_, other.size_);
  std::swap(start_shift_, other.start_shift_);
  std::swap(buckets_amount_, other.buckets_amount_);
  std::swap(buckets_, other.buckets_);
}

template <typename T>
Deque<T>& Deque<T>::operator=(const Deque& other) {
  if ((this) == &other) {
    return *this;
  }
  Deque<T> tmp(other);
  swap(tmp);
  return (*this);
}

template <typename T>
Deque<T>::Deque(const Deque& other)
    : size_(other.size_),
      start_shift_(other.start_shift_),
      buckets_amount_(other.buckets_amount_) {
  buckets_.resize(buckets_amount_);

  for (size_t i = 0; i < buckets_amount_; ++i) {
    if (other.buckets_[i] != nullptr) {
      init_bucket(i);
      try {
        std::uninitialized_copy(other.buckets_[i],
                                other.buckets_[i] + kBucketSize, buckets_[i]);
      } catch (...) {
        clear_buckets();
      }
    } else {
      buckets_[i] = nullptr;
    }
  }
}

template <typename T>
Deque<T>::Deque(size_t count, const T& value) : size_(count) {
  buckets_amount_ = std::ceil(size_ / static_cast<double>(kBucketSize));
  buckets_.resize(buckets_amount_);
  for (size_t i = 0; i < buckets_amount_; ++i) {
    try {
      init_bucket(i);
      for (size_t j = 0; j < kBucketSize; ++j) {
        new (buckets_[i] + j) T(value);
      }
      count -= std::min(count, kBucketSize);
    } catch (...) {
      clear_buckets();
      throw;
    }
  }
}

template <typename T>
T& Deque<T>::at(size_t index) {
  if (index >= size()) {
    throw std::out_of_range(
        "Deque: out of range: index (which is " + std::to_string(index) +
        ") >= deque size (which is " + std::to_string(size()) + ")");
    // std::format("Deque: out of range: index (which is '{}') >= deque size
    // (which is '{}')", index, size()));
  }
  return operator[](index);
}

template <typename T>
const T& Deque<T>::at(size_t index) const {
  if (index >= size()) {
    throw std::out_of_range(
        "Deque: out of range: index (which is " + std::to_string(index) +
        ") >= deque size (which is " + std::to_string(size()) + ")");
    // std::format("Deque: out of range: index (which is '{}') >= deque size
    // (which is '{}')", index, size()));
  }
  return operator[](index);
}

template <typename T>
T& Deque<T>::operator[](size_t index) {
  size_t absolute_index = get_absolute(index);
  return *(buckets_[absolute_index / kBucketSize] +
           (absolute_index % kBucketSize));
}

template <typename T>
const T& Deque<T>::operator[](size_t index) const {
  size_t absolute_index = get_absolute(index);
  return *(buckets_[absolute_index / kBucketSize] +
           (absolute_index % kBucketSize));
}

template <typename T>
template <bool IsConst>
class Deque<T>::BasicIterator {
 public:
  using iterator_category = std::random_access_iterator_tag;
  using difference_type = size_t;
  using value_type = std::conditional_t<IsConst, const T, T>;
  using pointer = value_type*;
  using reference = value_type&;

  BasicIterator(Deque<T>& deque, size_t index) : deque_(deque), index_(index){};

  BasicIterator(const BasicIterator& iterator)
      : deque_(iterator.deque_), index_(iterator.index_){};

  BasicIterator& operator=(const BasicIterator& iterator) {
    deque_ = iterator.deque_;
    index_ = iterator.index_;
    return (*this);
  };

  reference operator*() const {
    return *(deque_.buckets_[index_ / kBucketSize] + (index_ % kBucketSize));
  }

  pointer operator->() const { return &(operator*()); }

  BasicIterator& operator-=(int n) {
    index_ -= n;
    return *this;
  }

  BasicIterator& operator+=(int n) {
    index_ = (index_ + n) % (deque_.buckets_amount_ * kBucketSize + 1);
    return *this;
  }

  BasicIterator operator-(int n) const {
    auto copy = *this;
    copy -= n;

    return copy;
  }

  BasicIterator operator+(int n) const {
    auto copy = *this;
    copy += n;

    return copy;
  }

  BasicIterator operator++(int) {
    BasicIterator tmp = *this;
    ++(*this);
    return tmp;
  }

  BasicIterator& operator++() {
    index_ += 1;
    return *this;
  }

  BasicIterator operator--(int) {
    BasicIterator tmp = *this;
    --(*this);
    return tmp;
  }

  BasicIterator& operator--() {
    --index_;
    return *this;
  }

  bool operator==(const BasicIterator& other) const {
    return index_ == other.index_;
  }

  bool operator!=(const BasicIterator& other) const {
    return !((*this) == other);
  }

  bool operator<(const BasicIterator& other) const {
    return get_relative() < other.get_relative();
  }

  bool operator>(const BasicIterator& other) const { return other < (*this); }

  bool operator>=(const BasicIterator& other) const {
    return !(other > (*this));
  }

  bool operator<=(const BasicIterator& other) const {
    return !(other < (*this));
  }

  difference_type operator-(BasicIterator other) {
    return index_ - other.index_;
  }

 private:
  size_t get_relative() const {
    return (index_ - deque_.start_shift_ +
            deque_.buckets_amount_ * kBucketSize + 1) %
           (deque_.buckets_amount_ * kBucketSize + 1);
  }

  Deque<T>& deque_;
  size_t index_;
};