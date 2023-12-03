#pragma once
#include <string>

class AbstractToken {
 public:
  AbstractToken(const std::string& token) : token_(token){};

  const std::string& GetStringToken() const { return token_; };

  void UpdateStringToken(const std::string& token) { token_ = token; };

  virtual ~AbstractToken() = default;

 private:
  std::string token_;
};