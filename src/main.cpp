#include <iostream>
#include <utility>
#include <memory>
#include "parser.tab.hpp"
#include "TigerLexer.h"
#include "absyn.h"

using namespace std;
using namespace absyn;

int main(int argc, char *argv[])
{
  yy::TigerLexer x;
  yy::TigerParser::value_type value;
  yy::TigerParser y(x, value);

  if (y.parse() == 0)
  {
    auto result = shared_ptr<Exp>(value.as<Exp *>());
    result->print(cout, 0, false);
  }

  exit(0);
}
