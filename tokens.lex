%{
	#include <stdio.h>
	#include "ast.h"
	#include "parser.h"
	#define SAVE_TOKEN yylval.string = strndup(yytext, yyleng)
	#define TOKEN(t) (yylval.token = t)
	int yywrap(void) {
		return 1;
	}
%}

%%
[ \t\n]			;
[0-9]+			SAVE_TOKEN; return T_CINT;
[0-9]+.[0-9]+	SAVE_TOKEN; return T_CDOUBLE;
"if"			return TOKEN(T_IF);
"else"			return TOKEN(T_ELSE);
"for"			return TOKEN(T_FOR);
"while"			return TOKEN(T_WHILE);
"void"			return TOKEN(T_VOID);
"char"			return TOKEN(T_CHAR);
"short"			return TOKEN(T_SHORT);
"int"			return TOKEN(T_INT);
"long"			return TOKEN(T_LONG);
"signed"		return TOKEN(T_SIGNED);
"unsigned"		return TOKEN(T_UNSIGNED);
[a-zA-Z_][a-zA-Z0-9_]*	SAVE_TOKEN; return T_IDENT;
"->"		return TOKEN(T_PTRMEM);
"["			return TOKEN(T_LBRCK);
"]"			return TOKEN(T_RBRCK);
"&"			return TOKEN(T_BAND);
"="			return TOKEN(T_ASN);
"=="		return TOKEN(T_EQ);
"++"		return TOKEN(T_INC);
"--"		return TOKEN(T_DEC); 
"!="		return TOKEN(T_NEQ);
"<"			return TOKEN(T_LT);
"<="		return TOKEN(T_LTE);
"("			return TOKEN(T_LPAREN);
")"			return TOKEN(T_RPAREN);
"{"			return TOKEN(T_LCBRK);
"}"			return TOKEN(T_RCBRK);
"."			return TOKEN(T_DOT);
","			return TOKEN(T_COMMA);
"*"			return TOKEN(T_MUL);
"/"			return TOKEN(T_DIV);
"+"			return TOKEN(T_PLUS);
"-"			return TOKEN(T_SUB);
";"			return TOKEN(T_SEMIC);
.			printf("Unknown Token\n"); yyterminate();

%%
