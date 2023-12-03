#pragma once
#include "AbstractToken.hpp"
#include <sstream>
#include <string>

template <typename T>
class OperandToken: public AbstractToken {
 public:
  OperandToken(const std::string& str_operand);

  OperandToken(const T& value);

  const T& GetValue() { return operand_; }

 private:
  T operand_;
};

template<typename T>
OperandToken<T>::OperandToken(const T &value)
    : operand_(value), AbstractToken("") {
  std::stringstream string_stream;
  string_stream << value;
  UpdateStringToken(string_stream.str());
}

template<typename T>
OperandToken<T>::OperandToken(const std::string &str_operand)
    : AbstractToken(str_operand) {
  std::stringstream string_stream(str_operand);
  string_stream >> operand_;
}
