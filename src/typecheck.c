#include "../include/typecheck.h"

void t_block_check(symbol_table * symt, t_block * block)
{
	if (!block)
		return;

	for (int i = (block->num_statements - 1); i >= 0; i--) {
		t_stmt_check(symt, block->statements[i]);
	}
}

void t_stmt_check(symbol_table * symt, t_stmt * statement)
{

	if (!statement)
		return;
	switch (statement->type) {
	case 0:
		t_block_check(symt, statement->block);
		break;
	case 1:
		//block_ctx_add_decl(ctx, statement->declaration);
		break;
	case 2:
		t_expr_check(symt, statement->expression);
		break;
	case 3:		//if statements
		t_expr_check(symt, statement->cstmt->condition);
		t_block_check(symt, statement->cstmt->block);
		if (statement->cstmt->otherwise) {
			t_block_check(symt, statement->cstmt->otherwise);
		}
		break;
	case 4:
		;
		//for loop
		if (statement->itstmt->type == 0) {
			t_expr_check(symt, statement->itstmt->init);
			t_block_check(symt, statement->itstmt->block);
			t_expr_check(symt, statement->itstmt->iter);
			t_expr_check(symt, statement->itstmt->cond);
		} else {
			t_block_check(symt, statement->itstmt->block);
			t_expr_check(symt, statement->itstmt->cond);
		}
	}
}

void t_expr_check(symbol_table * symt, t_expr * expr)
{
	if (!expr)
		return;

	if (expr->type == 2) {
		if ((expr->binop->lhs->type != 2 && expr->binop->lhs->type != 4)
		    && (expr->binop->rhs->type != 2
			&& expr->binop->rhs->type != 4)) {
			t_expr_set_typeinfo(symt, expr->binop->lhs);
			t_expr_set_typeinfo(symt, expr->binop->rhs);
		} else if (expr->binop->lhs->type != 2
			   && expr->binop->lhs->type != 4) {
			t_expr_check(symt, expr->binop->rhs);
			t_expr_set_typeinfo(symt, expr->binop->lhs);
		} else if (expr->binop->rhs->type != 2
			   && expr->binop->rhs->type != 4) {
			t_expr_check(symt, expr->binop->lhs);
			t_expr_set_typeinfo(symt, expr->binop->rhs);
		} else {
			t_expr_check(symt, expr->binop->lhs);
			t_expr_check(symt, expr->binop->rhs);
		}

		if (0
		    && expr->binop->lhs->type_name !=
		    expr->binop->rhs->type_name) {
			print_expr_type(expr->binop->lhs);
			print_expr_type(expr->binop->rhs);
			printf("Type Error\n");
			exit(1);
		} else if (0 && expr->binop->lhs->num_ptr != expr->binop->rhs->num_ptr && (expr->binop->op != oper_add)) {	//Pointers addition with integer is legal
			print_expr_type(expr->binop->lhs);
			print_expr_type(expr->binop->rhs);
			printf("Invalid Pointer Arithmetic\n");
			exit(1);
		}
		//Both children should have the same typeinfo so use the left
		expr->num_ptr =
		    expr->binop->lhs->num_ptr >
		    expr->binop->rhs->num_ptr ? expr->binop->lhs->
		    num_ptr : expr->binop->rhs->num_ptr;
		expr->type_name = expr->binop->lhs->type_name;

	} else if (expr->type == 4) {
		if (expr->unop->term->type != 2 && expr->unop->term->type != 4) {
			//Assume constants are integers for now
			t_expr_set_typeinfo(symt, expr->unop->term);
		} else {
			t_expr_check(symt, expr->unop->term);
		}
		//Cascade type information up tree
		expr->num_ptr = expr->unop->term->num_ptr;
		if (expr->unop->op == oper_ref)
			expr->num_ptr++;
		else if (expr->unop->op == oper_deref)
			expr->num_ptr--;
		expr->type_name = expr->unop->term->type_name;
	}
}

void t_expr_set_typeinfo(symbol_table * symt, t_expr * leaf)
{
	if (leaf->type == 1) {
		leaf->type_name = type_signed_int;
	} else if (leaf->type == 0) {
		symbol *sym = symbol_table_lookup(symt, leaf->ident->ident);
		leaf->num_ptr += sym->type.num_ptr;
		leaf->type_name = sym->type.type_name;
		//print_expr_type(leaf);
	} else if (leaf->type == 3) {
		symbol *sym =
		    symbol_table_lookup(symt, get_decl_name(leaf->decl_spec));
		leaf->num_ptr += sym->type.num_ptr;
		leaf->type_name = sym->type.type_name;
		//print_expr_type(leaf);
	}
}

void print_expr_type(t_expr * expr)
{
	printf("Typeinfo: ptr:%d ", expr->num_ptr);
	if (expr->type == 0)
		printf("name: %s ", expr->ident->ident);
	print_type(expr->type_name);
	printf("\n");
}