#include <iostream>
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
    auto result = value.as<std::shared_ptr<Exp>>();
    Printer p(cout);
    result->accept(p);
    p.out << endl;
  }

  exit(0);
}
