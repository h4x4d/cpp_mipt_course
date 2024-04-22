#include "control_block.hpp"

template <typename T>
class SharedPtr {
 public:
  SharedPtr() = default;

  SharedPtr(std::nullptr_t){};

  template <typename Y>
    requires std::is_convertible_v<Y*, T*>
  SharedPtr(Y* ptr) : ptr_(ptr), control_(new BaseControlBlock<T>(ptr_)) {}

  template <typename Y, typename Deleter>
    requires std::is_convertible_v<Y*, T*>
  SharedPtr(Y* ptr, Deleter deleter)
      : ptr_(ptr),
        control_(new AllocatorControlBlock<T, Deleter>(ptr_, deleter)) {}

  template <typename Y, typename Deleter, typename Allocator>
    requires std::is_convertible_v<Y*, T*>
  SharedPtr(Y* ptr, Deleter deleter, Allocator allocator);

  template <typename Y>
    requires std::is_convertible_v<Y*, T*>
  SharedPtr(const SharedPtr<Y>& other);

  SharedPtr(const SharedPtr& other);

  template <typename Y>
    requires std::is_convertible_v<Y*, T*>
  SharedPtr(SharedPtr<Y>&& other);

  SharedPtr(SharedPtr&& other);

  template <typename Y>
    requires std::is_convertible_v<Y*, T*>
  SharedPtr& operator=(const SharedPtr<Y>& other);

  SharedPtr& operator=(const SharedPtr& other);

  template <typename Y>
    requires std::is_convertible_v<Y*, T*>
  SharedPtr& operator=(SharedPtr<Y>&& other);

  SharedPtr& operator=(SharedPtr&& other);

  T* get() const { return ptr_; };

  void swap(SharedPtr<T>& other);

  size_t use_count() const { return control_ ? (control_->shared_count) : 0; };

  T* operator->() const { return ptr_; };

  T& operator*() const { return *ptr_; };

  void reset() { SharedPtr().swap(*this); };

  ~SharedPtr();

 private:
  SharedPtr(BaseControlBlock<T>* block) : ptr_(block->ptr), control_(block){};

  template <typename U, typename Allocator, typename... Args>
  friend SharedPtr<U> AllocateShared(const Allocator& allocator,
                                     Args&&... args);

  template <typename U, typename... Args>
  friend SharedPtr<U> MakeShared(Args&&... args);

  template <typename X>
  friend class SharedPtr;

  template <typename U>
  friend class WeakPtr;

  T* ptr_ = nullptr;
  BaseControlBlock<T>* control_ = nullptr;
};

template <typename T>
SharedPtr<T>::~SharedPtr() {
  if (control_) {
    control_->release_shared();
  }
}

template <typename T>
template <typename Y>
  requires std::is_convertible_v<Y*, T*>
SharedPtr<T>& SharedPtr<T>::operator=(const SharedPtr<Y>& other) {
  if (ptr_ != dynamic_cast<T*>(other.ptr_)) {
    SharedPtr(other).swap(*this);
  }
  return *this;
}

template <typename T>
SharedPtr<T>& SharedPtr<T>::operator=(const SharedPtr& other) {
  if (ptr_ != other.ptr_) {
    SharedPtr(other).swap(*this);
  }
  return *this;
}

template <typename T>
template <typename Y>
  requires std::is_convertible_v<Y*, T*>
SharedPtr<T>& SharedPtr<T>::operator=(SharedPtr<Y>&& other) {
  if (ptr_ != dynamic_cast<T*>(other.ptr_)) {
    SharedPtr(std::move(other)).swap(*this);
  }
  return *this;
}

template <typename T>
SharedPtr<T>& SharedPtr<T>::operator=(SharedPtr&& other) {
  if (ptr_ != other.ptr_) {
    SharedPtr(std::move(other)).swap(*this);
  }
  return *this;
}

template <typename T>
void SharedPtr<T>::swap(SharedPtr<T>& other) {
  std::swap(ptr_, other.ptr_);
  std::swap(control_, other.control_);
}

template <typename T>
template <typename Y>
  requires std::is_convertible_v<Y*, T*>
SharedPtr<T>::SharedPtr(SharedPtr<Y>&& other)
    : ptr_(other.ptr_),
      control_(dynamic_cast<BaseControlBlock<T>*>(other.control_)) {
  other.ptr_ = nullptr;
  other.control_ = nullptr;
}

template <typename T>
SharedPtr<T>::SharedPtr(SharedPtr&& other)
    : ptr_(other.ptr_), control_(other.control_) {
  other.ptr_ = nullptr;
  other.control_ = nullptr;
}

template <typename T>
template <typename Y>
  requires std::is_convertible_v<Y*, T*>
SharedPtr<T>::SharedPtr(const SharedPtr<Y>& other)
    : ptr_(other.ptr_),
      control_(dynamic_cast<BaseControlBlock<T>*>(other.control_)) {
  if (control_ && *control_) {
    control_->add_shared();
  }
}

template <typename T>
SharedPtr<T>::SharedPtr(const SharedPtr& other)
    : ptr_(other.ptr_), control_(other.control_) {
  if (*control_) {
    control_->add_shared();
  }
}

template <typename T>
template <typename Y, typename Deleter, typename Allocator>
  requires std::is_convertible_v<Y*, T*>
SharedPtr<T>::SharedPtr(Y* ptr, Deleter deleter, Allocator allocator)
    : ptr_(ptr) {
  auto new_allocator =
      typename std::allocator_traits<Allocator>::template rebind_alloc<
          AllocatorControlBlock<Y, Deleter, Allocator>>(allocator);
  using NewTraits = std::allocator_traits<decltype(new_allocator)>;
  auto control = NewTraits::allocate(new_allocator, 1);
  control_ = control;
  NewTraits::construct(new_allocator, control, ptr, deleter, allocator);
}

template <typename T>
class WeakPtr {
 public:
  WeakPtr() = default;

  WeakPtr(const WeakPtr& other) : control_(other.control_) {
    control_->add_weak();
  };

  WeakPtr(WeakPtr&& other) : control_(std::move(other.control_)) {
    other.control_ = nullptr;
  };

  WeakPtr(const SharedPtr<T>& other) : control_(other.control_) {
    control_->add_weak();
  };

  WeakPtr& operator=(const WeakPtr& other);

  WeakPtr& operator=(WeakPtr&& other);

  bool expired() const { return (control_->shared_count) == 0; };

  SharedPtr<T> lock() const {
    return expired() ? nullptr : SharedPtr<T>(control_);
  };

  void swap(WeakPtr& other) { std::swap(control_, other.control_); }

  ~WeakPtr() { control_->release_weak(); };

 private:
  BaseControlBlock<T>* control_ = nullptr;
};

template <typename T>
WeakPtr<T>& WeakPtr<T>::operator=(const WeakPtr& other) {
  if (control_ != other.control_) {
    WeakPtr(other).swap(*this);
  }
  return *this;
}

template <typename T>
WeakPtr<T>& WeakPtr<T>::operator=(WeakPtr&& other) {
  if (control_ != other.control_) {
    WeakPtr(std::move(other)).swap(*this);
  }
  return *this;
}

template <typename T, typename Allocator, typename... Args>
SharedPtr<T> AllocateShared(const Allocator& allocator, Args&&... args) {
  using Traits = std::allocator_traits<Allocator>;
  using BlockAlloc =
      Traits::template rebind_alloc<AllocateSharedControlBlock<T, Allocator>>;
  BlockAlloc block_alloc(allocator);
  using BlockTraits = std::allocator_traits<decltype(block_alloc)>;

  auto* block = BlockTraits::allocate(block_alloc, 1);
  BlockTraits::construct(block_alloc, block, allocator,
                         std::forward<Args>(args)...);

  return SharedPtr<T>(dynamic_cast<BaseControlBlock<T>*>(block));
}

template <typename T, typename... Args>
SharedPtr<T> MakeShared(Args&&... args) {
  return AllocateShared<T, std::allocator<T>, Args...>(
      std::allocator<T>(), std::forward<Args>(args)...);
}