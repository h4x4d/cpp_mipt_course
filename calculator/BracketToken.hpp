#pragma once
#include "AbstractToken.hpp"
#include <string>

class BracketToken: public AbstractToken {
 public:
  BracketToken(const std::string& bracket): AbstractToken(bracket) {};

  bool IsOpening() const { return (GetStringToken() == "("); }
};