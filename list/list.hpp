#include <cstdint>
#include <initializer_list>
#include <iterator>

template <typename T, typename Allocator = std::allocator<T>>
class List {
 public:
  template <bool IsConst>
  class BasicIterator;

  using value_type = T;
  using allocator_type = Allocator;

  using iterator = BasicIterator<false>;
  using const_iterator = BasicIterator<true>;
  using reverse_iterator = std::reverse_iterator<iterator>;
  using const_reverse_iterator = std::reverse_iterator<const_iterator>;

  List() = default;

  explicit List(size_t count, const Allocator& allocator = Allocator());

  List(size_t count, const T& value, const Allocator& allocator = Allocator());

  List(const List& other);

  List(std::initializer_list<T> init, const Allocator& allocator = Allocator());

  List& operator=(const List& other);

  bool empty() const { return size_ == 0; };

  size_t size() const { return size_; };

  void push_front(const T& value);

  void push_back(const T& value);

  void pop_front();

  void pop_back();

  iterator begin() { return iterator(fake_node_.next, 0); };

  const_iterator begin() const { return cbegin(); };

  const_iterator cbegin() const { return const_iterator(fake_node_.next, 0); };

  reverse_iterator rbegin() const { return reverse_iterator(end()); };

  const_reverse_iterator crbegin() const {
    return const_reverse_iterator(cend());
  };

  iterator end() { return iterator(&fake_node_, size_); };

  const_iterator end() const { return cend(); };

  const_iterator cend() const { return const_iterator(&fake_node_, size_); };

  reverse_iterator rend() const { return reverse_iterator(begin()); };

  const_reverse_iterator crend() const {
    return const_reverse_iterator(cbegin());
  };

  T& front() { return fake_node_.next->value; };

  const T& front() const { return fake_node_.next->value; };

  T& back() { return fake_node_.prev->value; };

  const T& back() const { return fake_node_.prev->value; };

  auto get_allocator() { return allocator_; }

  ~List() { clear_list(); };

 private:
  void clear_list();

  struct BaseNode;

  struct Node;

  BaseNode fake_node_;

  size_t size_ = 0;

  using allocator_traits = std::allocator_traits<Allocator>;
  using node_allocator = allocator_traits::template rebind_alloc<Node>;
  using node_allocator_traits = std::allocator_traits<node_allocator>;

  node_allocator allocator_;
};

template <typename T, typename Allocator>
List<T, Allocator>::List(size_t count, const Allocator& allocator)
    : allocator_(allocator) {
  try {
    for (size_t i = 0; i < count; ++i) {
      Node* created_element = node_allocator_traits::allocate(allocator_, 1);
      try {
        node_allocator_traits::construct(allocator_, created_element);
      } catch (...) {
        node_allocator_traits::deallocate(allocator_, created_element, 1);
        throw;
      }
      created_element->prev = fake_node_.prev;
      created_element->next = &fake_node_;

      fake_node_.prev->next = created_element;
      fake_node_.prev = created_element;
      if (size_ == 0) {
        fake_node_.next = created_element;
      }
      size_++;
    }
  } catch (...) {
    clear_list();
    throw;
  }
}

template <typename T, typename Allocator>
void List<T, Allocator>::pop_front() {
  BaseNode* element = fake_node_.next;
  fake_node_.next = element->next;
  element->next->prev = &fake_node_;

  node_allocator_traits::destroy(allocator_, static_cast<Node*>(element));
  node_allocator_traits::deallocate(allocator_, static_cast<Node*>(element), 1);
  size_--;
}

template <typename T, typename Allocator>
void List<T, Allocator>::pop_back() {
  BaseNode* element = fake_node_.prev;
  fake_node_.prev = element->prev;
  element->prev->next = &fake_node_;

  node_allocator_traits::destroy(allocator_, static_cast<Node*>(element));
  node_allocator_traits::deallocate(allocator_, static_cast<Node*>(element), 1);
  size_--;
}

template <typename T, typename Allocator>
void List<T, Allocator>::push_back(const T& value) {
  Node* created_element = node_allocator_traits::allocate(allocator_, 1);
  try {
    node_allocator_traits::construct(allocator_, created_element, value,
                                     fake_node_.prev, &fake_node_);
  } catch (...) {
    node_allocator_traits::deallocate(allocator_, created_element, 1);
    throw;
  }

  fake_node_.prev->next = created_element;
  fake_node_.prev = created_element;
  if (size_ == 0) {
    fake_node_.next = created_element;
  }
  size_++;
}

template <typename T, typename Allocator>
void List<T, Allocator>::push_front(const T& value) {
  Node* created_element = node_allocator_traits::allocate(allocator_, 1);
  created_element->value = value;
  created_element->prev = &fake_node_;
  created_element->next = fake_node_.next;

  fake_node_.next->prev = created_element;
  fake_node_.next = created_element;
  if (size_ == 0) {
    fake_node_.prev = created_element;
  }
  size_++;
}

template <typename T, typename Allocator>
void List<T, Allocator>::clear_list() {
  BaseNode* current_element = static_cast<Node*>(fake_node_.next);
  while (current_element != &fake_node_) {
    BaseNode* next_element = current_element->next;
    node_allocator_traits::destroy(allocator_,
                                   static_cast<Node*>(current_element));
    node_allocator_traits::deallocate(allocator_,
                                      static_cast<Node*>(current_element), 1);
    current_element = next_element;
  }
  size_ = 0;
  fake_node_.prev = &fake_node_;
  fake_node_.next = &fake_node_;
}

template <typename T, typename Allocator>
List<T, Allocator>& List<T, Allocator>::operator=(const List& other) {
  if (this == &other) {
    return *this;
  }
  size_t start_size = size_;
  auto tmp_allocator = allocator_;

  if (node_allocator_traits::propagate_on_container_copy_assignment::value) {
    allocator_ = other.allocator_;
  }
  try {
    for (auto iter = other.cbegin(); iter != other.cend(); ++iter) {
      push_back(*iter);
    }
  } catch (...) {
    while (size_ != start_size) {
      pop_back();
    }
    allocator_ = tmp_allocator;
    throw;
  }
  allocator_ = tmp_allocator;
  for (size_t i = 0; i < start_size; ++i) {
    pop_front();
  }
  if (node_allocator_traits::propagate_on_container_copy_assignment::value) {
    allocator_ = other.allocator_;
  }

  return *this;
}

template <typename T, typename Allocator>
List<T, Allocator>::List(size_t count, const T& value,
                         const Allocator& allocator)
    : allocator_(allocator) {
  try {
    for (size_t i = 0; i < count; ++i) {
      push_back(value);
    }
  } catch (...) {
    clear_list();
    throw;
  }
}

template <typename T, typename Allocator>
List<T, Allocator>::List(const List& other)
    : allocator_(node_allocator_traits::select_on_container_copy_construction(
          other.allocator_)) {
  try {
    for (auto iter = other.cbegin(); iter != other.cend(); ++iter) {
      push_back(*iter);
    }
  } catch (...) {
    clear_list();
  }
}

template <typename T, typename Allocator>
List<T, Allocator>::List(std::initializer_list<T> init,
                         const Allocator& allocator)
    : allocator_(allocator) {
  for (auto iter = init.begin(); iter != init.end(); ++iter) {
    push_back(*iter);
  }
}

template <typename T, typename Allocator>
struct List<T, Allocator>::BaseNode {
  BaseNode() = default;
  BaseNode(BaseNode* prev, BaseNode* next) : prev(prev), next(next){};

  BaseNode* prev{this};
  BaseNode* next{this};
};

template <typename T, typename Allocator>
struct List<T, Allocator>::Node : BaseNode {
  Node() = default;
  Node(const T& val, BaseNode* prev, BaseNode* next)
      : BaseNode(prev, next), value(val){};

  T value;
};

template <typename T, typename Allocator>
template <bool IsConst>
class List<T, Allocator>::BasicIterator {
 private:
  using base_node_type =
      std::conditional_t<IsConst, const BaseNode*, BaseNode*>;
  using node_type = std::conditional_t<IsConst, const Node*, Node*>;

 public:
  using iterator_category = std::bidirectional_iterator_tag;
  using difference_type = size_t;
  using value_type = std::conditional_t<IsConst, const T, T>;
  using pointer = value_type*;
  using reference = value_type&;

  BasicIterator(base_node_type node, size_t index)
      : node_(node), index_(index){};

  BasicIterator(const BasicIterator& iterator)
      : node_(iterator.node_), index_(iterator.index_){};

  BasicIterator& operator=(const BasicIterator& iterator) {
    node_ = iterator.node_;
    index_ = iterator.index_;
    return (*this);
  };

  reference operator*() const {
    return ((static_cast<node_type>(node_))->value);
  }

  pointer operator->() const { return &(operator*()); }

  BasicIterator operator++(int) {
    BasicIterator tmp = *this;
    ++(*this);
    return tmp;
  }

  BasicIterator& operator++() {
    node_ = node_->next;
    ++index_;
    return *this;
  }

  BasicIterator operator--(int) {
    BasicIterator tmp = *this;
    --(*this);
    return tmp;
  }

  BasicIterator& operator--() {
    node_ = node_->prev;
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
    return index_ < other.index_;
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
  base_node_type node_;
  size_t index_;
};