#pragma once
#include "AbstractToken.hpp"
#include "BracketToken.hpp"
#include <deque>
#include "InvalidExpr.hpp"
#include <map>
#include "OperandToken.hpp"
#include "OperatorToken.hpp"
#include <string>


namespace tokens {
std::unordered_map<std::string, int> priorities {
    {"+", 1},
    {"-", 1},
    {"*", 2},
    {"/", 2},
    {"(", -1},
    {")", -2},
};
}

template <typename T>
class ExprInPolishNotation {
 public:
  ExprInPolishNotation(const std::string& tokens_string);

  const std::vector<AbstractToken*>& GetTokens() { return tokens_; };

 private:
  std::vector<AbstractToken*> tokens_;
};

template <typename T>
ExprInPolishNotation<T>::ExprInPolishNotation(const std::string& tokens_string) {
  std::string number_string;
  std::vector<char> tokens(tokens_string.begin(), tokens_string.end());
  std::reverse(tokens.begin(), tokens.end());

  std::deque<AbstractToken*> stack;

  for (size_t i = 0; i < tokens.size(); ++i) {
    if (tokens[i] == ' ') {
      tokens.erase(tokens.begin() + i);
      --i;
    }
  }

  for (size_t i = 0; i < tokens.size(); ++i) {
    std::string now(1, tokens[i]);
    int priority = tokens::priorities[now];

    // Case 0: number
    if (priority == 0) {
      number_string += now;
      continue;
    } else {
      if (!number_string.empty()) {
        std::reverse(number_string.begin(), number_string.end());
        tokens_.push_back(new OperandToken<T>(number_string));
        number_string.clear();
      }
    }

    if (priority > 0) {
      // Case 1: Operator
      AbstractToken* operation;
      if (tokens::priorities[std::string(1, tokens[i + 1])] > 0 ||
          tokens::priorities[std::string(1, tokens[i + 1])] == -1) {
        operation = new OperatorToken<T, false>(now);
        priority += 2;
      } else {
        operation = new OperatorToken<T, true>(now);
      }

      while (!stack.empty() &&
            ((dynamic_cast<OperatorToken<T, false>*>(stack.back())) ||
            (tokens::priorities[stack.back()->GetStringToken()] > priority))) {
        tokens_.push_back(stack.back());
        stack.pop_back();
      }
      stack.push_back(operation);
    } else if (priority == -2) {
      // Case 2.1: Opening bracket
      stack.push_back(new BracketToken(now));
    } else {
      // Case 2.2: Closing bracket
      while (!stack.empty() && stack.back()->GetStringToken() != ")") {
        tokens_.push_back(stack.back());
        stack.pop_back();
      }
      if (stack.empty()) {
        throw InvalidExpr();
      }
      stack.pop_back();
    }
  }
  if (!number_string.empty()) {
    std::reverse(number_string.begin(), number_string.end());
    tokens_.push_back(new OperandToken<T>(number_string));
    number_string.clear();
  }
  while (!stack.empty()) {
    tokens_.push_back(stack.back());
    stack.pop_back();
  }

  std::reverse(tokens_.begin(), tokens_.end());
}
