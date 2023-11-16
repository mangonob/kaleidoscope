#pragma once
#include <string>
#include <memory>
#define is_instance(Cls, ptr) std::dynamic_pointer_cast<Cls>(ptr)

namespace utils
{
  std::string toUpper(std::string s);
}
