%{
	#include <stdio.h>
	#include "../include/ast.h"
	#include "../include/symtable.h"

	#define YYERROR_VERBOSE

	t_trans_unit *tunit;
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
	t_iterative_stmt *itstmt;
	t_func_def *func;
	t_decl_list *dlist;
	t_block *block;
	t_external_def *externdef;
	t_stmt *statement;
	t_jump *jmp;
	char *string;
	int token;
	int type;
	int oper;
	int ptr;
}

%token <string> T_IDENT T_CINT T_CDOUBLE T_CLIT T_CSTR
%token <token> T_ASN T_EQ T_NEQ T_LT T_LTE T_LPAREN
%token <token> T_RPAREN T_LCBRK T_RCBRK T_DOT T_COMMA
%token <token> T_MUL T_DIV T_PLUS T_SUB T_PTRMEM
%token <token> T_INC T_DEC T_LBRCK T_RBRCK T_GTE T_GT
%token <token> T_SEMIC T_VOID T_CHAR T_SHORT T_INT T_LONG
%token <token> T_SIGNED T_UNSIGNED T_FLOAT T_DOUBLE
%token <token> T_IF T_ELSE T_FOR T_WHILE T_DO
%token <token> T_BAND T_AND T_BOR T_OR T_XOR
%token <token> T_BREAK T_CONTINUE T_RETURN

%type <cnumeric> const
%type <expr> primary expr eq_expr add_expr mul_expr asn_expr declaration unary postfix expr_stmt rel_expr or_expr and_expr xor_expr bor_expr band_expr
%type <ident> ident
%type <oper> asn_op uni_op
%type <type> type_spec
%type <ptr> pointer
%type <declr> declr
%type <ddeclr> dir_declr
%type <decl_spec> decl_spec
%type <block> cmpd_stmt stmt_list
%type <statement> stmt
%type <cstmt> cond_stmt
%type <itstmt> iter_stmt
%type <dlist> decl_list
%type <func> func_def
%type <externdef> external_def
%type <jmp> jmp_stmt

/*Operator Precedence To Resolve Conflicts*/
%left T_ASN
%left T_EQ T_NEQ
%left T_PLUS T_SUB
%left T_MUL T_DIV
%right T_LCBRK T_RCBRK
%right T_IF T_ELSE


%start trans_unit

%%

trans_unit
	: external_def { tunit = t_trans_unit_init($1); }	
	| trans_unit external_def { tunit = t_trans_unit_add(tunit, $2); }
	;

external_def
	: func_def { $$ = t_external_def_init0($1); }
	| declaration { $$ = t_external_def_init1($1); }
	;


decl_list
	: %empty { $$ = 0; } 
	| declaration { $$ = t_decl_list_init($1); }
	| decl_list T_COMMA  declaration { $$ = t_decl_list_add($1, $3); }
	;

func_def
	: decl_spec T_LPAREN decl_list T_RPAREN cmpd_stmt { $$ = t_func_def_init($1, $3, $5); }
	;

stmt_list
	: stmt { $$ = t_block_init($1); }
	| stmt stmt_list { $$ = t_block_add($2, $1); }
	;

cmpd_stmt
	: T_LCBRK T_RCBRK { $$ = 0; }
	| T_LCBRK stmt_list T_RCBRK { $$ = $2; }
	;

stmt
	: cmpd_stmt { $$ = t_stmt_init0($1); }
	| expr_stmt { $$ = t_stmt_init2($1); }
	| cond_stmt { $$ = t_stmt_init3($1); }
	| iter_stmt { $$ = t_stmt_init4($1); }
	| jmp_stmt  { $$ = t_stmt_init5($1); }
	;

jmp_stmt
	: T_CONTINUE T_SEMIC { $$ = t_jump_init0(); }
	| T_BREAK T_SEMIC { $$ = t_jump_init1(); }
	| T_RETURN T_SEMIC { $$ = t_jump_init2(NULL); }
	| T_RETURN expr T_SEMIC { $$ = t_jump_init2($2); }
	;

declaration
	: decl_spec { $$ = t_expr_init3($1); }
	| decl_spec T_ASN asn_expr T_SEMIC { $$ = t_expr_init2(t_expr_init3($1), oper_assign, $3); }
	;

iter_stmt
	: T_FOR T_LPAREN expr_stmt expr_stmt expr T_RPAREN stmt { $$ = t_iterative_stmt_init0($3, $4, $5, t_block_init($7)); }
	| T_FOR T_LPAREN expr_stmt expr_stmt T_RPAREN stmt { $$ = t_iterative_stmt_init0($3, $4, NULL, t_block_init($6)); }
	| T_WHILE T_LPAREN expr T_RPAREN stmt { $$ = t_iterative_stmt_init1($3, t_block_init($5), 0); }
	| T_DO stmt T_WHILE T_LPAREN expr T_RPAREN T_SEMIC { $$ = t_iterative_stmt_init1($5, t_block_init($2), 1); }
	;

cond_stmt
	: T_IF T_LPAREN asn_expr T_RPAREN stmt { $$ = t_conditional_stmt_init($3, t_block_init($5), NULL); }
	| T_IF T_LPAREN asn_expr T_RPAREN stmt T_ELSE stmt { $$ = t_conditional_stmt_init($3, t_block_init($5), t_block_init($7)); }
	;

expr_stmt
	: T_SEMIC { $$ = 0; }
	| expr T_SEMIC { $$ = $1; }
	| declaration { $$ = $1; }
	;

expr
	: asn_expr { $$=$1; }
	| expr T_COMMA asn_expr
	;

asn_expr
	: %empty { $$ = 0; }
	| or_expr
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
	| postfix T_LBRCK expr T_RBRCK { $$ = t_expr_init4(t_expr_init2($1, oper_add, $3), oper_deref); }
	| postfix T_INC { $$ = t_expr_init4($1, oper_incpost); } 
	| postfix T_DEC { $$ = t_expr_init4($1, oper_decpost); }
	;

asn_op
	: T_ASN { $$ = oper_assign; }
	;

primary
	: ident { $$ = t_expr_init0($1); }
	| const { $$ = t_expr_init1($1); }
	| T_CSTR { $$ = t_expr_init5($1); }
	| T_LPAREN or_expr T_RPAREN { $$ = $2; }
	;

ident
	: T_IDENT { $$ = t_ident_init($1); }
	;

const
	: T_CINT { $$ = t_numeric_init0($1); }
	| T_CDOUBLE { $$ = t_numeric_init1($1); }
	| T_CLIT { $$ = t_numeric_init2($1[1]); }
	;

band_expr
	: eq_expr
	| band_expr T_BAND eq_expr { $$ = t_expr_init2($1, oper_band,$3); }
	;

xor_expr
	: band_expr
	| xor_expr T_XOR band_expr { $$ = t_expr_init2($1, oper_xor, $3); }
	;

bor_expr
	: xor_expr { $$ = $1; }
	| bor_expr T_BOR xor_expr { $$ = t_expr_init2($1, oper_bor, $3); }
	;

and_expr
	: bor_expr { $$ = $1; }
	| and_expr T_AND bor_expr { $$ = t_expr_init2($1, oper_and, $3); }
	;

or_expr
	: and_expr { $$ = $1; }
	| or_expr T_OR and_expr { $$ = t_expr_init2($1, oper_or, $3); }
	;

rel_expr
	: add_expr
	| rel_expr T_LT add_expr { $$ = t_expr_init2($1, oper_lt, $3); }
	| rel_expr T_GT add_expr { $$ = t_expr_init2($1, oper_gt, $3); }
	| rel_expr T_LTE add_expr { $$ = t_expr_init2($1, oper_lte, $3); }
	| rel_expr T_GTE add_expr { $$ = t_expr_init2($1, oper_gte, $3); }
	;

eq_expr
	: rel_expr
	| eq_expr T_EQ rel_expr { $$ = t_expr_init2($1, oper_equal, $3); }
	| eq_expr T_NEQ rel_expr { $$ = t_expr_init2($1, oper_notequal, $3); }
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
	: type_spec declr { $$ = t_decl_spec_init($1, $2); }
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
	//| dir_declr T_LPAREN T_RPAREN { $$ = $1;}
	;

%%
