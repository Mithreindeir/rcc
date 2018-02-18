#include "threecode.h"

block_ctx *block_ctx_init(block_ctx *parent)
{
	block_ctx *ctx = malloc(sizeof(block_ctx));

	ctx->declarations = NULL;
	ctx->num_decl = 0;
	ctx->parent = parent;
	ctx->num_registers = 0;
	ctx->start = tac_new_label(ctx);

	return ctx;
}

void block_ctx_add_decl(block_ctx *ctx, t_decl_spec *decl)
{
	ctx->num_decl++;
	if (ctx->num_decl==1) {
		ctx->declarations = malloc(sizeof(t_decl_spec*));
	} else {
		ctx->declarations = realloc(ctx->declarations, sizeof(t_decl_spec*) * ctx->num_decl);
	}
	ctx->declarations[ctx->num_decl-1] = decl;
}

t_decl_spec *block_ctx_get_decl(block_ctx *ctx, char * ident)
{
	for (int i = 0; i < ctx->num_decl; i++) {
		if (!strcmp(ident, get_decl_name(ctx->declarations[i])))
		return ctx->declarations[i];
	}
	if (ctx->parent) return block_ctx_get_decl(ctx->parent, ident);
	return NULL;
}

char * get_decl_name(t_decl_spec *decl)
{
	t_declr *declor = NULL;
	t_dir_declr *ddecl = NULL;
	int type = 0;
	while (1) {
		if (type==0) {
			declor = decl->declarator;
			type = 1;
		} else if (type == 1) {
			ddecl = declor->ddecl;
			type = 2;
		} else {
			if (ddecl->type) {
				declor = ddecl->decl;
				type = 1;
			} else {
				return ddecl->ident->ident;
			}
		}
	}
}

void block_ctx_apphend_instr(block_ctx *ctx, tac_instr *instr)
{
	if (!ctx->start) {
		ctx->start = instr;
		return;
	}
	tac_instr *cur = ctx->start;
	while (cur->next) cur = cur->next;
	cur->next = instr;
}

int block_ctx_get_register(block_ctx *ctx)
{
	ctx->num_registers++;
	return ctx->num_registers-1;
}

int block_ctx_last_register(block_ctx *ctx)
{
	return ctx->num_registers-1;
}

int block_ctx_get_label(block_ctx *ctx)
{
	ctx->num_labels++;
	return ctx->num_labels-1;
}

int block_ctx_last_label(block_ctx *ctx)
{
	return ctx->num_labels-1;
}

char *expr_string(block_ctx *ctx, t_expr *expr)
{
	char buf[16];
	if (expr->virt_reg != -1) {
		snprintf(buf, 16, "tmp%d", expr->virt_reg);
		return strdup(buf);
	}

	if (expr->type==0) {
		snprintf(buf, 16, "_%s", expr->ident->ident);
		return strdup(buf);
	} else if (expr->type==1) {
		if (expr->cnumeric->type==0)
			snprintf(buf, 16, "%lldd", expr->cnumeric->cint);
		else
			snprintf(buf, 16, "%ff", expr->cnumeric->cdouble);
		return strdup(buf);
	} else if (expr->type==3) {
		snprintf(buf, 16, "_%s",get_decl_name(expr->decl_spec));
		return strdup(buf);
	}
	return NULL;
}

char *oper_string(int oper)
{
	char * str = NULL;
	switch(oper) {
		case oper_assign:
			str = "=";
			break;
		case oper_add:
			str = "+";
			break;
		case oper_sub:
			str = "-";
			break;
		case oper_mult:
			str = "*";
			break;
		case oper_div:
			str = "/";
			break;
		case oper_or:
			str = "||";
			break;
		case oper_and:
			str = "&&";
			break;
		case oper_equal:
			str = "==";
			break;
		case oper_lt:
			str = "<";
			break;
		case oper_lte:
			str = "<=";
			break;
		case oper_gt:
			str = ">";
			break;
		case oper_gte:
			str = ">=";
			break;
		case oper_notequal:
			str = "!=";
			break;
		case oper_deref:
			str = "*";
			break;
		case oper_ref:
			str = "&";
			break;
		case oper_incpost:
			str = "++";
			break;
		case oper_incpre:
			str = "++";
			break;
		case oper_decpost:
			str = "--";
			break;
		case oper_decpre:
			str = "--";
			break;
		case oper_neg:
			str = "-";
			break;
	}
	return str;
}

void tac_operand_print(tac_operand *opr)
{
	if (opr->ptr_access)
		printf("*");

	if (opr->tac_type == TAC_IDENT) {
		printf("%s", opr->identifier);
	} else if (opr->tac_type == TAC_CONST) {
		printf("%s", opr->constant_value);
	} else if (opr->tac_type == TAC_TEMP) {
		printf("tmp%d", opr->temp_register);
	}
}

tac_operand *tac_opr_from_expr(block_ctx *ctx, t_expr *expr)
{
	if (!expr) return NULL;

	tac_operand *opr = malloc(sizeof(tac_operand));
	opr->ptr_access = 0;

	if (expr->virt_reg != -1) {
		opr->temp_register = expr->virt_reg;
		opr->tac_type = TAC_TEMP;
	} else if (expr->type == 0) {
		opr->identifier = strdup(expr->ident->ident);
		opr->tac_type = TAC_IDENT;
	} else if (expr->type == 1) {
		char buf[16];
		if (expr->cnumeric->type==0)
			snprintf(buf, 16, "%lld", expr->cnumeric->cint);
		else
			snprintf(buf, 16, "%f", expr->cnumeric->cdouble);

		opr->constant_value = strdup(buf);
		opr->tac_type = TAC_CONST;
	} else if (expr->type == 3) {
		opr->identifier = strdup(get_decl_name(expr->decl_spec));
		opr->tac_type = TAC_IDENT;
	} else if (expr->type == 4 && expr->unop->op == oper_deref) {
		opr->ptr_access = 1;
		if (expr->unop->term->type == 0) {
			opr->identifier = strdup(expr->unop->term->ident->ident);
			opr->tac_type = TAC_IDENT;
		}
	} else {
		printf("FATAL ERROR: NOT LEAF\n");
		getchar();
		exit(1);
	}
	if (expr->type == 4 && expr->unop->op == oper_deref) {
		opr->ptr_access = 1;
	}
	return opr;
}

/*
typedef struct backpatch
{
	int *labels;
	tac_instr **instruction;
	int num;
} backpatch;
*/

backpatch *b_makelist(tac_instr *instr)
{
	backpatch *bp = malloc(sizeof(backpatch));

	bp->instructions = malloc(sizeof(tac_instr*));
	bp->instructions[0] = instr;
	bp->num = 1;
	bp->labels = malloc(sizeof(int));
	bp->labels[0] = 0;

	return bp;
}

backpatch *b_merge(backpatch *b1, backpatch *b2)
{
	if (!b1 || !b2) return NULL;

	int oldsize = b1->num;
	b1->num += b2->num;
	b1->instructions = realloc(b1->instructions, sizeof(tac_instr*) * b1->num);
	b1->labels = realloc(b1->labels, sizeof(int) * b1->num);

	for (int i = 0; i < b2->num; i++) {
		b1->instructions[oldsize+i] = b2->instructions[i];
		b1->labels[oldsize+i] = b2->labels[i];
	}

	free(b2);
	free(b2->instructions);
	free(b2->labels);
	return b1;
}

void b_patch(backpatch *b1, int label)
{
	if (!b1) return;

	for (int i = 0; i < b1->num; i++) {
		b1->instructions[i]->clabel = label;
	}
}

tac_operand *tac_opr_temp(block_ctx *ctx)
{
	tac_operand *opr = malloc(sizeof(tac_operand));

	opr->ptr_access = 0;
	opr->temp_register = block_ctx_get_register(ctx);
	opr->tac_type = TAC_TEMP;

	return opr;
}

tac_instr *tac_new_label(block_ctx *ctx)
{
	tac_instr * instr = malloc(sizeof(tac_instr));
    instr->truelist = NULL;
    instr->falselist = NULL;
	instr->tac_type = TAC_LABEL;
	instr->label = block_ctx_get_label(ctx);

	return instr;
}

int oper_is_conditional(int oper)
{
    return oper == oper_equal || oper == oper_notequal || oper == oper_gt || oper == oper_gte || oper == oper_lt || oper == oper_lte;
}

int oper_is_boolean(int oper)
{
    return oper == oper_and || oper == oper_or;
}

tac_instr *tac_from_cond(block_ctx *ctx, t_expr *expr)
{
    tac_instr *tac = malloc(sizeof(tac_instr));

    tac->tac_type = TAC_CONDJMP;
    tac->truelist = NULL;
    tac->falselist = NULL;
    if (expr->type == 2 && oper_is_boolean(expr->binop->op)) {
    	tac = tac_from_bool(ctx, expr->binop);
        tac->clabel = -1;
    	return tac;
    } else if (expr->type == 2) {
        tac->coperator_t = inverse_cond(expr->binop->op);
        tac->clhs = tac_opr_from_expr(ctx, expr->binop->lhs);
        tac->crhs = tac_opr_from_expr(ctx, expr->binop->rhs);
        tac->clabel = -1;

        return tac;
    }
    tac->clhs = tac_opr_from_expr(ctx, expr);

    tac->coperator_t = oper_equal;
    tac->crhs = malloc(sizeof(tac_operand));
    tac->crhs->constant_value = strdup("0");
    tac->crhs->tac_type = TAC_CONST;
    tac->clabel = -1;


    return tac;
}

int inverse_cond(int cond)
{
    if (cond == oper_lt) return oper_gte;
    if (cond == oper_lte) return oper_gt;
    if (cond == oper_gt) return oper_lte;
    if (cond == oper_gte) return oper_lt;
    if (cond == oper_equal) return oper_equal;
    if (cond == oper_notequal) return oper_equal;

    return cond;
}

tac_instr *tac_from_bool(block_ctx *ctx, t_binop *binop)
{
    if (oper_is_conditional(binop->op)) {
        tac_instr *tac = malloc(sizeof(tac_instr));

        tac->tac_type = TAC_CONDJMP;
        tac->truelist = NULL;
        tac->falselist = NULL;
        tac->clhs = tac_opr_from_expr(ctx, binop->lhs);
        tac->crhs = tac_opr_from_expr(ctx, binop->rhs);
        tac->coperator_t = binop->op;
        tac->clabel = -1;

        return tac;
    }

    tac_instr *t1 = tac_from_cond(ctx, binop->lhs);

    tac_instr *t2 = tac_from_cond(ctx, binop->rhs);
    t1->next = t2;

    if (binop->op == oper_and) {

    } else if (binop->op == oper_or) {
    }

    return t1;
}

tac_instr *tac_from_binop(block_ctx *ctx, t_binop *binop)
{
	tac_instr *tac = malloc(sizeof(tac_instr));

    tac->truelist = NULL;
    tac->falselist = NULL;
	//Is copy instruction?
	if (binop->op == oper_assign) {
		tac->tac_type = TAC_COPY;
		tac->lhs = tac_opr_from_expr(ctx, binop->lhs);
		//If lhs is a dereferenced pointer, then type is ptr assign
		if (binop->lhs->type == 4 && binop->lhs->unop->op == oper_deref)
			tac->tac_type = TAC_PTR_ASN;
		tac->operator_t = oper_assign;
		tac->rhs = tac_opr_from_expr(ctx, binop->rhs);
	} else {//If operation is pointer arithmetic, then multiply the index by the pointer type size
		tac->tac_type = TAC_BASN;

		if (binop->lhs->num_ptr > 0 && binop->rhs->num_ptr == 0 && (binop->op == oper_add || binop->op == oper_sub)) {
			int size = PTR_SIZE;
			if (binop->lhs->num_ptr-1 == 0) size = type_size[binop->lhs->type_name];
			t_binop *bop = malloc(sizeof(bop));
			bop->op = oper_mult;
			bop->lhs = binop->rhs;
			bop->rhs = t_expr_init1(t_numeric_init2(size));
			tac_instr *pbop = tac_from_binop(ctx, bop);
			pbop->next = tac;
			binop->rhs->virt_reg = block_ctx_last_register(ctx);

			free(bop);
			tac->lhs = tac_opr_from_expr(ctx, binop->lhs);
			tac->rhs = tac_opr_from_expr(ctx, binop->rhs);
			tac->temp_dst = tac_opr_temp(ctx);
			tac->operator_t = binop->op;
			return pbop;
		} else if (binop->rhs->num_ptr > 0 && binop->lhs->num_ptr == 0 && (binop->op == oper_add || binop->op == oper_sub)) {

			int size = PTR_SIZE;
			if (binop->lhs->num_ptr-1 == 0) size = type_size[binop->lhs->type_name];
			t_binop *bop = malloc(sizeof(bop));
			bop->op = oper_mult;
			bop->lhs = binop->lhs;
			bop->rhs = t_expr_init1(t_numeric_init2(size));
			tac_instr *pbop = tac_from_binop(ctx, bop);
			pbop->next = tac;
			binop->lhs->virt_reg = block_ctx_last_register(ctx);

			free(bop);
			tac->temp_dst = tac_opr_temp(ctx);
			tac->lhs = tac_opr_from_expr(ctx, binop->lhs);
			tac->rhs = tac_opr_from_expr(ctx, binop->rhs);
			tac->operator_t = binop->op;
			return pbop;
		}
		tac->temp_dst = tac_opr_temp(ctx);
		tac->lhs = tac_opr_from_expr(ctx, binop->lhs);
		tac->operator_t = binop->op;
		tac->rhs = tac_opr_from_expr(ctx, binop->rhs);

	}

	return tac;
}

tac_instr *tac_from_unop(block_ctx *ctx, t_unop *unop)
{
	tac_instr *tac = malloc(sizeof(tac_instr));

    tac->truelist = NULL;
    tac->falselist = NULL;
	tac->tac_type = TAC_UASN;
	tac->temp_dst = tac_opr_temp(ctx);
	tac->operator_t = unop->op;
	tac->rhs = tac_opr_from_expr(ctx, unop->term);
	//If pointer arithmetic then create an instruction multiplying

	return tac;
}

tac_instr *tac_goto(block_ctx * ctx, int label)
{
	tac_instr *tac = malloc(sizeof(tac_instr));

    tac->truelist = NULL;
    tac->falselist = NULL;
	tac->tac_type = TAC_GOTO;
	tac->label = label;

	return tac;
}

void tac_instr_print(tac_instr *instr)
{
	if (!instr) return;

	if (instr->tac_type == TAC_LABEL) {
		printf(".L%d:", instr->label);
	}
	printf("\t");
	if (instr->tac_type == TAC_COPY) {
		//x = y
		tac_operand_print(instr->lhs);
		printf(" %s ", oper_string(instr->operator_t));
		tac_operand_print(instr->rhs);
	} else if (instr->tac_type == TAC_BASN) {
		//temp = x + y
		tac_operand_print(instr->temp_dst);
		printf(" := ");
		tac_operand_print(instr->lhs);
		printf(" %s ", oper_string(instr->operator_t));
		tac_operand_print(instr->rhs);
	} else if (instr->tac_type == TAC_UASN) {
		//temp = -x
		tac_operand_print(instr->temp_dst);
		printf(" := %s", oper_string(instr->operator_t));
		tac_operand_print(instr->rhs);
	} else if (instr->tac_type == TAC_CONDJMP) {
		printf("if ");
		tac_operand_print(instr->clhs);
		printf(" %s ", oper_string(instr->coperator_t));
		tac_operand_print(instr->crhs);
		printf(" then goto ");
		printf(".L%d", instr->clabel);
	} else if (instr->tac_type == TAC_GOTO) {
		printf("goto .L%d", instr->label);
	} else if (instr->tac_type == TAC_PTR_ASN) {
		//*x = y
		tac_operand_print(instr->lhs);
		printf(" %s ", oper_string(instr->operator_t));
		tac_operand_print(instr->rhs);
	}
	printf("\n");

	tac_instr_print(instr->next);
}


/*Three address code from conditional statement
*/
tac_instr *tac_from_cstmt(block_ctx *ctx, t_conditional_stmt *cstmt, int label)
{
	tac_instr *tac = malloc(sizeof(tac_instr));

	tac->tac_type = TAC_CONDJMP;
    tac->truelist = NULL;
    tac->falselist = NULL;
    if (cstmt->condition->type == 2) {
        tac->coperator_t = inverse_cond(cstmt->condition->binop->op);
        tac->clhs = tac_opr_from_expr(ctx, cstmt->condition->binop->lhs);
        tac->crhs = tac_opr_from_expr(ctx, cstmt->condition->binop->rhs);
        tac->clabel = label;

        return tac;
    }

	tac->clhs = tac_opr_from_expr(ctx, cstmt->condition);

	tac->coperator_t = oper_equal;
	tac->clabel = label;
	tac->crhs = malloc(sizeof(tac_operand));
	tac->crhs->constant_value = strdup("0");
	tac->crhs->tac_type = TAC_CONST;

	return tac;
}

tac_instr *tac_from_itstmt(block_ctx *ctx, t_iterative_stmt *itstmt, int label)
{
	tac_instr *tac = malloc(sizeof(tac_instr));

	tac->tac_type = TAC_CONDJMP;
    tac->truelist = NULL;
    tac->falselist = NULL;
	tac->clhs = tac_opr_from_expr(ctx, itstmt->cond);
	if (!tac->clhs) {
		tac->tac_type = TAC_GOTO;
		tac->label = label;
	} else {
		tac->coperator_t = oper_notequal;
		tac->clabel = label;
		tac->crhs = malloc(sizeof(tac_operand));
		char buf[16];
		snprintf(buf, 16, "0");
		tac->crhs->constant_value = strdup(buf);
		tac->crhs->tac_type = TAC_CONST;
	}

	return tac;
}

void tac_opr_destroy(tac_operand *opr)
{
	if (!opr) return;

	if (opr->tac_type == TAC_IDENT) {
		free(opr->identifier);
	} else if (opr->tac_type == TAC_CONST) {
		free(opr->constant_value);
	}

	free(opr);
}

void tac_instr_destroy(tac_instr * tac)
{
	if (!tac) return;
	if (tac->tac_type == TAC_COPY) {
		tac_opr_destroy(tac->lhs);
		tac_opr_destroy(tac->rhs);
	} else if (tac->tac_type == TAC_BASN) {
		tac_opr_destroy(tac->lhs);
		tac_opr_destroy(tac->rhs);
		tac_opr_destroy(tac->temp_dst);
	} else if (tac->tac_type == TAC_UASN) {
		tac_opr_destroy(tac->temp_dst);
		tac_opr_destroy(tac->rhs);
	} else if (tac->tac_type == TAC_CONDJMP) {
		tac_opr_destroy(tac->clhs);
		tac_opr_destroy(tac->crhs);
	} else if (tac->tac_type == TAC_PTR_ASN) {
		tac_opr_destroy(tac->lhs);
		tac_opr_destroy(tac->rhs);
	}

	free(tac);
}

void t_block_convert(block_ctx *p, t_block *block)
{
	if (!block) return;
	block_ctx * ctx = NULL;
	if (!p)
		ctx = block_ctx_init(p);
	else ctx = p;

	for (int i = (block->num_statements-1); i >= 0; i--) {
		t_stmt_convert(ctx, block->statements[i]);
	}
	//tac_instr_print(ctx->start);
	if (!p) {
		tac_instr_print(ctx->start);
		printf("\n");
	}
}

tac_instr *block_ctx_get_last(block_ctx *ctx)
{
    tac_instr * cur = ctx->start;
    while (cur && cur->next) cur = cur->next;
    return cur;
}


void t_stmt_convert(block_ctx *ctx, t_stmt *statement)
{
	if (!statement) return;
    tac_instr *first = block_ctx_get_last(ctx);

	switch (statement->type) {
		case 0:
			t_block_convert(ctx, statement->block);
			break;
		case 1:
			//block_ctx_add_decl(ctx, statement->declaration);
			break;
		case 2:
			t_expr_convert(ctx, statement->expression);
			break;
		case 3://if statements
			;
			//Make first Label
			tac_instr *label1 = tac_new_label(ctx);
			tac_instr *label2 = NULL;
			if (statement->cstmt->otherwise) label2 = tac_new_label(ctx);
			//t_expr_convert(ctx, statement->cstmt->condition);
			//tac_instr *condjmp = tac_from_cstmt(ctx, statement->cstmt, label1->label);
			
			t_expr_convert(ctx, statement->cstmt->condition);
			tac_instr *condjmp = tac_from_cond(ctx, statement->cstmt->condition);
			condjmp->clabel = label1->label;
			block_ctx_apphend_instr(ctx, condjmp);
			t_block_convert(ctx, statement->cstmt->block);
			if (label2) block_ctx_apphend_instr(ctx, tac_goto(ctx, label2->label));
			block_ctx_apphend_instr(ctx, label1);

			if (label2) {
				t_block_convert(ctx, statement->cstmt->otherwise);
				block_ctx_apphend_instr(ctx, label2);
			}
			break;
		case 4:
			;
			//for loop
			if (statement->itstmt->type == 0) {
				//
				tac_instr *block_start = tac_new_label(ctx);
				tac_instr *compares = tac_new_label(ctx);
				t_expr_convert(ctx, statement->itstmt->init);
				block_ctx_apphend_instr(ctx, tac_goto(ctx, compares->label));
				block_ctx_apphend_instr(ctx, block_start);
				t_block_convert(ctx, statement->itstmt->block);
				t_expr_convert(ctx, statement->itstmt->iter);
				block_ctx_apphend_instr(ctx, compares);
				t_expr_convert(ctx, statement->itstmt->cond);
				block_ctx_apphend_instr(ctx, tac_from_itstmt(ctx, statement->itstmt, block_start->label));
			} else {
				tac_instr *block_start = tac_new_label(ctx);
				tac_instr *compares = NULL;
				//Do statements start executing first, and don't skip straight to the comparison
				if (!statement->itstmt->first) {
					compares = tac_new_label(ctx);
					block_ctx_apphend_instr(ctx, tac_goto(ctx, compares->label));
				}
				block_ctx_apphend_instr(ctx, block_start);
				t_block_convert(ctx, statement->itstmt->block);
				if (compares) block_ctx_apphend_instr(ctx, compares);
				t_expr_convert(ctx, statement->itstmt->cond);
				block_ctx_apphend_instr(ctx, tac_from_itstmt(ctx, statement->itstmt, block_start->label));
			}
	}
}

/*
Calculate three address codes for leafs first, then assign interior nodes
a unique temporary value and use that to generate more
*/
void t_expr_convert(block_ctx *ctx, t_expr *expr)
{
	if (!expr) return;

	tac_instr *tac = NULL;

	if (expr->type == 2) {
		if ((expr->binop->lhs->type != 2 && expr->binop->lhs->type != 4) && (expr->binop->rhs->type != 2 && expr->binop->rhs->type != 4)) {
			tac=tac_from_binop(ctx, expr->binop);
			block_ctx_apphend_instr(ctx, tac);
		} else if (expr->binop->lhs->type != 2 && expr->binop->lhs->type != 4) {
			t_expr_convert(ctx, expr->binop->rhs);
			tac=tac_from_binop(ctx, expr->binop);
			block_ctx_apphend_instr(ctx, tac);
		} else if (expr->binop->rhs->type != 2 && expr->binop->rhs->type != 4) {
			t_expr_convert(ctx, expr->binop->lhs);
			tac=tac_from_binop(ctx, expr->binop);
			block_ctx_apphend_instr(ctx, tac);
		} else {
			t_expr_convert(ctx, expr->binop->lhs);
			t_expr_convert(ctx, expr->binop->rhs);
			tac=tac_from_binop(ctx, expr->binop);
			block_ctx_apphend_instr(ctx, tac);
		}
		expr->virt_reg = block_ctx_last_register(ctx);
	} else if (expr->type == 4) {
		//Dont use temps when dereferencing a value
		if (expr->unop->term->type != 2 && expr->unop->term->type != 4) {
			if (expr->unop->op != oper_deref)
				tac = tac_from_unop(ctx, expr->unop);
			block_ctx_apphend_instr(ctx, tac);
		} else {
			t_expr_convert(ctx, expr->unop->term);
			if (expr->unop->op != oper_deref)
				tac = tac_from_unop(ctx, expr->unop);
			block_ctx_apphend_instr(ctx, tac);
		}
		expr->virt_reg = block_ctx_last_register(ctx);
	}
}
