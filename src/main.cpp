#include <iostream>
#include <memory>
#include <sstream>
#include <nlohmann/json.hpp>
#include "parser.tab.hpp"
#include "TigerLexer.h"
#include "absyn.h"

using namespace std;
using namespace absyn;
using nlohmann::ordered_json;

int main(int argc, char *argv[])
{
  yy::TigerLexer x;
  yy::TigerParser::value_type value;
  yy::TigerParser y(x, value);

  if (y.parse() == 0)
  {
    auto result = value.as<std::shared_ptr<Exp>>();
    ostringstream o;
    Printer p(o);
    result->accept(p);
    auto j = ordered_json::parse(o.str());
    cout << setw(2) << j << endl;
  }

  exit(0);
}
