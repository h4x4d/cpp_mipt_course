#pragma once
#include <deque>
#include <list>
#include <string>
#include <unordered_map>
#include <vector>

#include "AbstractToken.hpp"
#include "BracketToken.hpp"
#include "InvalidExpr.hpp"
#include "OperandToken.hpp"
#include "OperatorToken.hpp"

namespace tokens {
const std::unordered_map<std::string, int> kPriorities{
    {"+", 1}, {"-", 1}, {"*", 2}, {"/", 2}, {"(", -1}, {")", -2},
};
}

int GetPriority(const std::string& token) {
  int priority = 0;
  if (tokens::kPriorities.contains(token)) {
    priority = tokens::kPriorities.at(token);
  }
  return priority;
}

template <typename T>
class ExprInPolishNotation {
 public:
  ExprInPolishNotation(const std::string& tokens_string);

  const std::vector<AbstractToken*>& GetTokens() { return tokens_; };

 private:
  void RemoveSpaces(std::list<char>& tokens);

  void PushNumber(std::string& number_string);

  void PostProcess(std::deque<AbstractToken*>& waiting_operations);

  void ProcessOperator(std::list<char>::iterator iter, std::string& now,
                       int priority,
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
    std::list<char>::iterator iter, std::string& now, int priority,
    std::deque<AbstractToken*>& waiting_operations) {
  AbstractToken* operation;
  if (GetPriority(std::string(1, *++iter)) > 0 ||
      GetPriority(std::string(1, *iter)) == -1) {
    operation = new OperatorToken<T, false>(now);
    priority += 2;
  } else {
    operation = new OperatorToken<T, true>(now);
  }

  while (
      !waiting_operations.empty() &&
      ((dynamic_cast<OperatorToken<T, false>*>(waiting_operations.back())) ||
       (GetPriority(waiting_operations.back()->GetStringToken()) > priority))) {
    tokens_.push_back(waiting_operations.back());
    waiting_operations.pop_back();
  }
  waiting_operations.push_back(operation);
}

template <typename T>
void ExprInPolishNotation<T>::RemoveSpaces(std::list<char>& tokens) {
  for (auto token_it = tokens.begin(); token_it != tokens.end(); ++token_it) {
    if ((*token_it) == ' ') {
      tokens.erase(token_it++);
    }
  }
}

template <typename T>
ExprInPolishNotation<T>::ExprInPolishNotation(
    const std::string& tokens_string) {
  std::string number_string;
  std::list<char> tokens(tokens_string.begin(), tokens_string.end());
  std::reverse(tokens.begin(), tokens.end());
  std::deque<AbstractToken*> waiting_operations;

  RemoveSpaces(tokens);

  for (auto token_it = tokens.begin(); token_it != tokens.end(); ++token_it) {
    std::string now(1, *token_it);
    int priority = GetPriority(now);

    if (priority == 0) {
      number_string += now;
      continue;
    }
    PushNumber(number_string);

    if (priority > 0) {
      ProcessOperator(token_it, now, priority, waiting_operations);
    } else {
      ProcessBracket(now, priority, waiting_operations);
    }
  }
  PushNumber(number_string);
  PostProcess(waiting_operations);
}
