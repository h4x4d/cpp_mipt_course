#pragma once
#include <iostream>
#include <vector>

class String {
 public:
  String() = default;

  String(unsigned size, char character);

  String(const char* string, int size, int capacity);

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

  unsigned Size() const;

  unsigned Capacity() const;

  char* Data();

  const char* Data() const;

  void Clear();

  void PushBack(char character);

  void PopBack();

  void Resize(unsigned new_size);

  void Resize(unsigned new_size, char character);

  void Reserve(unsigned new_cap);

  void ShrinkToFit();

  void Swap(String& other);

  int Find(int start_pos, const String& to_find);

  std::vector<String> Split(const String& delim = " ");

  String Join(const std::vector<String>& strings) const;

  ~String();

 private:
  void ChangeCapacity(unsigned new_capacity);

  unsigned size_ = 0;
  unsigned capacity_ = 0;
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