#include <iostream>
#include "TigerLexer.h"

int main(int argc, char *argv[])
{
  yy::TigerLexer y;
  while (y.yylex())
    ;
  exit(0);
}
