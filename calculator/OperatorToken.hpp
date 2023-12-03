#pragma once
#include <sstream>
#include <unordered_map>

#include "AbstractToken.hpp"
#include "OperandToken.hpp"

namespace operations {
template <typename T>
const std::unordered_map<std::string, T (*)(T, T)> kBinaryOperations{
    {"+", [](T lhs, T rhs) { return lhs + rhs; }},
    {"-", [](T lhs, T rhs) { return lhs - rhs; }},
    {"*", [](T lhs, T rhs) { return lhs * rhs; }},
    {"/", [](T lhs, T rhs) { return lhs / rhs; }}};

template <typename T>
const std::unordered_map<std::string, T (*)(T)> kUnaryOperations{
    {"+", [](T element) { return element; }},
    {"-", [](T element) { return -element; }}};
}  // namespace operations

template <typename T, bool Binary>
class OperatorToken : public AbstractToken {
 public:
  OperatorToken(const std::string& token)
      : AbstractToken(token), operation_(token){};

  OperandToken<T>* Calculate(OperandToken<T>* lhs, OperandToken<T>* rhs);

  OperandToken<T>* Calculate(OperandToken<T>* operand);

  bool IsBinary() { return Binary; }

 private:
  std::string operation_;
};
template <typename T, bool Binary>
OperandToken<T>* OperatorToken<T, Binary>::Calculate(OperandToken<T>* operand) {
  static_assert(!Binary);
  auto* ans = new OperandToken(
      operations::kUnaryOperations<T>.at(operation_)(operand->GetValue()));
  return ans;
}
template <typename T, bool Binary>
OperandToken<T>* OperatorToken<T, Binary>::Calculate(OperandToken<T>* lhs,
                                                     OperandToken<T>* rhs) {
  static_assert(Binary);
  auto* ans = new OperandToken(operations::kBinaryOperations<T>.at(operation_)(
      lhs->GetValue(), rhs->GetValue()));
  return ans;
}
