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
%token <std::string> STRING "string"
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
%nterm <absyn::ptr<absyn::Exp>> exp
%nterm <absyn::ptr<absyn::Var>> lvalue
%nterm <absyn::ptrs<absyn::Exp>> expseq expseq_ arg_list arg_list_
%nterm <absyn::ptrs<absyn::Record>> record_list record_list_
%nterm <absyn::ptr<absyn::Record>> record
%nterm <absyn::ptrs<absyn::Dec>> decs
%nterm <absyn::ptr<absyn::Dec>> dec typedec vardec funcdec
%nterm <absyn::ptr<absyn::Type>> ty
%nterm <absyn::ptrs<absyn::Field>> tyfields tyfields_
%nterm <absyn::ptr<absyn::Field>> tyfield
%nterm <absyn::ptr<absyn::ID>> id

%start program
%%

program: exp            { root.emplace<std::shared_ptr<Exp>>($1); }

exp:        INT                                 { $$ = make_shared<Int>($1, @1.begin); }
            | STRING                            { $$ = make_shared<String>($1, @1.begin); }
            | NIL                               { $$ = make_shared<Nil>(@1.begin); }
            | lvalue                            { $$ = make_shared<VarExp>($1, @1.begin); }
            | lvalue ":=" exp                   { $$ = make_shared<Assign>($1, $3, @1.begin); }
            | "(" expseq ")"                    { $$ = make_shared<Seq>($2, @1.begin); }
            | "-" exp %prec UMINUS              {
                                                  auto zero = make_shared<Int>(0, @1.begin);
                                                  $$ = make_shared<BinOp>(zero, $2, Oper::minusOp, @1.begin);
                                                }
            | id "(" arg_list ")"               { $$ = make_shared<Call>($1, $3, @1.begin); }
            | exp "+" exp                       { $$ = make_shared<BinOp>($1, $3, Oper::plusOp, @2.begin); }
            | exp "-" exp                       { $$ = make_shared<BinOp>($1, $3, Oper::minusOp, @2.begin); }
            | exp "*" exp                       { $$ = make_shared<BinOp>($1, $3, Oper::timesOp, @2.begin); }
            | exp "/" exp                       { $$ = make_shared<BinOp>($1, $3, Oper::divideOp, @2.begin); }
            | exp "=" exp                       { $$ = make_shared<BinOp>($1, $3, Oper::eqOp, @2.begin); }
            | exp "<>" exp                      { $$ = make_shared<BinOp>($1, $3, Oper::neqOp, @2.begin); }
            | exp ">" exp                       { $$ = make_shared<BinOp>($1, $3, Oper::gtOp, @2.begin); }
            | exp "<" exp                       { $$ = make_shared<BinOp>($1, $3, Oper::ltOp, @2.begin); }
            | exp ">=" exp                      { $$ = make_shared<BinOp>($1, $3, Oper::geOp, @2.begin); }
            | exp "<=" exp                      { $$ = make_shared<BinOp>($1, $3, Oper::leOp, @2.begin); }
            | exp "&" exp                       {
                                                  auto zero = make_shared<Int>(0, @1.begin);
                                                  $$ = make_shared<If>($1, $3, zero, @1.begin);
                                                }
            | exp "|" exp                       {
                                                  auto one = make_shared<Int>(1, @1.begin);
                                                  $$ = make_shared<If>($1, one, $3, @1.begin);
                                                }
            | id "{" record_list "}"            { $$ = make_shared<RecordExp>($1, $3, @1.begin); }
            | id "[" exp "]" "of" exp           { $$ = make_shared<Array>($1, $3, $6, @1.begin); }
            | IF exp THEN exp ELSE exp          { $$ = make_shared<If>($2, $4, $6, @1.begin); }
            | IF exp THEN exp                   { $$ = make_shared<If>($2, $4, nullptr, @1.begin); }
            | WHILE exp DO exp                  { $$ = make_shared<While>($2, $4, @1.begin); }
            | FOR id ":=" exp TO exp DO exp     { $$ = make_shared<For>($2, $4, $6, $8, @1.begin); }
            | BREAK                             { $$ = make_shared<Break>(@1.begin); }
            | LET decs IN expseq END            {
                                                  auto seq = make_shared<Seq>($4, @4.begin);
                                                  $$ = make_shared<Let>($2, seq, @1.begin);
                                                }

lvalue:     id %prec LVALUE                     { $$ = make_shared<SimpleVar>($1, @1.begin); }
            | id "[" exp "]"                    {
                                                  auto var = make_shared<SimpleVar>($1, @1.begin);
                                                  $$ = make_shared<SubscriptVar>(var, $3, @1.begin);
                                                }
            | lvalue "." id                     { $$ = make_shared<FieldVar>($1, $3, @1.begin); }
            | lvalue "[" exp "]"                { $$ = make_shared<SubscriptVar>($1, $3, @1.begin); }

expseq:     /* empty */                         { $$ = ptrs<Exp>(); }
            | expseq_                           { $$ = $1; }

expseq_:    exp                                 { $$ = ptrs<Exp>{ $1 }; }
            | expseq_ ";" exp                   { $$ = $1; $$.push_back($3); }

arg_list:   /* empty */                         { $$ = ptrs<Exp>(); }
            | arg_list_                         { $$ = $1; }

arg_list_:  exp                                 { $$ = ptrs<Exp>{ $1 }; }
            | arg_list_ "," exp                 { $$ = $1; $$.push_back($3); }

record_list:    /* empty */                     { $$ = ptrs<Record>(); }
            | record_list_                      { $$ = $1; }

record_list_:   record                          { $$ = ptrs<Record>{ $1 }; }
            | record_list_ "," record           { $$ = $1; $$.push_back($3); }

record:     id "=" exp                          { $$ = make_shared<Record>($1, $3, @1.begin); }

decs:       /* empty */                         { $$ = ptrs<Dec>(); }
            | decs dec                          { $$ = $1, $$.push_back($2); }

dec:        typedec                             { $$ = $1; }
            | vardec                            { $$ = $1; }
            | funcdec                           { $$ = $1; }

typedec:    "type" id "=" ty                    { $$ = make_shared<TypeDec>($2, $4, @1.begin); }

ty:         id                                  { $$ = make_shared<NamedType>($1, @1.begin); }
            | "array" "of" id                   { $$ = make_shared<ArrayType>($3, @1.begin); }
            | "{" tyfields "}"                  { $$ = make_shared<RecordType>($2, @1.begin); }

tyfields:   /* empty */                         { $$ = ptrs<Field>(); }
            | tyfields_                         { $$ = $1; }

tyfields_:  tyfield                             { $$ = ptrs<Field>{ $1 }; }
            | tyfields_ "," tyfield             { $$ = $1; $$.push_back($3); }

tyfield:    id ":" id                           { $$ = make_shared<Field>($1, $3, @1.begin); }

vardec:     "var" id ":=" exp                   { $$ = make_shared<VarDec>($2, nullptr, $4, @1.begin); }
            | "var" id ":" id ":=" exp          { $$ = make_shared<VarDec>($2, $4, $6, @1.begin); }

funcdec:    "function" id "(" tyfields ")" "=" exp
                                                { $$ = make_shared<FunctionDec>($2, $4, nullptr, $7, @1.begin); }
            | "function" id "(" tyfields ")" ":" id "=" exp
                                                { $$ = make_shared<FunctionDec>($2, $4, $7, $9, @1.begin); }

id:         ID                                  { $$ = make_shared<ID>($1, @1.begin); }

%%

void yy::TigerParser::error(const yy::location &loc, const string &msg) {
  cout << "parser error (reason: " << msg << ")." << endl;
}
