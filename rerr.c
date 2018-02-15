#include "rerr.h"



/*
struct t_decl_spec
{
	int type_name;
	t_declr *declarator;
};

t_decl_spec *t_decl_spec_init(int type, t_declr *decl);

struct t_declr {
	int ptr;
	t_dir_declr *ddecl;
};

t_declr *t_declr_init(int n_ptr, t_dir_declr *ddecl);

struct t_dir_declr
{
	union {
		t_ident *ident;
		t_declr *decl;
	};
	int type;
};
*/


int compatible_decl(t_decl_spec *d1, t_decl_spec *d2)
{
	if (!(d1 && d2)) return 0;

	if (d1->type_name == d2->type_name) return 1;
	/*
	t_declr *declor1 = NULL;
	t_dir_declr *ddecl1 = NULL;
	int type1 = 0;

	t_declr *declor2 = NULL;
	t_dir_declr *ddecl2 = NULL;
	int type2 = 0;
	char * name1 = NULL;
	char * name2 = NULL;
	while (1) {
		if (type1==0) {
			declor1 = decl1->declarator;
			type1 = 1;
		} else if (type1 == 1) {
			ddecl1 = declor1->ddecl;
			type1 = 2;
		} else {
			if (ddecl1->type) {
				declor1 = ddecl1->decl;
				type1 = 1;
			} else {
				
			}
		}
	}
	*/
	return 0;
}

void t_block_check(block_ctx *p, t_block *block)
{
	if (!block) return;
	block_ctx *ctx = block_ctx_init(p);
	for (int i = (block->num_statements-1); i >= 0; i--) {
		t_stmt_check(ctx, block->statements[i]);
		printf("\n");
	}
}

void t_stmt_check(block_ctx *ctx, t_stmt *statement)
{
	if (!statement) return;
	switch (statement->type) {
		case 0:
			t_block_check(ctx, statement->block);
			break;
		case 1:
			block_ctx_add_decl(ctx, statement->declaration);
			//t_decl_spec_print(statement->declaration);
			break;
		case 2:
			t_expr_check(ctx, statement->expression);
			break;
	}
}

t_decl_spec *t_expr_check(block_ctx *ctx, t_expr *expr)
{
	if (!expr) return NULL;
	t_decl_spec * d1 = NULL;
	t_decl_spec * d2 = NULL;

	switch (expr->type) {
		case 0:
			if (!expr->ident->decl_spec)
				expr->ident->decl_spec = block_ctx_get_decl(ctx, expr->ident->ident);
			if (!expr->ident->decl_spec) {
				printf("Syntax Error: ");
				printf("Undefined identifier %s\n", expr->ident->ident);
				exit(1);
			}
			return expr->ident->decl_spec;
			break;
		case 1:
			//t_num_print(expr->cnumeric);
			break;
		case 2:
			d1 = t_expr_check(ctx, expr->binop->lhs);
			d2 = t_expr_check(ctx, expr->binop->rhs);
			if (d1 && d2 && !compatible_decl(d1, d2)) {
				printf("Syntax Error: ");
				printf("Declaration ");
				t_decl_spec_print(d1);
				printf(" is not compatible with ");
				t_decl_spec_print(d2);
				printf("\n");
				exit(1);
			}
			return d1 && d2 ? d1 : d2;
			break;
		case 3:
			block_ctx_add_decl(ctx, expr->decl_spec);
			return expr->decl_spec;
			break;
	}
	return NULL;
}
