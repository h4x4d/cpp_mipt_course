#pragma once
#include <deque>
#include <string>
#include <unordered_map>
#include <vector>

#include "AbstractToken.hpp"
#include "BracketToken.hpp"
#include "InvalidExpr.hpp"
#include "OperandToken.hpp"
#include "OperatorToken.hpp"

namespace tokens {
std::unordered_map<std::string, int> priorities{
    {"+", 1}, {"-", 1}, {"*", 2}, {"/", 2}, {"(", -1}, {")", -2},
};
}

template <typename T>
class ExprInPolishNotation {
 public:
  ExprInPolishNotation(const std::string& tokens_string);

  const std::vector<AbstractToken*>& GetTokens() { return tokens_; };

 private:
  void RemoveSpaces(std::vector<char>& tokens);

  void PushNumber(std::string& number_string);

  void PostProcess(std::deque<AbstractToken*>& waiting_operations);

  void ProcessOperator(size_t index, std::vector<char>& tokens,
                       std::string& now, int priority,
                       std::deque<AbstractToken*>& waiting_operations);

  void ProcessBracket(std::string& now, int priority,
                      std::deque<AbstractToken*>& waiting_operations);

  std::vector<AbstractToken*> tokens_;
};
template <typename T>
void ExprInPolishNotation<T>::PostProcess(
    std::deque<AbstractToken*>& waiting_operations) {
  while (!waiting_operations.empty()) {
    tokens_.push_back(waiting_operations.back());
    waiting_operations.pop_back();
  }

  std::reverse(tokens_.begin(), tokens_.end());
}
template <typename T>
void ExprInPolishNotation<T>::PushNumber(std::string& number_string) {
  if (!number_string.empty()) {
    std::reverse(number_string.begin(), number_string.end());
    tokens_.push_back(new OperandToken<T>(number_string));
    number_string.clear();
  }
}
template <typename T>
void ExprInPolishNotation<T>::ProcessBracket(
    std::string& now, int priority,
    std::deque<AbstractToken*>& waiting_operations) {
  if (priority == -2) {
    // Opening bracket
    waiting_operations.push_back(new BracketToken(now));
  } else {
    // Closing bracket
    while (!waiting_operations.empty() &&
           waiting_operations.back()->GetStringToken() != ")") {
      tokens_.push_back(waiting_operations.back());
      waiting_operations.pop_back();
    }
    if (waiting_operations.empty()) {
      while (!waiting_operations.empty()) {
        delete waiting_operations.front();
        waiting_operations.pop_front();
      }
      while (!tokens_.empty()) {
        delete tokens_.back();
        tokens_.pop_back();
      }
      throw InvalidExpr();
    }
    delete waiting_operations.back();
    waiting_operations.pop_back();
  }
}
template <typename T>
void ExprInPolishNotation<T>::ProcessOperator(
    size_t index, std::vector<char>& tokens, std::string& now, int priority,
    std::deque<AbstractToken*>& waiting_operations) {
  AbstractToken* operation;
  if (tokens::priorities[std::string(1, tokens[index + 1])] > 0 ||
      tokens::priorities[std::string(1, tokens[index + 1])] == -1) {
    operation = new OperatorToken<T, false>(now);
    priority += 2;
  } else {
    operation = new OperatorToken<T, true>(now);
  }

  while (!waiting_operations.empty() &&
         ((dynamic_cast<OperatorToken<T, false>*>(waiting_operations.back())) ||
          (tokens::priorities[waiting_operations.back()->GetStringToken()] >
           priority))) {
    tokens_.push_back(waiting_operations.back());
    waiting_operations.pop_back();
  }
  waiting_operations.push_back(operation);
}

template <typename T>
void ExprInPolishNotation<T>::RemoveSpaces(std::vector<char>& tokens) {
  for (size_t i = 0; i < tokens.size(); ++i) {
    if (tokens[i] == ' ') {
      tokens.erase(tokens.begin() + i);
      --i;
    }
  }
}

template <typename T>
ExprInPolishNotation<T>::ExprInPolishNotation(
    const std::string& tokens_string) {
  std::string number_string;
  std::vector<char> tokens(tokens_string.begin(), tokens_string.end());
  std::reverse(tokens.begin(), tokens.end());
  std::deque<AbstractToken*> waiting_operations;

  RemoveSpaces(tokens);

  for (size_t i = 0; i < tokens.size(); ++i) {
    std::string now(1, tokens[i]);
    int priority = tokens::priorities[now];

    if (priority == 0) {
      number_string += now;
      continue;
    }
    PushNumber(number_string);

    if (priority > 0) {
      ProcessOperator(i, tokens, now, priority, waiting_operations);
    } else {
      ProcessBracket(now, priority, waiting_operations);
    }
  }
  PushNumber(number_string);
  PostProcess(waiting_operations);
}
