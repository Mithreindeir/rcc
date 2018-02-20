#include "../include/irgen.h"

void unit_gen(quad_gen * gen, t_trans_unit * unit)
{
	if (!unit)
		return;

	for (int i = 0; i < unit->num_def; i++) {
		if (unit->definitions[i]->type == 0) {
			func_gen(gen, unit->definitions[i]->func);
		} else if (unit->definitions[i]->type == 1) {
			expr_gen(gen, unit->definitions[i]->declaration);
		}
	}
}

void func_gen(quad_gen * gen, t_func_def * func)
{
	if (!func)
		return;

	gen->symt = symbol_table_next(gen->symt);
	block_gen(gen, func->block);
	gen->symt = symbol_table_pop(gen->symt);
}

void block_gen(quad_gen * gen, t_block * block)
{
	if (!block)
		return;

	for (int i = (block->num_statements - 1); i >= 0; i--) {
		stmt_gen(gen, block->statements[i]);
		block->truelist =
		    merge(block->truelist, block->statements[i]->truelist);
		block->falselist =
		    merge(block->falselist, block->statements[i]->falselist);
	}

}

void stmt_gen(quad_gen * gen, t_stmt * stmt)
{
	if (!stmt)
		return;

	switch (stmt->type) {
	case 0:		//compound statements
		gen->symt = symbol_table_next(gen->symt);
		block_gen(gen, stmt->block);
		stmt->truelist = merge(stmt->truelist, stmt->block->truelist);
		stmt->falselist =
		    merge(stmt->falselist, stmt->block->falselist);
		gen->symt = symbol_table_pop(gen->symt);
		break;
	case 2:		//expression statements
		expr_gen(gen, stmt->expression);
		stmt->truelist =
		    merge(stmt->truelist, stmt->expression->truelist);
		stmt->falselist =
		    merge(stmt->falselist, stmt->expression->falselist);
		break;
	case 3:		//if/else statements
		;
		quadruple *block_end = quad_label(quad_gen_request_label(gen));
		quadruple *block_start =
		    quad_label(quad_gen_request_label(gen));
		quadruple *else_stmt = NULL;
		if (stmt->cstmt->otherwise)
			else_stmt = quad_label(quad_gen_request_label(gen));

		stmt->cstmt->condition = make_condition(stmt->cstmt->condition);
		expr_gen(gen, stmt->cstmt->condition);
		//quad_from_cond(gen, stmt->cstmt->condition);
		quad_gen_add(gen, block_start);
		gen->symt = symbol_table_next(gen->symt);
		block_gen(gen, stmt->cstmt->block);
		gen->symt = symbol_table_pop(gen->symt);
		if (else_stmt) {
			quadruple *jquad =
			    quad_jump(quad_jmp, else_stmt->label);
			quad_gen_add(gen, jquad);
		}
		quad_gen_add(gen, block_end);
		if (stmt->cstmt->condition->falselist)
			backpatch(stmt->cstmt->condition->falselist,
				  block_end->label);
		if (stmt->cstmt->condition->truelist)
			backpatch(stmt->cstmt->condition->truelist,
				  block_start->label);
		if (else_stmt) {
			gen->symt = symbol_table_next(gen->symt);
			block_gen(gen, stmt->cstmt->otherwise);
			gen->symt = symbol_table_pop(gen->symt);
			quad_gen_add(gen, else_stmt);
		}
		quadruple *else_start = NULL;
		stmt->truelist =
		    merge(stmt->truelist, stmt->cstmt->block->truelist);
		stmt->falselist =
		    merge(stmt->falselist, stmt->cstmt->block->falselist);
		if (else_stmt) {
			stmt->truelist =
			    merge(stmt->truelist,
				  stmt->cstmt->otherwise->truelist);
			stmt->falselist =
			    merge(stmt->falselist,
				  stmt->cstmt->otherwise->falselist);
		}
		break;
	case 4:		//for/while loops
		if (stmt->itstmt->type == 0) {
			quadruple *block_start, *comp_start, *iter_start;

			block_start = quad_label(quad_gen_request_label(gen));
			comp_start = quad_label(quad_gen_request_label(gen));
			iter_start = quad_label(quad_gen_request_label(gen));
			gen->symt = symbol_table_next(gen->symt);
			expr_gen(gen, stmt->itstmt->init);
			quad_gen_add(gen,
				     quad_jump(quad_jmp, comp_start->label));
			quad_gen_add(gen, block_start);
			block_gen(gen, stmt->itstmt->block);
			quad_gen_add(gen, iter_start);
			expr_gen(gen, stmt->itstmt->iter);
			quad_gen_add(gen, comp_start);
			stmt->itstmt->cond = make_condition(stmt->itstmt->cond);
			expr_gen(gen, stmt->itstmt->cond);
			gen->symt = symbol_table_pop(gen->symt);
			quadruple *end =
			    quad_label(quad_gen_request_label(gen));
			quad_gen_add(gen, end);
			if (stmt->itstmt->cond && stmt->itstmt->cond->truelist)
				backpatch(stmt->itstmt->cond->truelist,
					  block_start->label);
			if (stmt->itstmt->cond && stmt->itstmt->cond->falselist)
				backpatch(stmt->itstmt->cond->falselist,
					  end->label);

			if (stmt->itstmt->block
			    && stmt->itstmt->block->truelist)
				backpatch(stmt->itstmt->block->truelist,
					  iter_start->label);
			if (stmt->itstmt->block
			    && stmt->itstmt->block->falselist)
				backpatch(stmt->itstmt->block->falselist,
					  end->label);
		} else {
			quadruple *block_start, *comp_start;
			block_start = quad_label(quad_gen_request_label(gen));
			comp_start = quad_label(quad_gen_request_label(gen));

			if (!stmt->itstmt->first) {
				quad_gen_add(gen,
					     quad_jump(quad_jmp,
						       comp_start->label));
			}
			quad_gen_add(gen, block_start);
			gen->symt = symbol_table_next(gen->symt);
			block_gen(gen, stmt->itstmt->block);
			quad_gen_add(gen, comp_start);
			stmt->itstmt->cond = make_condition(stmt->itstmt->cond);
			expr_gen(gen, stmt->itstmt->cond);
			gen->symt = symbol_table_pop(gen->symt);
			quadruple *end =
			    quad_label(quad_gen_request_label(gen));
			quad_gen_add(gen, end);

			if (stmt->itstmt->cond && stmt->itstmt->cond->truelist)
				backpatch(stmt->itstmt->cond->truelist,
					  block_start->label);
			if (stmt->itstmt->cond && stmt->itstmt->cond->falselist)
				backpatch(stmt->itstmt->cond->falselist,
					  end->label);

			if (stmt->itstmt->block
			    && stmt->itstmt->block->truelist)
				backpatch(stmt->itstmt->block->truelist,
					  block_start->label);
			if (stmt->itstmt->block
			    && stmt->itstmt->block->falselist)
				backpatch(stmt->itstmt->block->falselist,
					  end->label);
		}
		break;
	case 5:
		//continue
		if (stmt->jump->type == 0) {
			printf("asdiasj\n");
			quadruple *continuej = quad_jump(quad_jmp, 0);
			quad_gen_add(gen, continuej);
			stmt->truelist = make_list(continuej);
		} else if (stmt->jump->type == 1) {

			quadruple *breakj = quad_jump(quad_jmp, 0);
			quad_gen_add(gen, breakj);
			stmt->falselist = make_list(breakj);
		}
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
			if (expr->unop->op != oper_deref
			    && expr->unop->op != oper_ref)
				quad_from_unop(gen, expr);
		} else {
			expr_gen(gen, expr->unop->term);
			if (expr->unop->op != oper_deref
			    && expr->unop->op != oper_ref)
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
	case oper_incpre:
		return quad_add;
	case oper_incpost:
		return quad_add;
	case oper_decpre:
		return quad_sub;
	case oper_decpost:
		return quad_sub;
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
	} else if (expr->type == 5) {
		opr->type = Q_CSTR;
		opr->cstr = expr->cstring;
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

t_expr *make_condition(t_expr * expr)
{
	t_expr *cond = expr;
	if (expr->type != 2 && expr->type != 4) {
		cond =
		    t_expr_init2(expr, oper_notequal,
				 t_expr_init1(t_numeric_init0("0")));
	}

	return cond;
}

/*If one side is pointer, the other isn't a pointer, and operation
 * is add or sub, then multiply the rhs by the pointer's size
 * Add instruction before the current one to adjust for
 * pointer size. Ex *(arr+i) -> *(arr+i*4)
 * */
int quad_ptr_scale(quad_gen * gen, t_expr * expr)
{
	quadruple *quad = NULL;

	quad_operand *result = NULL;
	quad_operand *arg1 = NULL;
	quad_operand *arg2 = NULL;

	quad_op op = quad_map_operation(expr->binop->op);

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
		return 1;
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
		return 1;
	}
	return 0;
}

void quad_from_binop(quad_gen * gen, t_expr * expr)
{
	quadruple *quad = NULL;

	quad_operand *result = NULL;
	quad_operand *arg1 = NULL;
	quad_operand *arg2 = NULL;

	quad_op op = quad_map_operation(expr->binop->op);

	if (op == quad_assign) {
		arg1 = quad_opr_from_expr(gen, expr->binop->lhs);
		arg2 = quad_opr_from_expr(gen, expr->binop->rhs);
		quad = quad_general(op, arg1, arg2, NULL);
	} else {
		if (quad_ptr_scale(gen, expr))
			return;
		quad_operand *result = quad_new_temp(gen);
		arg1 = quad_opr_from_expr(gen, expr->binop->lhs);
		arg2 = quad_opr_from_expr(gen, expr->binop->rhs);
		quad = quad_general(op, result, arg1, arg2);
	}
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
	quad_op op = quad_map_operation(expr->unop->op);
	quad_operand *result = quad_new_temp(gen);
	quad_operand *arg1 = quad_opr_from_expr(gen, expr->unop->term);
	quad_operand *arg2 = NULL;

	if (expr->unop->op == oper_incpost || expr->unop->op == oper_decpost) {
		quad_operand *arg2 = quad_operand_init();
		arg2->type = Q_CONST;
		arg2->constant = 1;

		quad_operand *arg1d = quad_opr_from_expr(gen, expr->unop->term);
		quad_operand *arg1dd =
		    quad_opr_from_expr(gen, expr->unop->term);
		quad_gen_add(gen,
			     quad_general(quad_assign, result, arg1, NULL));
		quad_gen_add(gen, quad_general(op, arg1d, arg1dd, arg2));
	} else if (expr->unop->op == oper_decpre
		   || expr->unop->op == oper_incpre) {
		quad_operand *r1 = quad_opr_from_expr(gen, expr->unop->term);
		quad_operand *r2 = quad_opr_from_expr(gen, expr->unop->term);
		quad_operand *inc = quad_operand_init();
		inc->type = Q_CONST;
		inc->constant = 1;

		quad_gen_add(gen, quad_general(op, r1, r2, inc));
		quad_gen_add(gen,
			     quad_general(quad_assign, result, arg1, NULL));
	} else {
		quad_gen_add(gen, quad_general(op, result, arg1, NULL));
	}
}
