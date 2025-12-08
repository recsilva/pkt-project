%{
#include "parser.tab.hpp"
  extern "C" int yylex();
%}
%option noyywrap
%option yylineno

%%
"open"        { return OPEN; }
"read"        { return READ; }
"write"        { return WRITE; }
"close"        { return CLOSE; }
"print"        { return PRINT; }

"if"          { return IF; }
"while"       { return WHILE; }
"def"         { return DEFINE; }
"return"      { return RETURN; }

"int[]"       { return INT_ARRAY_TYPE; }
"flt[]"       { return FLT_ARRAY_TYPE; }
"str"         { return STR_TYPE; }

"int"         { return INT_TYPE; }
"flt"         { return FLT_TYPE; }
"chr"         { return CHR_TYPE; }


[a-zA-Z][a-zA-Z0-9_]* { yylval.idName = strdup(yytext); return ID; }
[0-9]+\.[0-9]+ { yylval.floatVal = atof(yytext); return FLOAT_LITERAL; }
[0-9]+        { yylval.intVal = atoi(yytext); return INTEGER_LITERAL;}
"'"[^\n\'\\]"'" { yylval.intVal = yytext[1]; return CHAR_LITERAL; }
\"[^\n"]*\"   { int content_length = yyleng -2; char *content = (char*)malloc(content_length + 1); 
      strncpy(content,yytext+1,content_length); content[content_length]= '\0'; yylval.idName = content; return STRING_LITERAL; }
      
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
