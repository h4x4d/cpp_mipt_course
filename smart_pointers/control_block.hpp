#include <cstddef>
#include <utility>
#include <memory>

template <typename T>
struct BaseControlBlock {
  T* ptr = nullptr;
  size_t shared_count = 1;
  size_t weak_count = 0;

  BaseControlBlock(T* ptr, size_t shared_count = 1, size_t weak_count = 0)
      : ptr(ptr), shared_count(shared_count), weak_count(weak_count) {};

  operator bool() const { return ptr != nullptr; }

  void add_shared() { ++shared_count; }

  void add_weak() { ++weak_count; }

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
  --weak_count;
  if ((weak_count == 0) && (shared_count == 0)) {
    no_weak_and_shared();
  }
}

template <typename T>
void BaseControlBlock<T>::release_shared() {
  --shared_count;
  if (shared_count == 0) {
    no_shared();
    if (weak_count == 0) {
      no_weak_and_shared();
    }
  }
}

template <typename T, typename Deleter = std::default_delete<T>,
    typename Allocator = std::allocator<T>>
struct AllocatorControlBlock : BaseControlBlock<T> {
  [[no_unique_address]] Deleter deleter;
  [[no_unique_address]] Allocator allocator;

  void no_shared() override {
    deleter(BaseControlBlock<T>::ptr);
    BaseControlBlock<T>::ptr = nullptr;
  }

  void no_weak_and_shared() override {
    auto block_allocator = (typename std::allocator_traits<Allocator>::template rebind_alloc<
        AllocatorControlBlock>)(allocator);
    std::allocator_traits<decltype(block_allocator)>::destroy(block_allocator, this);
    std::allocator_traits<decltype(block_allocator)>::deallocate(block_allocator, this, 1);
  }

  AllocatorControlBlock(T* ptr, Deleter deleter = std::default_delete<T>(),
                        Allocator allocator = std::allocator<T>())
      :  deleter(deleter), allocator(allocator), BaseControlBlock<T>(ptr) {};
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
    BaseControlBlock<T>::ptr = nullptr;
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

