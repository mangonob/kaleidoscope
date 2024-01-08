#include <iostream>
#include <memory>
#include <sstream>
#include <llvm/Support/Format.h>
#include "parser.tab.hpp"
#include "TigerLexer.h"
#include "absyn.h"
#include "codegen.h"
#include "types.h"

using namespace std;
using namespace absyn;

int main(int argc, char *argv[])
{
  yy::TigerLexer x;
  shared_ptr<Exp> exp;
  yy::TigerParser y(x, exp);

  if (y.parse() == 0)
  {
    cg::CodeGenerator generator;
    generator.generate(*exp);
  }

  exit(0);
}
