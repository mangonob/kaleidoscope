#pragma once
#if !defined(yyFlexLexerOnce)
#include <FlexLexer.h>
#endif
#include <string>
#include "parser.tab.hpp"
#include "location.hh"

namespace yy
{
  class TigerLexer final : public yyFlexLexer
  {
  public:
    location loc;

    virtual yy::TigerParser::symbol_type _yylex();
    std::string s;
  };
}
