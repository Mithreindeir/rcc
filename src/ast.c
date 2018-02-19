#include "../include/ast.h"

t_func_def *t_func_def_init(t_decl_spec * decl_spec, t_decl_list * dlist,
			    t_block * block)
{
	t_func_def *func = malloc(sizeof(t_func_def));

	func->decl_spec = decl_spec;
	func->block = block;
	func->num_param = 0;
	func->decl_list = NULL;

	if (dlist) {
		func->num_param = dlist->num_decls;
		func->decl_list =
		    malloc(sizeof(t_decl_spec *) * func->num_param);
		for (int i = 0; i < dlist->num_decls; i++) {
			if (dlist->decls[i]->type == 3) {
				func->decl_list[i] = dlist->decls[i]->decl_spec;
				dlist->decls[i]->decl_spec = NULL;
				dlist->decls[i]->type = -1;
			} else {
				printf("Fatal error\n");
				exit(1);
			}

		}

		t_decl_list_destroy(dlist);
	}
	return func;
}

void t_func_def_destroy(t_func_def * func)
{
	if (!func)
		return;

	t_block_destroy(func->block);
	t_decl_spec_destroy(func->decl_spec);

	for (int i = 0; i < func->num_param; i++) {
		t_decl_spec_destroy(func->decl_list[i]);
	}

	free(func->decl_list);
	free(func);
}

t_decl_list *t_decl_list_init(t_expr * decl)
{
	t_decl_list *dlist = malloc(sizeof(t_decl_list));

	dlist->decls = malloc(sizeof(t_expr *));
	dlist->num_decls = 1;
	dlist->decls[0] = decl;

	return dlist;
}

t_decl_list *t_decl_list_add(t_decl_list * dlist, t_expr * decl)
{
	if (!decl)
		return dlist;
	dlist->num_decls++;
	dlist->decls =
	    realloc(dlist->decls, sizeof(t_expr *) * dlist->num_decls);
	dlist->decls[dlist->num_decls - 1] = decl;

	return dlist;
}

void t_decl_list_destroy(t_decl_list * dlist)
{
	if (!dlist)
		return;

	for (int i = 0; i < dlist->num_decls; i++) {
		t_expr_destroy(dlist->decls[i]);
	}

	free(dlist->decls);
	free(dlist);
}

t_call *t_call_init(t_ident * ident, t_expr ** expr_l, int num_expr)
{
	t_call *call = malloc(sizeof(t_call));

	call->ident = ident;
	call->expr_list = expr_l;
	call->num_expr = num_expr;

	return call;
}

t_conditional_stmt *t_conditional_stmt_init(t_expr * condition, t_block * block,
					    t_block * otherwise)
{
	t_conditional_stmt *cstmt = malloc(sizeof(t_conditional_stmt));

	cstmt->condition = condition;
	cstmt->block = block;
	cstmt->otherwise = otherwise;

	return cstmt;
}

t_iterative_stmt *t_iterative_stmt_init0(t_expr * init, t_expr * cond,
					 t_expr * iter, t_block * block)
{
	t_iterative_stmt *its = malloc(sizeof(t_iterative_stmt));

	its->init = init;
	its->cond = cond;
	its->iter = iter;
	its->block = block;
	its->type = 0;

	return its;
}

t_iterative_stmt *t_iterative_stmt_init1(t_expr * cond, t_block * block,
					 int first)
{
	t_iterative_stmt *its = malloc(sizeof(t_iterative_stmt));

	its->first = first;
	its->cond = cond;
	its->block = block;
	its->type = 1;

	return its;
}

t_ident *t_ident_init(char *ident)
{
	t_ident *idnt = malloc(sizeof(t_ident));

	idnt->ident = ident;
	idnt->decl_spec = NULL;

	return idnt;
}

t_binop *t_binop_init(t_expr * lhs, int op, t_expr * rhs)
{
	t_binop *binop = malloc(sizeof(t_binop));

	binop->lhs = lhs;
	binop->op = op;
	binop->rhs = rhs;

	return binop;
}

t_unop *t_unop_init(int op, t_expr * s)
{
	t_unop *unop = malloc(sizeof(t_unop));

	unop->term = s;
	unop->op = op;

	return unop;
}

t_expr *t_expr_init0(t_ident * ident)
{
	t_expr *expr = malloc(sizeof(t_expr));

	expr->ident = ident;
	expr->type = 0;
	expr->virt_reg = -1;
	expr->num_ptr = 0;
	expr->type_name = 0;
	expr->truelist = NULL;
	expr->falselist = NULL;

	return expr;
}

t_expr *t_expr_init1(t_numeric * num)
{
	t_expr *expr = malloc(sizeof(t_expr));

	expr->cnumeric = num;
	expr->type = 1;
	expr->virt_reg = -1;
	expr->num_ptr = 0;
	expr->type_name = 0;
	expr->truelist = NULL;
	expr->falselist = NULL;

	return expr;
}

t_expr *t_expr_init2(t_expr * lhs, int op, t_expr * rhs)
{
	t_expr *expr = malloc(sizeof(t_expr));

	expr->binop = t_binop_init(lhs, op, rhs);
	expr->type = 2;
	expr->virt_reg = -1;
	expr->num_ptr = 0;
	expr->type_name = 0;
	expr->truelist = NULL;
	expr->falselist = NULL;

	return expr;
}

t_expr *t_expr_init3(t_decl_spec * decl_spec)
{
	t_expr *expr = malloc(sizeof(t_expr));

	expr->decl_spec = decl_spec;
	expr->type = 3;
	expr->virt_reg = -1;
	expr->num_ptr = 0;
	expr->type_name = 0;
	expr->truelist = NULL;
	expr->falselist = NULL;

	return expr;
}

t_expr *t_expr_init4(t_expr * term, int oper)
{
	t_expr *expr = malloc(sizeof(t_expr));

	expr->unop = t_unop_init(oper, term);
	expr->type = 4;
	expr->virt_reg = -1;
	expr->num_ptr = 0;
	expr->type_name = 0;
	expr->truelist = NULL;
	expr->falselist = NULL;

	return expr;
}

t_numeric *t_numeric_init0(char *cint)
{
	t_numeric *n = malloc(sizeof(t_numeric));

	n->cint = atoi(cint);
	n->type = 0;

	return n;
}

t_numeric *t_numeric_init1(char *cdouble)
{
	t_numeric *n = malloc(sizeof(t_numeric));

	n->cdouble = atof(cdouble);
	n->type = 1;

	return n;
}

t_numeric *t_numeric_init2(int cint)
{
	t_numeric *n = malloc(sizeof(t_numeric));

	n->cint = cint;
	n->type = 0;

	return n;
}

t_decl_spec *t_decl_spec_init(int type, t_declr * decl)
{
	t_decl_spec *decl_spec = malloc(sizeof(t_decl_spec));

	decl_spec->type_name = type;
	decl_spec->declarator = decl;

	return decl_spec;
}

t_declr *t_declr_init(int n_ptr, t_dir_declr * ddecl)
{
	t_declr *declr = malloc(sizeof(t_declr));

	declr->ptr = n_ptr;
	declr->ddecl = ddecl;

	return declr;
}

t_dir_declr *t_dir_declr_init0(t_ident * ident)
{
	t_dir_declr *ddecl = malloc(sizeof(t_dir_declr));

	ddecl->ident = ident;
	ddecl->type = 0;

	return ddecl;
}

t_dir_declr *t_dir_declr_init1(t_declr * decl)
{
	t_dir_declr *ddecl = malloc(sizeof(t_dir_declr));

	ddecl->decl = decl;
	ddecl->type = 1;

	return ddecl;
}

t_block *t_block_init(t_stmt * stmt)
{
	t_block *block = malloc(sizeof(t_block));

	block->statements = NULL;
	block->num_statements = 0;
	if (stmt)
		t_block_add(block, stmt);

	return block;
}

t_block *t_block_add(t_block * block, t_stmt * statement)
{
	block->num_statements++;
	if (block->num_statements == 1) {
		block->statements = malloc(sizeof(t_stmt *));
	} else {
		block->statements =
		    realloc(block->statements,
			    sizeof(t_stmt *) * block->num_statements);
	}
	block->statements[block->num_statements - 1] = statement;

	return block;
}

t_block *t_block_merge(t_block * b1, t_block * b2)
{
	int olds = b1->num_statements;
	b1->num_statements += b2->num_statements;
	b1->statements =
	    realloc(b1->statements, sizeof(t_stmt *) * b1->num_statements);

	for (int i = 0; i < b2->num_statements; i++) {
		b1->statements[olds + i] = b2->statements[i];
	}

	free(b2->statements);
	free(b2);
	return b1;
}

t_stmt *t_stmt_init0(t_block * block)
{
	t_stmt *statement = malloc(sizeof(t_stmt));

	statement->type = 0;
	statement->block = block;

	return statement;
}

t_stmt *t_stmt_init1(t_decl_spec * declaration)
{
	t_stmt *statement = malloc(sizeof(t_stmt));

	statement->type = 1;
	statement->declaration = declaration;

	return statement;
}

t_stmt *t_stmt_init2(t_expr * expr)
{
	t_stmt *statement = malloc(sizeof(t_stmt));

	statement->type = 2;
	statement->expression = expr;

	return statement;
}

t_stmt *t_stmt_init3(t_conditional_stmt * cstmt)
{
	t_stmt *statement = malloc(sizeof(t_stmt));

	statement->type = 3;
	statement->cstmt = cstmt;

	return statement;
}

t_stmt *t_stmt_init4(t_iterative_stmt * itstmt)
{
	t_stmt *statement = malloc(sizeof(t_stmt));

	statement->type = 4;
	statement->itstmt = itstmt;

	return statement;
}

/*Printing Functions*/

void t_block_print(t_block * block)
{
	if (!block)
		return;
	for (int i = (block->num_statements - 1); i >= 0; i--) {
		t_stmt_print(block->statements[i]);
		printf("\n");
	}
}

void t_stmt_print(t_stmt * statement)
{
	if (!statement)
		return;
	switch (statement->type) {
	case 0:
		t_block_print(statement->block);
		break;
	case 1:
		t_decl_spec_print(statement->declaration);
		break;
	case 2:
		t_expr_print(statement->expression);
		break;
	}
}

void t_expr_print(t_expr * expr)
{
	if (!expr)
		return;

	switch (expr->type) {
	case 0:
		printf("%s", expr->ident->ident);
		break;
	case 1:
		t_num_print(expr->cnumeric);
		break;
	case 2:
		t_binop_print(expr->binop);
		break;
	case 3:
		t_decl_spec_print(expr->decl_spec);
		break;
	case 4:
		t_unop_print(expr->unop);
		break;
	}
}

void t_binop_print(t_binop * bin)
{
	if (!bin)
		return;

	t_expr_print(bin->lhs);
	switch (bin->op) {
	case oper_assign:
		printf("=");
		break;
	case oper_add:
		printf("+");
		break;
	case oper_sub:
		printf("-");
		break;
	case oper_mult:
		printf("*");
		break;
	case oper_div:
		printf("/");
	}
	t_expr_print(bin->rhs);
}

void t_unop_print(t_unop * unop)
{
	if (!unop)
		return;

	switch (unop->op) {
	case oper_incpost:
		printf("++");
		break;
	case oper_incpre:
		printf("++");
		break;
	case oper_decpost:
		printf("--");
		break;
	case oper_decpre:
		printf("--");
		break;
	case oper_neg:
		printf("-");
		break;
	}
	t_expr_print(unop->term);

}

void t_num_print(t_numeric * num)
{
	if (!num)
		return;

	if (num->type == 0)
		printf("%lld", num->cint);
	else
		printf("%f", num->cdouble);
}

void t_decl_spec_print(t_decl_spec * decl_spec)
{
	switch (decl_spec->type_name) {
	case type_signed_int:
		printf("int ");
		break;
	case type_unsigned_int:
		printf("unsigned int ");
		break;
	case type_signed_char:
		printf("char ");
		break;
	case type_unsigned_char:
		printf("unsigned char ");
		break;
	case type_unsigned_long:
		printf("unsigned long ");
		break;
	case type_signed_long:
		printf("long ");
		break;
	case type_signed_short:
		printf("short ");
		break;
	case type_unsigned_short:
		printf("unsigned short ");
		break;
	case type_float:
		printf("float ");
		break;
	case type_double:
		printf("double ");
		break;
	}
	t_declr_print(decl_spec->declarator);
}

void t_declr_print(t_declr * declr)
{
	if (!declr)
		return;

	for (int i = 0; i < declr->ptr; i++)
		printf("*");
	t_dir_declr_print(declr->ddecl);

}

void t_dir_declr_print(t_dir_declr * ddeclr)
{
	if (!ddeclr)
		return;
	if (ddeclr->type == 0)
		printf("%s", ddeclr->ident->ident);
	else
		t_declr_print(ddeclr->decl);
}

void t_conditional_stmt_destroy(t_conditional_stmt * cstmt)
{
	if (!cstmt)
		return;

	t_expr_destroy(cstmt->condition);
	t_block_destroy(cstmt->block);
	t_block_destroy(cstmt->otherwise);

	free(cstmt);
}

void t_iterative_stmt_destroy(t_iterative_stmt * itstmt)
{
	if (!itstmt)
		return;

	t_block_destroy(itstmt->block);
	t_expr_destroy(itstmt->init);
	t_expr_destroy(itstmt->cond);
	t_expr_destroy(itstmt->iter);

	free(itstmt);
}

void t_ident_destroy(t_ident * ident)
{
	if (!ident)
		return;

	t_decl_spec_destroy(ident->decl_spec);

	free(ident->ident);
	free(ident);
}

void t_binop_destroy(t_binop * binop)
{
	if (!binop)
		return;

	t_expr_destroy(binop->lhs);
	t_expr_destroy(binop->rhs);

	free(binop);
}

void t_unop_destroy(t_unop * unop)
{
	if (!unop)
		return;

	t_expr_destroy(unop->term);

	free(unop);
}

void t_expr_destroy(t_expr * expr)
{
	if (!expr)
		return;

	if (expr->type == 0)
		t_ident_destroy(expr->ident);
	else if (expr->type == 1)
		t_numeric_destroy(expr->cnumeric);
	else if (expr->type == 2)
		t_binop_destroy(expr->binop);
	else if (expr->type == 3)
		t_decl_spec_destroy(expr->decl_spec);
	else if (expr->type == 4)
		t_unop_destroy(expr->unop);

	free(expr);
}

void t_numeric_destroy(t_numeric * num)
{
	if (!num)
		return;

	free(num);
}

void t_decl_spec_destroy(t_decl_spec * decl_spec)
{
	if (!decl_spec)
		return;

	t_declr_destroy(decl_spec->declarator);

	free(decl_spec);
}

void t_declr_destroy(t_declr * decl)
{
	if (!decl)
		return;

	t_dir_declr_destroy(decl->ddecl);

	free(decl);
}

void t_dir_declr_destroy(t_dir_declr * ddecl)
{
	if (!ddecl)
		return;

	if (ddecl->type == 0)
		t_ident_destroy(ddecl->ident);
	else if (ddecl->type == 1)
		t_declr_destroy(ddecl->decl);

	free(ddecl);
}

void t_block_destroy(t_block * block)
{
	if (!block)
		return;

	for (int i = 0; i < block->num_statements; i++) {
		t_stmt_destroy(block->statements[i]);
	}

	free(block->statements);
	free(block);
}

void t_stmt_destroy(t_stmt * stmt)
{
	if (!stmt)
		return;

	if (stmt->type == 0)
		t_block_destroy(stmt->block);
	else if (stmt->type == 1)
		t_decl_spec_destroy(stmt->declaration);
	else if (stmt->type == 2)
		t_expr_destroy(stmt->expression);
	else if (stmt->type == 3)
		t_conditional_stmt_destroy(stmt->cstmt);
	else if (stmt->type == 4)
		t_iterative_stmt_destroy(stmt->itstmt);

	free(stmt);
}
