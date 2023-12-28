#pragma once
#if !defined(yyFlexLexerOnce)
#include <FlexLexer.h>
#endif
#include <sstream>
#include "parser.tab.hpp"
#include "location.hh"

namespace yy
{
  class TigerLexer final : public yyFlexLexer
  {
  public:
    location loc;

    virtual yy::TigerParser::symbol_type _yylex();
    std::ostringstream str_o;

    void lexerError(std::string reason, location loc);
  };
}
