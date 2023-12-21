#pragma once
#include <string>
#include <memory>
#define is_instance(T, ptr) (dynamic_cast<const T *>(ptr))

namespace utils
{
  std::string toUpper(std::string s);
}
