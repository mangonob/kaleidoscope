%{
    #include <iostream>
    #include <string>
    #include "utils.h"
    #include "TigerLexer.h"
    using namespace std;
%}


%option noyywrap
%option stack
%option c++
%option yyclass="yy::TigerLexer"
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
\"                          ;
<STR>\"                     ;
<STR>\\a                    ;
<STR>\\b                    ;
<STR>\\f                    ;
<STR>\\n                    ;
<STR>\\r                    ;
<STR>\\t                    ;
<STR>\\v                    ;
<STR>\\\\                   ;
<STR>\\\'                   ;
<STR>\\\"                   ;
<STR>\\0                    ;
<STR>.                      ;
"while"                     |
"for"                       |
"to"                        |
"break"                     |
"let"                       |
"in"                        |
"end"                       |
"function"                  |
"var"                       |
"type"                      |
"array"                     |
"if"                        |
"then"                      |
"else"                      |
"do"                        |
"of"                        |
"nil"                       { cout << "<KEYWORD: " << yytext << ">" << endl; }
","                         ;
":"                         ;
";"                         ;
"("                         ;
")"                         ;
"{"                         ;
"}"                         ;
"["                         ;
"]"                         ;
"."                         ;
"+"                         ;
"-"                         ;
"*"                         ;
"/"                         ;
"="                         ;
"<>"                        ;
"<"                         ;
"<="                        ;
">"                         ;
">="                        ;
"&"                         ;
"|"                         ;
":="                        { cout << yytext << endl; }
{DIGIT}+                    { cout << "<INT, " << yytext << ">" << endl; }
{ID}                        { cout << "<ID, " << yytext << ">" << endl; }
.                           { cerr << "bad token " << yytext; return 0; }
<<EOF>>                     { cout << "<EOF>" << endl; yyterminate(); }

%%
