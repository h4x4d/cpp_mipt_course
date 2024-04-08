#include <array>
#include <cmath>
#include <exception>
#include <iostream>
#include <iterator>
#include <string>
#include <vector>

template <typename T, typename Allocator = std::allocator<T>>
class Deque {
 public:
  template <bool IsConst>
  class BasicIterator;

  using value_type = T;
  using allocator_type = Allocator;

  using iterator = BasicIterator<false>;
  using const_iterator = BasicIterator<true>;
  using reverse_iterator = std::reverse_iterator<iterator>;
  using const_reverse_iterator = std::reverse_iterator<const_iterator>;

  Deque() = default;

  explicit Deque(const Allocator& allocator) : allocator_(allocator){};

  explicit Deque(size_t count, const Allocator& allocator = Allocator());

  Deque(size_t count, const T& value, const Allocator& allocator = Allocator());

  Deque(Deque&& other);

  Deque(const Deque& other);

  Deque(std::initializer_list<T> init, const Allocator& alloc = Allocator());

  Deque& operator=(const Deque& other);

  Deque& operator=(Deque&& other);

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

  void push_back(T&& value);

  void push_front(const T& value);

  void push_front(T&& value);

  template <typename... Arguments>
  void emplace_back(Arguments&&... args);

  template <typename... Arguments>
  void emplace_front(Arguments&&... args);

  void pop_back();

  void pop_front();

  void insert(iterator iter, const T& value);

  void erase(iterator iter);

  auto get_allocator() { return allocator_; }

  ~Deque() { clear_buckets(); }

 private:
  allocator_type allocator_;
  using allocator_traits = std::allocator_traits<Allocator>;
  using bucket_allocator_type =
      typename allocator_traits::template rebind_alloc<T*>;
  using bucket_traits = std::allocator_traits<bucket_allocator_type>;

  constexpr static size_t kBucketSize = 8;
  using bucket = T*;

  void clear_buckets();

  void swap(Deque<T, Allocator>& other);

  void reallocate_buckets();

  void init_bucket(size_t bucket_index) {
    return init_any_bucket(bucket_index, buckets_, allocator_);
  };

  void init_any_bucket(size_t bucket_index,
                       std::vector<bucket, bucket_allocator_type>& buckets,
                       Allocator& allocator) const;

  size_t get_absolute(size_t index) const {
    return (start_shift_ + index + kBucketSize * buckets_amount_) %
           (kBucketSize * buckets_amount_);
  };

  size_t size_ = 0;
  size_t start_shift_ = 0;
  size_t buckets_amount_ = 0;
  std::vector<bucket, bucket_allocator_type> buckets_;
};

template <typename T, typename Allocator>
Deque<T, Allocator>::Deque(size_t count, const Allocator& allocator)
    : allocator_(allocator) {
  buckets_amount_ = std::ceil(count / static_cast<double>(kBucketSize));
  buckets_.resize(buckets_amount_);
  try {
    for (size_t i = 0; i < buckets_amount_; ++i) {
      init_bucket(i);
      for (size_t j = 0; j < std::min(count, kBucketSize); ++j) {
        allocator_traits::construct(allocator_, buckets_[i] + j);
        size_++;
      }
      count -= std::min(count, kBucketSize);
    }
  } catch (...) {
    clear_buckets();
    throw;
  }
}

template <typename T, typename Allocator>
Deque<T, Allocator>::Deque(std::initializer_list<T> init,
                           const Allocator& allocator)
    : allocator_(allocator) {
  for (auto iter = init.begin(); iter != init.end(); ++iter) {
    emplace_back(std::forward<decltype(*iter)>(*iter));
  }
}

template <typename T, typename Allocator>
Deque<T, Allocator>::Deque(Deque&& other)
    : size_(other.size_),
      start_shift_(other.start_shift_),
      buckets_amount_(other.buckets_amount_),
      allocator_(other.allocator_),
      buckets_(std::move(other.buckets_)) {
  other.buckets_.clear();
  other.size_ = 0;
  other.start_shift_ = 0;
  other.buckets_amount_ = 0;
}

template <typename T, typename Allocator>
void Deque<T, Allocator>::pop_back() {
  allocator_traits::destroy(allocator_, &operator[](size() - 1));
  --size_;
}

template <typename T, typename Allocator>
void Deque<T, Allocator>::erase(Deque::iterator iter) {
  for (; iter + 1 != end(); ++iter) {
    *iter = *(iter + 1);
  }
  pop_back();
}

template <typename T, typename Allocator>
void Deque<T, Allocator>::insert(Deque::iterator iter, const T& value) {
  T last_value = value;
  for (; iter != end(); iter++) {
    std::swap(last_value, *iter);
    iter++;
  }
  push_back(last_value);
}

template <typename T, typename Allocator>
void Deque<T, Allocator>::pop_front() {
  allocator_traits::destroy(allocator_, &operator[](start_shift_));
  (++start_shift_) %= (kBucketSize * buckets_amount_);
  --size_;
}

template <typename T, typename Allocator>
void Deque<T, Allocator>::push_front(T&& value) {
  emplace_front(std::move(value));
}

template <typename T, typename Allocator>
void Deque<T, Allocator>::push_front(const T& value) {
  emplace_front(value);
}

template <typename T, typename Allocator>
void Deque<T, Allocator>::push_back(T&& value) {
  emplace_back(std::move(value));
}

template <typename T, typename Allocator>
void Deque<T, Allocator>::push_back(const T& value) {
  emplace_back(value);
}

template <typename T, typename Allocator>
template <typename... Arguments>
void Deque<T, Allocator>::emplace_back(Arguments&&... args) {
  if (size_ == kBucketSize * buckets_amount_) {
    reallocate_buckets();
  }

  size_t absolute_index = get_absolute(size_);
  size_t bucket = absolute_index / kBucketSize;

  if (buckets_[bucket] == nullptr) {
    init_bucket(bucket);
  }

  allocator_traits::construct(allocator_, &operator[](size_),
                              std::forward<Arguments>(args)...);
  size_++;
}

template <typename T, typename Allocator>
template <typename... Arguments>
void Deque<T, Allocator>::emplace_front(Arguments&&... args) {
  if (size_ >= kBucketSize * buckets_amount_) {
    reallocate_buckets();
  }

  size_t absolute_index = get_absolute(-1);
  size_t bucket = absolute_index / kBucketSize;

  if (buckets_[bucket] == nullptr) {
    init_bucket(bucket);
  }

  allocator_traits::construct(allocator_, &operator[](-1),
                              std::forward<Arguments>(args)...);
  start_shift_ = absolute_index;
  size_++;
}

template <typename T, typename Allocator>
void Deque<T, Allocator>::reallocate_buckets() {
  size_t big_buckets_amount = buckets_amount_ * 2;
  if (buckets_amount_ == 0) {
    big_buckets_amount = 1;
  }

  std::vector<bucket, bucket_allocator_type> created_buckets(
      big_buckets_amount, nullptr, allocator_);
  size_t now = 0;
  size_t start_bucket = start_shift_ / kBucketSize;
  if (start_shift_ % kBucketSize != 0 &&
      size_ == kBucketSize * buckets_amount_) {
    size_t local_ind = start_shift_ % kBucketSize;
    try {
      init_any_bucket(now, created_buckets, allocator_);
      for (; local_ind < kBucketSize; ++local_ind) {
        allocator_traits::construct(
            allocator_, created_buckets[now] + local_ind,
            std::move(buckets_[start_bucket][local_ind]));
      }
    } catch (...) {
      for (size_t j = start_shift_ % kBucketSize; j < local_ind; ++j) {
        allocator_traits::destroy(allocator_, created_buckets[now] + j);
      }
      allocator_traits::deallocate(allocator_, created_buckets[now],
                                   kBucketSize);
      throw;
    }
    ++now;
    created_buckets[buckets_amount_] = buckets_[start_bucket];
    start_shift_ %= kBucketSize;
  } else if (buckets_amount_ != 0) {
    created_buckets[now++] = buckets_[start_bucket];
    start_shift_ %= kBucketSize;
  }

  for (size_t i = start_bucket + 1; i < buckets_amount_; ++i) {
    created_buckets[now++] = buckets_[i];
  }
  for (size_t i = 0; i < start_bucket; ++i) {
    created_buckets[now++] = buckets_[i];
  }
  buckets_amount_ = big_buckets_amount;
  buckets_ = std::move(created_buckets);
}

template <typename T, typename Allocator>
void Deque<T, Allocator>::init_any_bucket(
    size_t bucket_index, std::vector<bucket, bucket_allocator_type>& buckets,
    Allocator& allocator) const {
  if (buckets[bucket_index] == nullptr) {
    buckets[bucket_index] = allocator_traits::allocate(allocator, kBucketSize);
  }
}

template <typename T, typename Allocator>
void Deque<T, Allocator>::clear_buckets() {
  for (size_t j = 0; j < size_; ++j) {
    allocator_traits::destroy(allocator_, (&operator[](j)));
  }

  for (size_t i = 0; i < buckets_amount_; ++i) {
    if (buckets_[i] != nullptr) {
      allocator_traits::deallocate(allocator_, buckets_[i], kBucketSize);
      buckets_[i] = nullptr;
    }
  }
}

template <typename T, typename Allocator>
void Deque<T, Allocator>::swap(Deque& other) {
  std::swap(size_, other.size_);
  std::swap(start_shift_, other.start_shift_);
  std::swap(buckets_amount_, other.buckets_amount_);
  std::swap(buckets_, other.buckets_);
}

template <typename T, typename Allocator>
Deque<T, Allocator>& Deque<T, Allocator>::operator=(Deque&& other) {
  swap(std::move(other));
  return *this;
}

template <typename T, typename Allocator>
Deque<T, Allocator>& Deque<T, Allocator>::operator=(const Deque& other) {
  if ((this) == &other) {
    return *this;
  }

  auto tmp_allocator = allocator_;
  if (allocator_traits::propagate_on_container_copy_assignment::value) {
    tmp_allocator = other.allocator_;
  }
  std::vector<bucket, typename std::allocator_traits<
                          decltype(tmp_allocator)>::template rebind_alloc<T*>>
      replace_buckets(other.buckets_amount_, tmp_allocator);
  size_t left_to_copy = other.size_;
  size_t shift = other.start_shift_;
  size_t created_now = 0;

  try {
    for (size_t i = 0; i < other.buckets_amount_; ++i) {
      if (other.buckets_[i] != nullptr) {
        replace_buckets[i] =
            allocator_traits::allocate(tmp_allocator, kBucketSize);

        for (size_t j = shift; j < std::min(shift + left_to_copy, kBucketSize);
             ++j) {
          allocator_traits::construct(allocator_, replace_buckets[i] + j,
                                      *(other.buckets_[i] + j));
          ++created_now;
        }
        shift = 0;
        left_to_copy -= kBucketSize;

      } else {
        buckets_[i] = nullptr;
      }
    }
  } catch (...) {
    std::swap(size_, created_now);
    std::swap(replace_buckets, buckets_);
    clear_buckets();
    std::swap(size_, created_now);
    std::swap(replace_buckets, buckets_);
    throw;
  }
  clear_buckets();

  buckets_ = std::move(replace_buckets);
  size_ = other.size_;
  start_shift_ = other.start_shift_;
  buckets_amount_ = other.buckets_amount_;
  allocator_ = tmp_allocator;

  return (*this);
}

template <typename T, typename Allocator>
Deque<T, Allocator>::Deque(const Deque& other)
    : size_(other.size_),
      start_shift_(other.start_shift_),
      buckets_amount_(other.buckets_amount_),
      allocator_(allocator_traits::select_on_container_copy_construction(
          other.allocator_)) {
  buckets_.resize(buckets_amount_);

  size_t left_to_copy = size_;
  size_t shift = start_shift_;
  try {
    for (size_t i = 0; i < buckets_amount_; ++i) {
      if (other.buckets_[i] != nullptr) {
        init_bucket(i);
        for (size_t j = shift; j < std::min(shift + left_to_copy, kBucketSize);
             ++j) {
          allocator_traits::construct(allocator_, buckets_[i] + j,
                                      *(other.buckets_[i] + j));
        }
        shift = 0;
        left_to_copy -= kBucketSize;

      } else {
        buckets_[i] = nullptr;
      }
    }
  } catch (...) {
    clear_buckets();
  }
}

template <typename T, typename Allocator>
Deque<T, Allocator>::Deque(size_t count, const T& value,
                           const Allocator& allocator)
    : size_(count),
      allocator_(allocator),
      buckets_amount_(std::ceil(size_ / static_cast<double>(kBucketSize))),
      buckets_(buckets_amount_, nullptr, allocator_) {
  try {
    for (size_t i = 0; i < buckets_amount_; ++i) {
      init_bucket(i);
      for (size_t j = 0; j < std::min(count, kBucketSize); ++j) {
        allocator_traits::construct(allocator_, buckets_[i] + j, value);
      }
      count -= std::min(count, kBucketSize);
    }
  } catch (...) {
    clear_buckets();
    throw;
  }
}

template <typename T, typename Allocator>
T& Deque<T, Allocator>::at(size_t index) {
  if (index >= size()) {
    throw std::out_of_range(
        "Deque: out of range: index (which is " + std::to_string(index) +
        ") >= deque size (which is " + std::to_string(size()) + ")");
    // std::format("Deque: out of range: index (which is '{}') >= deque size
    // (which is '{}')", index, size()));
  }
  return operator[](index);
}

template <typename T, typename Allocator>
const T& Deque<T, Allocator>::at(size_t index) const {
  if (index >= size()) {
    throw std::out_of_range(
        "Deque: out of range: index (which is " + std::to_string(index) +
        ") >= deque size (which is " + std::to_string(size()) + ")");
    // std::format("Deque: out of range: index (which is '{}') >= deque size
    // (which is '{}')", index, size()));
  }
  return operator[](index);
}

template <typename T, typename Allocator>
T& Deque<T, Allocator>::operator[](size_t index) {
  size_t absolute_index = get_absolute(index);
  return *(buckets_[absolute_index / kBucketSize] +
           (absolute_index % kBucketSize));
}

template <typename T, typename Allocator>
const T& Deque<T, Allocator>::operator[](size_t index) const {
  size_t absolute_index = get_absolute(index);
  return *(buckets_[absolute_index / kBucketSize] +
           (absolute_index % kBucketSize));
}

template <typename T, typename Allocator>
template <bool IsConst>
class Deque<T, Allocator>::BasicIterator {
 public:
  using iterator_category = std::random_access_iterator_tag;
  using difference_type = size_t;
  using value_type = std::conditional_t<IsConst, const T, T>;
  using pointer = value_type*;
  using reference = value_type&;

  BasicIterator(Deque<T, Allocator>& deque, size_t index)
      : buckets_(&deque.buckets_.front()),
        index_(index),
        start_shift_(deque.start_shift_),
        buckets_size_(deque.buckets_amount_){};

  BasicIterator(const BasicIterator& iterator)
      : buckets_(iterator.buckets_),
        index_(iterator.index_),
        start_shift_(iterator.start_shift_),
        buckets_size_(iterator.buckets_size_){};

  BasicIterator& operator=(const BasicIterator& iterator) {
    buckets_ = iterator.buckets_;
    index_ = iterator.index_;
    start_shift_ = iterator.start_shift_;
    buckets_size_ = iterator.buckets_size_;
    return (*this);
  };

  reference operator*() const {
    return *(*(buckets_ + index_ / kBucketSize) + (index_ % kBucketSize));
  }

  pointer operator->() const { return &(operator*()); }

  BasicIterator& operator-=(int n) {
    index_ -= n;
    return *this;
  }

  BasicIterator& operator+=(int n) {
    index_ = (index_ + n) % (buckets_size_ * kBucketSize + 1);
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
    return (index_ + start_shift_ + buckets_size_ * kBucketSize + 1) %
           (buckets_size_ * kBucketSize + 1);
  }

  T** buckets_;
  size_t index_;
  size_t& start_shift_;
  size_t& buckets_size_;
};