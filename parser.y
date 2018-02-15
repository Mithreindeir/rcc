%{
	#include <stdio.h>
	#include "ast.h"
	#include "symtable.h"

	#define YYERROR_VERBOSE
	
	t_block *main_block;
	symbol_table *global_table;

	extern int yylex();
	void yyerror(const char *s) { printf("ERROR: %s\n", s); }
%}

%union {
	t_expr *expr;
	t_ident *ident;
	t_numeric *cnumeric;
	t_declr *declr;
	t_dir_declr *ddeclr;
	t_decl_spec *decl_spec;
	t_conditional_stmt * cstmt;
	t_block *block;
	t_stmt *statement;
	char *string;
	int token;
	int type;
	int oper;
	int ptr;
}

%token <string> T_IDENT T_CINT T_CDOUBLE
%token <token> T_ASN T_EQ T_NEQ T_LT T_LTE T_LPAREN
%token <token> T_RPAREN T_LCBRK T_RCBRK T_DOT T_COMMA
%token <token> T_MUL T_DIV T_PLUS T_SUB
%token <token> T_INC T_DEC
%token <token> T_SEMIC T_VOID T_CHAR T_SHORT T_INT T_LONG
%token <token> T_SIGNED T_UNSIGNED T_FLOAT T_DOUBLE
%token <token> T_IF T_ELSE T_FOR T_WHILE T_BAND


%type <cnumeric> const
%type <expr> primary expr eq_expr add_expr mul_expr asn_expr init unary postfix
%type <ident> ident
%type <oper> asn_op uni_op
%type <type> type_spec
%type <ptr> pointer
%type <declr> declr
%type <ddeclr> dir_declr
%type <decl_spec> decl_spec
%type <block> block
%type <statement> stmt
%type <cstmt> cond_stmt

/*Operator Precedence*/
%left T_ASN
%left T_EQ T_NEQ
%left T_PLUS T_SUB
%left T_MUL T_DIV

%start program

%%

program	
	: block { main_block = $1; }

block
	: stmt { $$ = t_block_init($1); }
	| stmt block { $$ = t_block_add($2, $1); }
	;

stmt
	: init T_SEMIC { $$ = t_stmt_init2($1); }
	| asn_expr T_SEMIC { $$ = t_stmt_init2($1); }
	| T_LCBRK block T_RCBRK { $$ = t_stmt_init0($2); }
	| cond_stmt { $$ = t_stmt_init3($1);  }
	;

cond_stmt
	: T_IF T_LPAREN asn_expr T_RPAREN stmt { $$ = t_conditional_stmt_init($3, t_block_init($5), NULL); }
	| T_IF T_LPAREN asn_expr T_RPAREN stmt T_ELSE stmt { $$ = t_conditional_stmt_init($3, t_block_init($5), t_block_init($7)); }
	;
/*
expr	
	: asn_expr T_SEMIC { $$=$1; }
	//| expr T_COMMA asn_expr
	;
*/

init
	: decl_spec { $$ = t_expr_init3($1); }
	| decl_spec T_ASN asn_expr { $$ = t_expr_init2(t_expr_init3($1), oper_assign, $3); }
	;
	
asn_expr
	: eq_expr
	| unary asn_op asn_expr { $$ = t_expr_init2($1, $2, $3); }
	;

unary
	: postfix { $$ = $1; }
	| T_INC  unary { $$ = t_expr_init4($2, oper_incpre); }
	| T_DEC  unary { $$ = t_expr_init4($2, oper_decpre); }
	| uni_op unary { $$ = t_expr_init4($2, $1); }
	;

uni_op
	: T_SUB { $$ = oper_neg; }
	| T_MUL { $$ = oper_deref; }
	| T_BAND { $$ = oper_ref; }
	;


postfix
	: primary { $$ = $1; }
	| postfix T_INC { $$ = t_expr_init4($1, oper_incpost); }
	| postfix T_DEC { $$ = t_expr_init4($1, oper_decpost); }
	;

asn_op
	: T_ASN { $$ = oper_assign; }
	;

primary
	: ident
		{
			if (!symbol_table_lookup(global_table, $1->ident)) {
				yyerror("Symbol Not Defined\n");
			}
			$$ = t_expr_init0($1);
		}
	| const { $$ = t_expr_init1($1); }
	| T_LPAREN eq_expr T_RPAREN { $$ = $2; }
	;

ident
	: T_IDENT { $$ = t_ident_init($1); }
	;

const
	: T_CINT { $$ = t_numeric_init0($1); }
	| T_CDOUBLE { $$ = t_numeric_init1($1); }
	;

eq_expr
	: add_expr
	| eq_expr T_EQ eq_expr { $$ = t_expr_init2($1, oper_equal, $3); }
	| eq_expr T_NEQ eq_expr { $$ = t_expr_init2($1, oper_notequal, $3); }
	;

add_expr
	: mul_expr
	| add_expr T_PLUS mul_expr { $$ = t_expr_init2($1, oper_add, $3); }
	| add_expr T_SUB mul_expr { $$ = t_expr_init2($1, oper_sub, $3); }
	;

mul_expr
	: unary
	| mul_expr T_MUL unary { $$ = t_expr_init2($1, oper_mult, $3); }
	| mul_expr T_DIV unary { $$ = t_expr_init2($1, oper_div, $3); }
	;

type_spec
	: T_VOID { $$ = type_void; }
	| T_UNSIGNED T_CHAR { $$ = type_unsigned_char; }
	| T_SIGNED T_CHAR { $$ = type_signed_char; }
	| T_CHAR {$$ = type_signed_char; }
	| T_UNSIGNED T_SHORT { $$ = type_unsigned_short;}
	| T_SIGNED T_SHORT { $$ = type_signed_short; }
	| T_SHORT { $$ = type_signed_short; }
	| T_UNSIGNED T_INT { $$ = type_unsigned_int; }
	| T_SIGNED T_INT { $$ = type_signed_int; }
	| T_INT { $$ = type_signed_int; }
	| T_FLOAT { $$ = type_float; }
	| T_DOUBLE { $$ = type_double; }
	;

decl_spec
	: type_spec declr
		{
			$$ = t_decl_spec_init($1, $2);
			if (symbol_table_insert_decl_spec(global_table, $$)) {
				yyerror("Redeclaration\n");
			}
		}
	;

declr
	: pointer dir_declr { $$ = t_declr_init($1, $2); }
	;

pointer
	: %empty { $$ = 0; }
	| T_MUL pointer { $$ = 1 + $2; }
	;

dir_declr
	: ident { $$ = t_dir_declr_init0($1); }
	| T_LPAREN declr T_RPAREN { $$ = t_dir_declr_init1($2); }
	| dir_declr T_LPAREN T_RPAREN { $$ = $1;}
	;

func_def
	: decl_spec T_LCBRK block T_RCBRK {}
	;

%%
