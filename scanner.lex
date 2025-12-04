%{
#include "parser.tab.hpp"
  extern "C" int yylex();
%}
%option noyywrap
%option yylineno

%%
"if"          { return IF; }
"while"       { return WHILE; }
"def"         { return DEFINE; }

[a-zA-Z][a-zA-Z0-9_]* { yylval.idName = strdup(yytext); return ID; }
[0-9]+\.[0-9]+ { yylval.floatVal = atof(yytext); return FLOAT_LITERAL; }
[0-9]+        { yylval.intVal = atoi(yytext); return INTEGER_LITERAL; }
"+"           { return PLUS; }
"-"           { return MINUS; }
"*"           { return MULT; }
"/"           { return DIV; }
"="           { return ASSIGN; }
";"           { return SEMI; }
","           { return COMMA; }

"["           { return ARRAY_START; }
"]"           { return ARRAY_END; }
"{"           { return BRACE_LEFT; }
"}"           { return BRACE_RIGHT; }
"("           { return PAREN_LEFT; }
")"           { return PAREN_RIGHT; }
":"           { return DEFINE_DEFAULT; }

"<=" { return LE; }
">=" { return GE; }
"==" { return EQ; }
"!=" { return NE; }
"<"  { return LT; }
">"  { return GT; }

[ \t\r\n\f]+   ; /* ignore whitespace */
