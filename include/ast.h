#ifndef AST_H
#define AST_H

#include <stdio.h>
#include <stdlib.h>

enum operators {
	oper_add,
	oper_sub,
	oper_mult,
	oper_div,
	oper_equal,
	oper_notequal,
	oper_band,
	oper_bor,
	oper_and,
	oper_or,
	oper_xor,
	oper_ref,
	oper_deref,
	oper_assign,
	oper_incpost,
	oper_incpre,
	oper_decpost,
	oper_decpre,
	oper_lt,
	oper_lte,
	oper_gt,
	oper_gte,
	oper_neg
};

enum type_names {
	//Void
	type_void,
	//Integer Types
	type_signed_char,
	type_unsigned_char,

	type_signed_short,
	type_unsigned_short,

	type_signed_int,
	type_unsigned_int,

	type_signed_long,
	type_unsigned_long,

	//Floating Point Types
	type_float,
	type_double,
	type_long_double
};

typedef struct t_expr t_expr;
typedef struct t_ident t_ident;
typedef struct t_binop t_binop;
typedef struct t_unop t_unop;
typedef struct t_numeric t_numeric;
typedef struct t_call t_call;
typedef struct t_declr t_declr;
typedef struct t_decl_spec t_decl_spec;
typedef struct t_dir_declr t_dir_declr;
typedef struct t_block t_block;
typedef struct t_func_def t_func_def;
typedef struct t_stmt t_stmt;
typedef struct t_param_list t_param_list;
typedef struct t_conditional_stmt t_conditional_stmt;
typedef struct t_iterative_stmt t_iterative_stmt;
typedef struct t_decl_list t_decl_list;
typedef struct t_external_def t_external_def;
typedef struct t_trans_unit t_trans_unit;
typedef struct t_jump t_jump;
typedef struct t_expr_list t_expr_list;

struct t_trans_unit {
	t_external_def **definitions;
	int num_def;
};

t_trans_unit *t_trans_unit_init(t_external_def * def);
t_trans_unit *t_trans_unit_add(t_trans_unit * trans, t_external_def * def);
void t_trans_unit_destroy(t_trans_unit * trans);

struct t_external_def {
	union {
		t_func_def *func;
		t_expr *declaration;
	};
	int type;
};

t_external_def *t_external_def_init0(t_func_def * func);
t_external_def *t_external_def_init1(t_expr * decl);
void t_external_def_destroy(t_external_def * def);

struct t_call {
	t_expr * func;
	t_expr **expr_list;
	int num_expr;
};

t_call *t_call_init(t_expr *func, t_expr_list *list);
void t_call_destroy(t_call * call);

struct t_expr_list {
	t_expr **expr_list;
	int num_expr;
};

t_expr_list *t_expr_list_init(t_expr * expr);
t_expr_list *t_expr_list_add(t_expr_list *expr_list, t_expr *expr);

struct t_func_def {
	t_decl_spec *decl_spec;
	t_decl_spec **decl_list;
	int num_param;
	t_block *block;
};

t_func_def *t_func_def_init(t_decl_spec * decl_spec, t_decl_list * dlist,
			    t_block * block);
void t_func_def_destroy(t_func_def * func);

struct t_decl_list {
	t_expr **decls;
	int num_decls;
};

t_decl_list *t_decl_list_init(t_expr * decl);
t_decl_list *t_decl_list_add(t_decl_list * dlist, t_expr * decl);
void t_decl_list_destroy(t_decl_list * dlist);

struct t_conditional_stmt {
	t_expr *condition;
	t_block *block;
	t_block *otherwise;
};

t_conditional_stmt *t_conditional_stmt_init(t_expr * condition, t_block * block,
					    t_block * otherwise);

struct t_iterative_stmt {
	t_block *block;

	t_expr *init;
	t_expr *cond;
	t_expr *iter;

	int first;

	int type;
};

t_iterative_stmt *t_iterative_stmt_init0(t_expr * init, t_expr * cond,
					 t_expr * iter, t_block * block);
t_iterative_stmt *t_iterative_stmt_init1(t_expr * cond, t_block * block,
					 int first);

struct t_ident {
	char *ident;
	//Type information to be filled with later steps
	t_decl_spec *decl_spec;
};

t_ident *t_ident_init(char *ident);

struct t_binop {
	t_expr *lhs;
	int op;
	t_expr *rhs;
};

t_binop *t_binop_init(t_expr * lhs, int op, t_expr * rhs);

struct t_unop {
	int op;
	t_expr *term;
};

t_unop *t_unop_init(int op, t_expr * s);

struct t_expr {
	union {
		t_ident *ident;
		t_numeric *cnumeric;
		t_binop *binop;
		t_decl_spec *decl_spec;
		t_unop *unop;
		char *cstring;
		t_call * call;
	};
	struct t_expr *next;
	int type;
	//For Code Generation
	void *truelist, *falselist;
	int virt_reg;
	int num_ptr;
	int type_name;
};

t_expr *t_expr_init0(t_ident * ident);
t_expr *t_expr_init1(t_numeric * num);
t_expr *t_expr_init2(t_expr * lhs, int op, t_expr * rhs);
t_expr *t_expr_init3(t_decl_spec * decl_spec);
t_expr *t_expr_init4(t_expr * term, int oper);
t_expr *t_expr_init5(char *string);
t_expr *t_expr_init6(t_call * call);
t_expr *t_expr_add(t_expr * old, t_expr * next);
void t_expr_destroy(t_expr * expr);

struct t_numeric {
	union {
		long long int cint;
		double cdouble;
	};
	int type;
};

t_numeric *t_numeric_init0(char *cint);
t_numeric *t_numeric_init1(char *cdouble);
t_numeric *t_numeric_init2(int cint);

void t_numeric_destroy(t_numeric * num);

struct t_decl_spec {
	int type_name;
	t_declr *declarator;
};

t_decl_spec *t_decl_spec_init(int type, t_declr * decl);
void t_decl_spec_destroy(t_decl_spec * decl_spec);

struct t_declr {
	int ptr;
	t_dir_declr *ddecl;
};

t_declr *t_declr_init(int n_ptr, t_dir_declr * ddecl);
void t_declr_destroy(t_declr * decl);

struct t_dir_declr {
	union {
		t_ident *ident;
		t_declr *decl;
	};
	int type;
};

t_dir_declr *t_dir_declr_init0(t_ident * ident);
t_dir_declr *t_dir_declr_init1(t_declr * decl);
void t_dir_declr_destroy(t_dir_declr * ddecl);

struct t_block {
	t_stmt **statements;
	int num_statements;
	void *truelist, *falselist;
};

t_block *t_block_init(t_stmt * stmt);
t_block *t_block_add(t_block * block, t_stmt * statement);
t_block *t_block_merge(t_block * b1, t_block * b2);
void t_block_destroy(t_block * block);

struct t_stmt {
	union {
		t_block *block;
		t_decl_spec *declaration;
		t_expr *expression;
		t_conditional_stmt *cstmt;
		t_iterative_stmt *itstmt;
		t_jump *jump;
	};
	int type;
	void *truelist, *falselist;
};

t_stmt *t_stmt_init0(t_block * block);
t_stmt *t_stmt_init1(t_decl_spec * declaration);
t_stmt *t_stmt_init2(t_expr * expr);
t_stmt *t_stmt_init3(t_conditional_stmt * cstmt);
t_stmt *t_stmt_init4(t_iterative_stmt * itstmt);
t_stmt *t_stmt_init5(t_jump * jump);
void t_stmt_destroy(t_stmt * stmt);

struct t_jump {
	union {
		t_expr *retval;
	};
	int type;
};

t_jump *t_jump_init0();
t_jump *t_jump_init1();
t_jump *t_jump_init2(t_expr * retval);

void t_jump_destroy(t_jump * jump);

/*Print Functions*/
void t_block_print(t_block * block);
void t_stmt_print(t_stmt * statement);
void t_expr_print(t_expr * expr);
void t_num_print(t_numeric * num);
void t_binop_print(t_binop * bin);
void t_unop_print(t_unop * unop);
void t_decl_spec_print(t_decl_spec * decl_spec);
void t_declr_print(t_declr * declr);
void t_dir_declr_print(t_dir_declr * ddeclr);

#endif
