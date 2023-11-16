%require "3.2"
%language "c++"
%skeleton "lalr1.cc"

%code requires {
  #include <memory>
  #include "absyn.h"

  namespace yy {
    class TigerLexer;
  }
}

%code {
  #include "TigerLexer.h"
  #define yylex lexer._yylex

  using namespace absyn;
  using namespace std;
}

%define api.token.raw
%define api.token.constructor
%define api.location.file "../include/location.hh"
%define api.value.type variant
%define api.parser.class { TigerParser }
%define parse.assert
%define api.token.prefix {TOK_}
%parse-param { yy::TigerLexer &lexer }
%parse-param { yy::TigerParser::value_type &root }

%locations

%define parse.trace
%define parse.error detailed
%define parse.lac full

%token WHILE        "while"
%token FOR          "for"
%token TO           "to"
%token BREAK        "break"
%token LET          "let"
%token IN           "in"
%token END          "end"
%token FUNCTION     "function"
%token VAR          "var"
%token TYPE         "type"
%token ARRAY        "array"
%token IF           "if"
%token THEN         "then"
%token ELSE         "else"
%token DO           "do"
%token OF           "of"
%token NIL          "nil"
%token COMMA        ","
%token COLON        ":"
%token SEMICOLON    ";"
%token LPAREN       "("
%token RPAREN       ")"
%token LBRACE       "{"
%token RBRACE       "}"
%token LRACKET      "["
%token RRACKET      "]"
%token DOT          "."
%token PLUS         "+"
%token MINUS        "-"
%token TIMES        "*"
%token DIVIDE       "/"
%token EQ           "="
%token NOTEQ        "<>"
%token LT           "<"
%token LE           "<="
%token GT           ">"
%token GE           ">="
%token AND          "&"
%token OR           "|"
%token ASSIGN       ":="
%token <int> INT "integer"
%token <std::string> STR "string"
%token <std::string> ID "id"
%token EOF 0

%nonassoc LVALUE ":=" "then" "do"
%nonassoc "else" "of" "["
%left "|"
%left "&"
%nonassoc "=" ">" ">=" "<" "<=" "<>"
%left "+" "-"
%left "*" "/"
%left UMINUS
%nterm <absyn::Exp *> exp

%start program
%%

program: exp            { root.emplace<Exp *>($1); }

exp: "while"            { $$ = new Int(1, yy::position()); }

%%

void yy::TigerParser::error(const yy::location &loc, const string &msg) {
  cout << "parser error (reason: " << msg << ")." << endl;
}
