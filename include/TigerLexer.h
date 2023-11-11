#pragma once
#if !defined(yyFlexLexerOnce)
#include <FlexLexer.h>
#endif

namespace yy
{
  class TigerLexer final : public yyFlexLexer
  {
  public:
    virtual int yylex();
  };
}
