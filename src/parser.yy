%require "3.2"
%language "c++"
%skeleton "lalr1.cc"
%header

%{
%}

%define api.token.raw
%define api.token.constructor
%define api.location.file "../include/location.hh"
%define api.value.type variant
%define api.parser.class { TigerParser }
%define parse.assert
%define api.token.prefix {TOK_}

%code requires {
}

%locations

%define parse.trace
%define parse.error detailed
%define parse.lac full

%code {
}

%token <int> INT "integer";
%token <std::string> STR "string";
%token COLON ":";
%token EOF 0;

%%
%start  program;

program: ":";
%%
