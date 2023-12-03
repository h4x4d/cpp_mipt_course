#pragma once
#include "AbstractToken.hpp"
#include "OperandToken.hpp"
#include <sstream>
#include <string>

namespace operations {
template<typename T>
const std::unordered_map<std::string, T (*)(T, T)> binary_operations_ {
    {"+", [](T lhs, T rhs) { return lhs + rhs; }},
    {"-", [](T lhs, T rhs) { return lhs - rhs; }},
    {"*", [](T lhs, T rhs) { return lhs * rhs; }},
    {"/", [](T lhs, T rhs) { return lhs / rhs; }}
};

template<typename T>
const std::unordered_map<std::string, T (*)(T)> unary_operations_ {
    {"+", [](T element) { return element; }},
    {"-", [](T element) { return -element; }}
};
}

template <typename T, bool is_binary>
class OperatorToken: public AbstractToken {
 public:
  OperatorToken(const std::string& token)
      : operation_(token), AbstractToken(token) {};

  OperandToken<T>* Calculate(OperandToken<T>* lhs, OperandToken<T>* rhs);

  OperandToken<T>* Calculate(OperandToken<T>* operand);

  bool IsBinary() { return is_binary; }

 private:
  std::string operation_;
};
template<typename T, bool is_binary>
OperandToken<T> *OperatorToken<T,
                               is_binary>::Calculate(OperandToken<T> *operand) {
  static_assert(!is_binary);
  auto ans = new OperandToken(
      operations::unary_operations_<T>.at(operation_)(operand->GetValue()));
  return ans;
}
template<typename T, bool is_binary>
OperandToken<T> *OperatorToken<T, is_binary>::Calculate(OperandToken<T> *lhs,
                                                        OperandToken<T> *rhs) {
  static_assert(is_binary);
  auto ans = new OperandToken(
      operations::binary_operations_<T>.at(operation_)(lhs->GetValue(),
          rhs->GetValue()));
  return ans;
}
