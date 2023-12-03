#pragma once
#include <string>

#include "AbstractToken.hpp"

class BracketToken : public AbstractToken {
 public:
  BracketToken(const std::string& bracket) : AbstractToken(bracket){};

  bool IsOpening() const { return (GetStringToken() == "("); }
};