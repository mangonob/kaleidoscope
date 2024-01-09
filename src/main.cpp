#include <iostream>
#include <memory>
#include <sstream>
#include <argparse/argparse.hpp>
#include "parser.tab.hpp"
#include "TigerLexer.h"
#include "absyn.h"
#include "codegen.h"
#include "types.h"

using namespace std;
using namespace absyn;

int main(int argc, char *argv[])
{
  argparse::ArgumentParser program("kalec", "1.0");
  program.add_description("a tiger language compiler");

  program.add_argument("input")
      .remaining();

  program.add_argument("-o", "--output")
      .help("ouput filename")
      .metavar("output")
      .required();

  program.add_argument("-emit-ir")
      .help("emit llvm-ir")
      .implicit_value(true)
      .default_value(false);

  try
  {
    program.parse_args(argc, argv);
  }
  catch (const exception &error)
  {
    cerr << program;
    exit(1);
  }

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
