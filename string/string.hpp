#pragma once
#include <iostream>
#include <vector>

class String {
 public:
  String() = default;

  String(unsigned size, char character);

  String(const char* string);

  String(const String& string);

  String& operator=(const String& other);

  char& operator[](int idx);

  char operator[](int idx) const;

  String& operator+=(const String& other);

  String operator*(int number) const;
  String& operator*=(int number);

  char& Front();

  char Front() const;

  char& Back();

  char Back() const;

  bool Empty() const;

  size_t Size() const;

  size_t Capacity() const;

  char* Data();

  const char* Data() const;

  void Clear();

  void PushBack(char character);

  void PopBack();

  void Resize(size_t new_size);

  void Resize(size_t new_size, char character);

  void Reserve(size_t new_cap);

  void ShrinkToFit();

  void Swap(String& other);

  int Find(int start_pos, const String& to_find);

  std::vector<String> Split(const String& delim = " ");

  String Join(const std::vector<String>& strings) const;

  ~String();

 private:
  String(const char* string, int size, int capacity);

  void ChangeCapacity(size_t new_capacity);

  size_t size_ = 0;
  size_t capacity_ = 0;
  char* string_ = nullptr;
};

bool operator<(const String& first, const String& second);

bool operator==(const String& first, const String& second);

bool operator>(const String& first, const String& second);

bool operator>=(const String& first, const String& second);

bool operator<=(const String& first, const String& second);

bool operator!=(const String& first, const String& second);

std::ostream& operator<<(std::ostream& os, const String& string);

std::istream& operator>>(std::istream& is, String& string);

String operator+(const String& first, const String& other);