#include "string.hpp"

#include <cstring>

String::String(unsigned size, char character)
    : size_(size), capacity_(size + 1), string_(new char[capacity_]) {
  std::fill(string_, string_ + size_, character);
  string_[capacity_ - 1] = '\0';
}

String::String(const char* string, int size, int capacity)
    : size_(size), capacity_(capacity), string_(new char[capacity_]) {
  std::copy(string, string + size_, string_);
  if (capacity_ == 0 || capacity_ == size_) {
    ChangeCapacity(capacity_ + 1);
  }
  string_[capacity_ - 1] = '\0';
}

String::String(const char* string)
    : String(string, std::strlen(string), std::strlen(string) + 1) {}

String::String(const String& string)
    : String(string.string_, string.size_, string.capacity_) {}

bool String::Empty() const { return size_ == 0; }

size_t String::Size() const { return size_; }

size_t String::Capacity() const { return capacity_; }

char* String::Data() { return string_; }

const char* String::Data() const { return string_; }

char& String::Front() { return string_[0]; }

char String::Front() const { return string_[0]; }

char& String::Back() { return string_[size_ - 1]; }

char String::Back() const { return string_[size_ - 1]; }

String& String::operator=(const String& other) {
  if (this == &other) {
    return *this;
  }
  ChangeCapacity(other.size_);
  size_ = other.size_;
  std::copy(other.string_, other.string_ + size_, string_);
  return *this;
}

void String::ChangeCapacity(size_t new_capacity) {
  char* new_string = new char[new_capacity];
  if (new_capacity < size_) {
    std::copy(string_, string_ + new_capacity, new_string);
  } else if (string_ != nullptr) {
    std::copy(string_, string_ + size_, new_string);
  }
  capacity_ = new_capacity;
  size_ = size_ <= capacity_ ? size_ : capacity_;
  delete[] string_;
  string_ = new_string;
}

void String::PushBack(char character) {
  if (capacity_ < size_ + 1) {
    ChangeCapacity(capacity_ > 0 ? capacity_ * 2 : 1);
  }
  size_ += 1;
  string_[size_ - 1] = character;
}

void String::PopBack() {
  if (size_ > 0) {
    size_ -= 1;
  }
}

void String::Resize(size_t new_size) {
  if (new_size > capacity_) {
    ChangeCapacity(new_size);
  }
  size_ = new_size;
}

void String::Resize(size_t new_size, char character) {
  if (new_size > capacity_) {
    ChangeCapacity(new_size);
  }
  if (new_size > size_) {
    std::fill(string_ + size_, string_ + new_size, character);
  }
  size_ = new_size;
}

void String::Reserve(size_t new_cap) {
  if (new_cap > capacity_) {
    ChangeCapacity(new_cap);
  }
}

void String::ShrinkToFit() {
  if (capacity_ > size_) {
    ChangeCapacity(size_);
  }
}

void String::Swap(String& other) {
  std::swap(string_, other.string_);
  std::swap(size_, other.size_);
  std::swap(capacity_, other.capacity_);
}

String& String::operator+=(const String& other) {
  if (capacity_ < size_ + other.size_) {
    ChangeCapacity(size_ + other.size_);
  }
  std::copy(other.string_, other.string_ + other.size_, (string_ + size_));
  size_ += other.size_;
  return *this;
}

String operator+(const String& first, const String& other) {
  String new_string(first);
  new_string += other;
  return new_string;
}

String& String::operator*=(int number) {
  char* new_string = new char[size_ * number];
  for (unsigned i = 0; i < size_ * number; ++i) {
    new_string[i] = string_[i % size_];
  }
  delete[] string_;
  string_ = new_string;
  size_ *= number;
  capacity_ = size_;
  return *this;
}

String String::operator*(int number) const {
  String new_string(*this);
  new_string *= number;
  return new_string;
}

std::ostream& operator<<(std::ostream& os, const String& string) {
  os << string.Data();
  return os;
}

std::istream& operator>>(std::istream& is, String& string) {
  char symbol;
  while (is.get(symbol)) {
    if ((symbol == ' ' || symbol == '\n')) {
      if (!string.Empty()) {
        break;
      }
    } else {
      string.PushBack(symbol);
    }
  }
  return is;
}

void String::Clear() { size_ = 0; }

String::~String() { delete[] string_; }

char& String::operator[](int idx) { return string_[idx]; }

char String::operator[](int idx) const { return string_[idx]; }

bool operator<(const String& first, const String& second) {
  unsigned size = first.Size() > second.Size() ? second.Size() : first.Size();
  for (unsigned i = 0; i < size; ++i) {
    if (first[i] < second[i]) {
      return true;
    }
    if (first[i] > second[i]) {
      return false;
    }
  }
  return first.Size() < second.Size();
}

bool operator==(const String& first, const String& second) {
  if (first.Size() != second.Size()) {
    return false;
  }
  for (unsigned i = 0; i < first.Size(); ++i) {
    if (first[i] != second[i]) {
      return false;
    }
  }
  return true;
}

bool operator>(const String& first, const String& second) {
  return second < first;
}

bool operator>=(const String& first, const String& second) {
  return !(first < second);
}

bool operator<=(const String& first, const String& second) {
  return !(first > second);
}

bool operator!=(const String& first, const String& second) {
  return !(first == second);
}

int String::Find(int start_pos, const String& to_find) {
  int find_ind = 0;
  int size = to_find.Size();
  for (unsigned i = start_pos; i < size_; ++i) {
    if (string_[i] == to_find[find_ind]) {
      find_ind += 1;
      if (find_ind == size) {
        return i - size + 1;
      }
    } else {
      find_ind = 0;
    }
  }
  if (find_ind == size) {
    return size_ - size;
  }
  return -1;
}

std::vector<String> String::Split(const String& delim) {
  std::vector<String> strings;
  int start_pos = 0;
  int find_ind = Find(start_pos, delim);

  while (find_ind != -1) {
    String buffer;
    for (int i = start_pos; i < find_ind; ++i) {
      buffer.PushBack(string_[i]);
    }
    strings.push_back(buffer);
    start_pos = find_ind + delim.size_;
    find_ind = Find(start_pos, delim);
  }
  String buffer;
  for (unsigned i = start_pos; i < size_; ++i) {
    buffer.PushBack(string_[i]);
  }
  strings.push_back(buffer);
  return strings;
}

String String::Join(const std::vector<String>& strings) const {
  String new_string;
  int size = strings.size();
  for (int string_ind = 0; string_ind < size - 1; ++string_ind) {
    new_string += strings[string_ind];
    new_string += *this;
  }
  if (size > 0) {
    new_string += strings[size - 1];
  }
  return new_string;
}