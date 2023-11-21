%{
    #include <iostream>
    #include <string>
    #include "utils.h"
    #include "TigerLexer.h"
    #undef YY_DECL
    #define YY_DECL yy::TigerParser::symbol_type yy::TigerLexer::_yylex()

    using namespace std;
    using y = yy::TigerParser;
%}


%option noyywrap
%option stack
%option c++
%option yyclass="TigerLexer"
%x  STR
%x  COMMENT

DIGIT       [0-9]
ID          [a-zA-Z_][a-zA-Z0-9_]*

%%

[ \t\n]                     ;
\/\/.*\n                    ;
<INITIAL,COMMENT>\/\*       yy_push_state(COMMENT);
<COMMENT>"*/"               yy_pop_state();
<COMMENT>.                  ;
<COMMENT>[ \t\n]            ;
\"                          { yy_push_state(STR); s = ""; }
<STR>\"                     {
                                std::string str = s;
                                s.clear();
                                yy_pop_state();
                                return y::make_STRING(str, loc);
                            }
<STR>\\a                    { s.append("\a"); }
<STR>\\b                    { s.append("\b"); }
<STR>\\f                    { s.append("\f"); }
<STR>\\n                    { s.append("\n"); }
<STR>\\r                    { s.append("\r"); }
<STR>\\t                    { s.append("\t"); }
<STR>\\v                    { s.append("\v"); }
<STR>\\\\                   { s.append("\\"); }
<STR>\\\'                   { s.append("\'"); }
<STR>\\\"                   { s.append("\""); }
<STR>\\0                    { s.append("\0"); }
<STR>.                      { s.append(yytext); }
"while"                     { return y::make_WHILE(loc); }
"for"                       { return y::make_FOR(loc); }
"to"                        { return y::make_TO(loc); }
"break"                     { return y::make_BREAK(loc); }
"let"                       { return y::make_LET(loc); }
"in"                        { return y::make_IN(loc); }
"end"                       { return y::make_END(loc); }
"function"                  { return y::make_FUNCTION(loc); }
"var"                       { return y::make_VAR(loc); }
"type"                      { return y::make_TYPE(loc); }
"array"                     { return y::make_ARRAY(loc); }
"if"                        { return y::make_IF(loc); }
"then"                      { return y::make_THEN(loc); }
"else"                      { return y::make_ELSE(loc); }
"do"                        { return y::make_DO(loc); }
"of"                        { return y::make_OF(loc); }
"nil"                       { return y::make_NIL(loc); }
","                         { return y::make_COMMA(loc); }
":"                         { return y::make_COLON(loc); }
";"                         { return y::make_SEMICOLON(loc); }
"("                         { return y::make_LPAREN(loc); }
")"                         { return y::make_RPAREN(loc); }
"{"                         { return y::make_LBRACE(loc); }
"}"                         { return y::make_RBRACE(loc); }
"["                         { return y::make_LRACKET(loc); }
"]"                         { return y::make_RRACKET(loc); }
"."                         { return y::make_DOT(loc); }
"+"                         { return y::make_PLUS(loc); }
"-"                         { return y::make_MINUS(loc); }
"*"                         { return y::make_TIMES(loc); }
"/"                         { return y::make_DIVIDE(loc); }
"="                         { return y::make_EQ(loc); }
"<>"                        { return y::make_NOTEQ(loc); }
"<"                         { return y::make_LT(loc); }
"<="                        { return y::make_LE(loc); }
">"                         { return y::make_GT(loc); }
">="                        { return y::make_GE(loc); }
"&"                         { return y::make_AND(loc); }
"|"                         { return y::make_OR(loc); }
":="                        { return y::make_ASSIGN(loc); }
{DIGIT}+                    { return y::make_INT(atoi(yytext), loc); }
{ID}                        { return y::make_ID(yytext, loc); }
.                           { cerr << "lexer error (reason: bad token " << yytext << ")." << endl; exit(1); }
<<EOF>>                     { return y::make_EOF(loc); }

%%
