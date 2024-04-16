#include <cstddef>
#include <utility>
#include <memory>

template <typename T>
struct BaseControlBlock {
  T* ptr = nullptr;
  size_t* shared_count = nullptr;
  size_t* weak_count = nullptr;

  BaseControlBlock(T* ptr, size_t* shared_count, size_t* weak_count)
      : ptr(ptr), shared_count(shared_count), weak_count(weak_count) {};

  operator bool() const { return ptr != nullptr; }

  void add_shared() { ++(*shared_count); }

  void add_weak() { ++(*weak_count); }

  void release_shared();

  void release_weak();

  virtual void no_shared();

  virtual void no_weak_and_shared() { operator delete(this); };

  virtual ~BaseControlBlock() = default;
};

template <typename T>
void BaseControlBlock<T>::no_shared() {
  delete ptr;
  ptr = nullptr;
}

template <typename T>
void BaseControlBlock<T>::release_weak() {
  --(*weak_count);
  if (((*weak_count) == 0) && ((*shared_count) == 0)) {
    no_weak_and_shared();
  }
}

template <typename T>
void BaseControlBlock<T>::release_shared() {
  --(*shared_count);
  if ((*shared_count) == 0) {
    no_shared();
    if ((*weak_count) == 0) {
      no_weak_and_shared();
    }
  }
}

template <typename T>
struct CountControlBlock : BaseControlBlock<T> {
  size_t shared_count_value = 1;
  size_t weak_count_value = 0;

  CountControlBlock(T* ptr, size_t shared_count_val = 1,
                    size_t weak_count_val = 0)
      : shared_count_value(shared_count_val),
        weak_count_value(weak_count_val),
        BaseControlBlock<T>(ptr, &shared_count_value, &weak_count_value) {}
};

template <typename T, typename Deleter>
struct DeleterControlBlock : CountControlBlock<T> {
  Deleter deleter;

  void no_shared() override {
    deleter(BaseControlBlock<T>::ptr);
    BaseControlBlock<T>::ptr = nullptr;
  }

  DeleterControlBlock(T* ptr, Deleter deleter)
      : CountControlBlock<T>(ptr), deleter(deleter) {};
};

template <typename T, typename Deleter, typename Allocator>
struct AllocatorControlBlock : DeleterControlBlock<T, Deleter> {
  Allocator allocator;

  void no_weak_and_shared() override {
    auto block_allocator = (typename std::allocator_traits<Allocator>::template rebind_alloc<
        AllocatorControlBlock>)(allocator);
    std::allocator_traits<decltype(block_allocator)>::destroy(block_allocator, this);
    std::allocator_traits<decltype(block_allocator)>::deallocate(block_allocator, this, 1);
  }

  AllocatorControlBlock(T* ptr, Deleter deleter, Allocator allocator)
      : allocator(allocator), DeleterControlBlock<T, Deleter>(ptr, deleter) {};
};

template <typename T, typename Allocator>
struct AllocateSharedControlBlock :
    AllocatorControlBlock<T,std::default_delete<T>, Allocator> {
  using BaseClass = AllocatorControlBlock<T, std::default_delete<T>,
                                          Allocator>;

  T object;

  void no_shared() override {
    using traits = std::allocator_traits<decltype(BaseClass::allocator)>;
    traits::destroy(BaseClass::allocator, &object);
    CountControlBlock<T>::ptr = nullptr;
  }

  void no_weak_and_shared() override {
    auto block_allocator =
        (typename std::allocator_traits<Allocator>::template rebind_alloc<
            AllocateSharedControlBlock>)(BaseClass::allocator);
    std::allocator_traits<
        decltype(block_allocator)>::deallocate(block_allocator,this, 1);
  }

  template <typename... Args>
  AllocateSharedControlBlock(Allocator allocator, Args&&... args)
      : object(std::forward<Args>(args)...),
        BaseClass(&object, std::default_delete<T>(), allocator) {}
};

template <typename T>
struct MakeSharedControlBlock : CountControlBlock<T> {
  T object;

  void no_shared() {
    object.~T();
  }

  template <typename... Args>
  MakeSharedControlBlock(Args&&... args)
      : object(std::forward<Args>(args)...), CountControlBlock<T>(&object) {}
};

