#pragma once
#include <deque>
#include "ExprInPolishNotation.hpp"
#include "OperandToken.hpp"
#include "OperatorToken.hpp"
#include <string>

template <typename T>
class Calculator {
 public:
  static T CalculateExpr(const std::string& expr);

  static void CalculateTokens(std::deque<AbstractToken*>& tokens);
};

template<typename T>
T Calculator<T>::CalculateExpr(const std::string &expr) {
  auto tokens = ExprInPolishNotation<T>(expr).GetTokens();
  auto deque = std::deque<AbstractToken*>(tokens.begin(), tokens.end());
  CalculateTokens(deque);
  if (deque.size() != 1) {
    throw InvalidExpr();
  }
  return dynamic_cast<OperandToken<T>*>(deque.front())->GetValue();
}

template<typename T>
void Calculator<T>::CalculateTokens(std::deque<AbstractToken*>& tokens) {
  AbstractToken* now = tokens.front();

  if (dynamic_cast<OperatorToken<T, true>*>(now) != nullptr) {
    tokens.pop_front();
    auto operation = dynamic_cast<OperatorToken<T, true>*>(now);
    CalculateTokens(tokens);
    OperandToken<T>* first = dynamic_cast<OperandToken<T>*>(tokens.front());
    tokens.pop_front();
    CalculateTokens(tokens);
    OperandToken<T>* second = dynamic_cast<OperandToken<T>*>(tokens.front());
    tokens.pop_front();
    tokens.push_front(operation->Calculate(first, second));
  } else if (dynamic_cast<OperatorToken<T, false>*>(now) != nullptr) {
    tokens.pop_front();
    auto operation = dynamic_cast<OperatorToken<T, false>*>(now);
    CalculateTokens(tokens);
    OperandToken<T>* element = dynamic_cast<OperandToken<T>*>(tokens.front());
    tokens.pop_front();
    tokens.push_front(operation->Calculate(element));
  }
}
