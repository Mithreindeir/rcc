#include "../include/typecheck.h"

void t_trans_unit_check(symbol_table * symt, t_trans_unit * unit)
{
	for (int i = 0; i < unit->num_def; i++) {
		if (unit->definitions[i]->type == 0) {
			t_func_check(symt, unit->definitions[i]->func);
		} else if (unit->definitions[i]->type == 1) {
			t_expr_check(symt, unit->definitions[i]->declaration);
		}
	}
}

void t_func_check(symbol_table * symt, t_func_def * func)
{
	symbol_table_insert_decl_spec(symt, func->decl_spec);

	symt = symbol_table_push(symt);

	for (int i = 0; i < func->num_param; i++) {
		if (!symbol_table_insert_decl_spec(symt, func->decl_list[i])) {
			printf("Redeclaration of %s\n",
			       get_decl_name(func->decl_list[i]));
			exit(1);
		}
	}

	t_block_check(symt, func->block);
	symt = symbol_table_pop(symt);
}

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
		symt = symbol_table_push(symt);
		t_block_check(symt, statement->block);
		symt = symbol_table_pop(symt);
		break;
	case 1:
		//block_ctx_add_decl(ctx, statement->declaration);
		break;
	case 2:
		t_expr_check(symt, statement->expression);
		break;
	case 3:		//if statements
		t_expr_check(symt, statement->cstmt->condition);
		symt = symbol_table_push(symt);
		t_block_check(symt, statement->cstmt->block);
		symt = symbol_table_pop(symt);

		if (statement->cstmt->otherwise) {
			symt = symbol_table_push(symt);
			t_block_check(symt, statement->cstmt->otherwise);
			symt = symbol_table_pop(symt);
		}
		break;
	case 4:
		;
		//for loop
		if (statement->itstmt->type == 0) {
			symt = symbol_table_push(symt);
			t_expr_check(symt, statement->itstmt->init);
			t_block_check(symt, statement->itstmt->block);
			t_expr_check(symt, statement->itstmt->iter);
			t_expr_check(symt, statement->itstmt->cond);
			symt = symbol_table_pop(symt);
		} else {
			symt = symbol_table_push(symt);
			t_block_check(symt, statement->itstmt->block);
			t_expr_check(symt, statement->itstmt->cond);
			symt = symbol_table_pop(symt);
		}
	}
}

void t_expr_check(symbol_table * symt, t_expr * expr)
{
	if (!expr)
		return;

	if (expr->type == 0) {
		symbol * sym = symbol_table_lookup(symt, expr->ident->ident);

		if (!sym) {
			printf("%s not declared\n", expr->ident->ident);
			exit(1);
		}
		expr->type_name = sym->type.type_name;
		expr->num_ptr = sym->type.num_ptr;
	} else if (expr->type == 2) {
		t_expr_check(symt, expr->binop->lhs);
		t_expr_check(symt, expr->binop->rhs);

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

	} else if (expr->type == 3) {
		symbol * sym = symbol_table_insert_decl_spec(symt, expr->decl_spec);
		if (!sym) {
			printf("Redeclaration of %s\n",
			       get_decl_name(expr->decl_spec));
			exit(1);
		}
		expr->type_name = sym->type.type_name;
		expr->num_ptr = sym->type.num_ptr;
	} else if (expr->type == 4) {
		t_expr_check(symt, expr->unop->term);
		if (expr->unop->term->type != 2 && expr->unop->term->type != 4) {
			//Assume constants are integers for now
			t_expr_set_typeinfo(symt, expr->unop->term);
		}
		//Cascade type information up tree
		expr->num_ptr = expr->unop->term->num_ptr;
		if (expr->unop->op == oper_ref)
			expr->num_ptr++;
		else if (expr->unop->op == oper_deref)
			expr->num_ptr--;
		expr->type_name = expr->unop->term->type_name;
	} else if (expr->type == 6) {
		t_expr_check(symt, expr->call->func);
		expr->type_name = expr->call->func->type_name;
		expr->num_ptr = expr->call->func->num_ptr;
	}
}

void t_expr_set_typeinfo(symbol_table * symt, t_expr * leaf)
{
	if (leaf->type == 1) {
		leaf->type_name = type_signed_int;
	} else if (leaf->type == 0) {
		symbol *sym = symbol_table_lookup(symt, leaf->ident->ident);
		if (sym) {
			leaf->num_ptr += sym->type.num_ptr;
			leaf->type_name = sym->type.type_name;
		} else {
			printf("FATAL ERROR\n");
			exit(1);
		}
		//print_expr_type(leaf);
	} else if (leaf->type == 3) {
		symbol *sym =
		    symbol_table_lookup(symt, get_decl_name(leaf->decl_spec));
		if (sym) {
			leaf->num_ptr += sym->type.num_ptr;
			leaf->type_name = sym->type.type_name;
		} else {
			printf("FATAL ERROR\n");
			exit(1);
		}
		//print_expr_type(leaf);
	} else if (leaf->type == 5) {
		leaf->type_name = type_signed_char;
		leaf->num_ptr = 1;
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
