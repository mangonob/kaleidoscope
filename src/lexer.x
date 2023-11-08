%{
%}

%option noyywrap
%option c++
%option stack
%x  STR
%x  COMMENT

DIGIT       [0-9]
ID          [a-zA-Z_][a-zA-Z0-9_]*
%%

    yyin = input ? input : stdin;

[ \t\n]                     ;
\/\/.*\n                    ;
<INITIAL,COMMENT>\/\*       { yy_push_state(COMMENT); } /* Begin comments */
<COMMENT>"*/"               { yy_pop_state(); }         /* End comments */
<COMMENT>.                  ;
<COMMENT>[ \t\n]            ;
\"                          { str_buffer_cursor = 0; yy_push_state(STR); }
<STR>\"                     {
                                yy_pop_state();
                                append_char('\0');
                                yylval.sval = strdup(str_buffer);
                                return STRING;
                            }
<STR>\\a                    append_char('\a');
<STR>\\b                    append_char('\b');
<STR>\\f                    append_char('\f');
<STR>\\n                    append_char('\n');
<STR>\\r                    append_char('\r');
<STR>\\t                    append_char('\t');
<STR>\\v                    append_char('\v');
<STR>\\\\                   append_char('\\');
<STR>\\\'                   append_char('\'');
<STR>\\\"                   append_char('\"');
<STR>\\0                    append_char('\0');
<STR>.                      append_char(yytext[0]);
"while"                     return WHILE;
"for"                       return FOR;
"to"                        return TO;
"break"                     return BREAK;
"let"                       return LET;
"in"                        return IN;
"end"                       return END;
"function"                  return FUNCTION;
"var"                       return VAR;
"type"                      return TYPE;
"array"                     return ARRAY;
"if"                        return IF;
"then"                      return THEN;
"else"                      return ELSE;
"do"                        return DO;
"of"                        return OF;
"nil"                       return NIL;
","                         return COMMA;
":"                         return COLON;
";"                         return SEMICOLON;
"("                         return LPAREN;
")"                         return RPAREN;
"{"                         return LBRACE;
"}"                         return RBRACE;
"["                         return LRACKET;
"]"                         return RRACKET;
"."                         return DOT;
"+"                         return PLUS;
"-"                         return MINUS;
"*"                         return TIMES;
"/"                         return DIVIDE;
"="                         return EQ;
"<>"                        return NOTEQ;
"<"                         return LT;
"<="                        return LE;
">"                         return GT;
">="                        return GE;
"&"                         return AND;
"|"                         return OR;
":="                        return ASSIGN;
{DIGIT}+                    { sscanf(yytext, "%ld", &yylval.ival); return INT; }
{ID}                        { yylval.idval = strdup(yytext); return ID; }
.                           { fprintf(stderr, "bad token %s\n", yytext); exit(1); }
<<EOF>>                     return EOF;

%%
