#pragma once
#include <iostream>

namespace absyn
{
  struct Exp
  {
    enum
    {
      intExp,
      stringExp,
      nilExp
    } kind;

    union
    {
      int int_value;
      std::string string;
    };
  };
}
