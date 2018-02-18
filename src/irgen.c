#include "../include/irgen.h"

void block_gen(quad_gen * gen, t_block * block)
{
	if (!block)
		return;

	for (int i = (block->num_statements - 1); i >= 0; i--) {
		stmt_gen(gen, block->statements[i]);
	}

	printf("\n");
	for (int i = 0; i < gen->num_quads; i++) {
		printf("%d:\t", i);
		quad_print(gen->quads[i]);
		printf("\n");
	}
	printf("\n");
}

void stmt_gen(quad_gen * gen, t_stmt * stmt)
{
	if (!stmt)
		return;

	switch (stmt->type) {
	case 0:		//compound statements
		block_gen(gen, stmt->block);
		break;
	case 2:		//expression statements
		expr_gen(gen, stmt->expression);
		break;
	case 3:		//if/else statements
		;
		quadruple *block_end = quad_label(quad_gen_request_label(gen));
		quadruple *block_start =
		    quad_label(quad_gen_request_label(gen));
		quadruple *else_stmt = NULL;
		if (stmt->cstmt->otherwise)
			else_stmt = quad_label(quad_gen_request_label(gen));

		//turn "if (1)" into "if (1!=0)"
		if (stmt->cstmt->condition->type != 2
		    && stmt->cstmt->condition->type != 4) {
			t_expr *expr =
			    t_expr_init2(stmt->cstmt->condition, oper_notequal,
					 t_expr_init1(t_numeric_init0("0")));
			stmt->cstmt->condition = expr;
		}
		expr_gen(gen, stmt->cstmt->condition);
		//quad_from_cond(gen, stmt->cstmt->condition);
		quad_gen_add(gen, block_start);
		block_gen(gen, stmt->cstmt->block);
		quad_gen_add(gen, block_end);
		if (stmt->cstmt->condition->falselist)
			backpatch(stmt->cstmt->condition->falselist,
				  block_end->label);
		if (stmt->cstmt->condition->truelist)
			backpatch(stmt->cstmt->condition->truelist,
				  block_start->label);
		if (else_stmt) {
			quad_jump(quad_jmp, else_stmt->label);
			block_gen(gen, stmt->cstmt->otherwise);
			quad_gen_add(gen, else_stmt);
		}
		quadruple *else_start = NULL;

		break;
	case 4:		//for/while loops
		break;
	default:
		break;		//Dont add declarations to quads
	}
}

/* If the expression is a binary of unary operation,
 * then call expr_gen on it, and after it returns assign it a
 * temporary variable. Leaf operations are generated straight into
 * quadruples */
void expr_gen(quad_gen * gen, t_expr * expr)
{
	if (!expr)
		return;

	if (expr->type == 2) {
		if ((expr->binop->lhs->type != 2 && expr->binop->lhs->type != 4)
		    && (expr->binop->rhs->type != 2
			&& expr->binop->rhs->type != 4)) {
			quad_from_binop(gen, expr);
		} else if (expr->binop->lhs->type != 2
			   && expr->binop->lhs->type != 4) {
			expr_gen(gen, expr->binop->rhs);
			quad_from_binop(gen, expr);
		} else if (expr->binop->rhs->type != 2
			   && expr->binop->rhs->type != 4) {
			expr_gen(gen, expr->binop->lhs);
			quad_from_binop(gen, expr);
		} else {
			expr_gen(gen, expr->binop->lhs);
			expr_gen(gen, expr->binop->rhs);
			quad_from_binop(gen, expr);
		}
		expr->virt_reg = quad_gen_last_temp(gen);

	} else if (expr->type == 4) {
		if (expr->unop->term->type != 2 && expr->unop->term->type != 4) {
			if (expr->unop->op != oper_deref)
				quad_from_unop(gen, expr);
		} else {
			expr_gen(gen, expr->unop->term);
			if (expr->unop->op != oper_deref)
				quad_from_unop(gen, expr);
		}

		expr->virt_reg = quad_gen_last_temp(gen);
	}
}

quad_op quad_map_operation(int oper)
{
	switch (oper) {
	case oper_assign:
		return quad_assign;
	case oper_add:
		return quad_add;
	case oper_sub:
		return quad_sub;
	case oper_mult:
		return quad_mul;
	case oper_div:
		return quad_div;
	case oper_and:
		return quad_and;
	case oper_or:
		return quad_or;
	case oper_equal:
		return quad_jneq;
	case oper_notequal:
		return quad_jeq;
	case oper_lt:
		return quad_jge;
	case oper_lte:
		return quad_jg;
	case oper_gt:
		return quad_jle;
	case oper_gte:
		return quad_jl;
	case oper_neg:
		return quad_neg;
	}

	return quad_none;
}

quad_operand *quad_opr_from_expr(quad_gen * gen, t_expr * expr)
{
	quad_operand *opr = quad_operand_init();
	if (expr->virt_reg != -1) {
		opr->type = Q_TEMP;
		opr->temp = expr->virt_reg;
	} else if (expr->type == 0) {
		opr->type = Q_SYM;
		opr->sym = symbol_table_lookup(gen->symt, expr->ident->ident);
	} else if (expr->type == 1) {
		opr->type = Q_CONST;
		opr->constant = expr->cnumeric->cint;
	} else if (expr->type == 3) {
		opr->type = Q_SYM;
		opr->sym =
		    symbol_table_lookup(gen->symt,
					get_decl_name(expr->decl_spec));
	} else if (expr->type == 4 && expr->unop->op == oper_deref) {
		opr->indirect = 1;
		if (expr->unop->term->type == 0) {
			opr->type = Q_SYM;
			opr->sym =
			    symbol_table_lookup(gen->symt,
						expr->unop->term->ident->ident);
		}
	} else if (expr->type == 4 && expr->unop->op == oper_ref) {
		opr->indirect = 2;
		if (expr->unop->term->type == 0) {
			opr->type = Q_SYM;
			opr->sym =
			    symbol_table_lookup(gen->symt,
						expr->unop->term->ident->ident);
		}
	} else {
		printf("%d\n", expr->type);
		printf
		    ("FATAL ERROR: quad_opr_from_expr recieved unparsed interior node\n");
		abort();
		exit(1);
	}

	if (expr->type == 4 && expr->unop->op == oper_deref) {
		opr->indirect = 1;
	} else if (expr->type == 4 && expr->unop->op == oper_ref) {
		opr->indirect = 2;
	}

	return opr;
}

quad_operand *quad_new_temp(quad_gen * gen)
{
	quad_operand *oper = quad_operand_init();

	oper->type = Q_TEMP;
	oper->temp = quad_gen_request_temp(gen);

	return oper;
}

void quad_from_cond(quad_gen * gen, t_expr * expr)
{
	quadruple *quad = NULL;
	quad_operand *cond = quad_opr_from_expr(gen, expr);

	quad = quad_general(quad_jeq, NULL, cond, NULL);
	expr->falselist = make_list(quad);
	quad_gen_add(gen, quad);
}

void quad_from_binop(quad_gen * gen, t_expr * expr)
{
	quadruple *quad = NULL;

	quad_operand *result = NULL;
	quad_operand *arg1 = NULL;
	quad_operand *arg2 = NULL;

	quad_op op = quad_map_operation(expr->binop->op);
	//If one side is pointer, the other isn't a pointer, and operation
	//is add or sub, then multiply the rhs by the pointer's size
	int lptr_op = expr->binop->lhs->num_ptr > 0;
	lptr_op = lptr_op && (expr->binop->rhs->num_ptr == 0);
	lptr_op = lptr_op && (expr->binop->op == oper_add
			      || expr->binop->op == oper_sub);

	int rptr_op = expr->binop->rhs->num_ptr > 0;
	rptr_op = rptr_op && (expr->binop->lhs->num_ptr == 0);
	rptr_op = rptr_op && (expr->binop->op == oper_add
			      || expr->binop->op == oper_sub);
	int size = PTR_SIZE;
	if (lptr_op && expr->binop->lhs->num_ptr - 1 == 0)
		size = type_size[expr->binop->lhs->type_name];
	else if (rptr_op && expr->binop->rhs->num_ptr - 1 == 0)
		size = type_size[expr->binop->rhs->type_name];

	if (op == quad_assign) {
		arg1 = quad_opr_from_expr(gen, expr->binop->lhs);
		arg2 = quad_opr_from_expr(gen, expr->binop->rhs);
		quad = quad_general(op, arg1, arg2, NULL);
	} else {
		//Add instruction before the current one to adjust for
		//pointer size. Ex *(arr+i) -> *(arr+i*4)
		if (lptr_op) {
			t_expr *ebop;
			ebop =
			    t_expr_init2(expr->binop->rhs, oper_mult,
					 t_expr_init1(t_numeric_init2(size)));
			quad_from_binop(gen, ebop);
			ebop->binop->lhs = NULL;
			t_expr_destroy(ebop);

			expr->binop->rhs->virt_reg = quad_gen_last_temp(gen);
			result = quad_new_temp(gen);
			arg1 = quad_opr_from_expr(gen, expr->binop->lhs);
			arg2 = quad_opr_from_expr(gen, expr->binop->rhs);
			quad_gen_add(gen, quad_general(op, result, arg1, arg2));
			return;
		} else if (rptr_op) {
			t_expr *ebop;
			ebop =
			    t_expr_init2(expr->binop->lhs, oper_mult,
					 t_expr_init1(t_numeric_init2(size)));
			quad_from_binop(gen, ebop);
			ebop->binop->rhs = NULL;
			t_expr_destroy(ebop);

			expr->binop->rhs->virt_reg = quad_gen_last_temp(gen);
			result = quad_new_temp(gen);
			arg1 = quad_opr_from_expr(gen, expr->binop->lhs);
			arg2 = quad_opr_from_expr(gen, expr->binop->rhs);
			quad_gen_add(gen, quad_general(op, result, arg1, arg2));
			return;
		}
		quad_operand *result = quad_new_temp(gen);
		arg1 = quad_opr_from_expr(gen, expr->binop->lhs);
		arg2 = quad_opr_from_expr(gen, expr->binop->rhs);
		quad = quad_general(op, result, arg1, arg2);
	}
	//Todo switch index array to pointers, so an array insertion doesnt screw up the following indexes
	if (quad->operation == quad_and) {
		int max_idx = quad_list_max(expr->binop->lhs->truelist);
		quadruple *qlabel = quad_label(quad_gen_request_label(gen));
		quad_gen_insert(gen, max_idx + 1, qlabel);
		backpatch(expr->binop->lhs->truelist, qlabel->label);
		expr->binop->lhs->truelist = NULL;
		expr->truelist = expr->binop->rhs->truelist;
		expr->falselist =
		    merge(expr->binop->lhs->falselist,
			  expr->binop->rhs->falselist);
	} else if (quad->operation == quad_or) {
		int max_idx = quad_list_max(expr->binop->lhs->truelist);
		quadruple *qlabel = quad_label(quad_gen_request_label(gen));
		quad_gen_insert(gen, max_idx + 1, qlabel);
		backpatch(expr->binop->lhs->falselist, qlabel->label);

		expr->binop->lhs->falselist = NULL;
		expr->falselist = expr->binop->rhs->falselist;
		expr->truelist =
		    merge(expr->binop->lhs->truelist,
			  expr->binop->rhs->truelist);
	} else {
		quad_gen_add(gen, quad);
	}

	if (quad->type == Q_CGOTO) {
		expr->falselist = make_list(quad);
		quadruple *jquad = quad_jump(quad_jmp, 0);
		quad_gen_add(gen, jquad);
		expr->truelist = make_list(jquad);
	}
}

void quad_from_unop(quad_gen * gen, t_expr * expr)
{
	quad_operand *result = quad_new_temp(gen);
	quad_operand *arg1 = quad_opr_from_expr(gen, expr->unop->term);
	quad_op op = quad_map_operation(expr->unop->op);

	quad_gen_add(gen, quad_general(op, result, arg1, NULL));
}
