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
		case oper_equal:
			str = "==";
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

/*
enum tac_types
{
	TAC_BASN,
	TAC_UASN,
	TAC_COPY,
	TAC_GOTO,
	TAC_CONDJMP,
	TAC_CALL,
	TAC_ARR_ASN,
	TAC_PTR_ASN
};
*/


void tac_operand_print(tac_operand *opr)
{
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
	} else {
		printf("FATAL ERROR: NOT LEAF\n");
		exit(1);
	}

	return opr;
}

tac_operand *tac_opr_temp(block_ctx *ctx)
{
	tac_operand *opr = malloc(sizeof(tac_operand));

	opr->temp_register = block_ctx_get_register(ctx);
	opr->tac_type = TAC_TEMP;

	return opr;
}

tac_instr *tac_new_label(block_ctx *ctx)
{
	tac_instr * instr = malloc(sizeof(tac_instr));

	instr->tac_type = TAC_LABEL;
	instr->label = block_ctx_get_label(ctx);

	return instr;
}

tac_instr *tac_from_binop(block_ctx *ctx, t_binop *binop)
{
	tac_instr *tac = malloc(sizeof(tac_instr));

	//Is copy instruction?
	if (binop->op == oper_assign) {
		tac->tac_type = TAC_COPY;
		tac->lhs = tac_opr_from_expr(ctx, binop->lhs);
		tac->operator_t = oper_assign;
		tac->rhs = tac_opr_from_expr(ctx, binop->rhs);
	} else {
		tac->tac_type = TAC_BASN;
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

	tac->tac_type = TAC_UASN;
	tac->temp_dst = tac_opr_temp(ctx);
	tac->operator_t = unop->op;
	tac->rhs = tac_opr_from_expr(ctx, unop->term);

	return tac;
}

tac_instr *tac_goto(block_ctx * ctx, int label)
{
	tac_instr *tac = malloc(sizeof(tac_instr));

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

	tac->clhs = tac_opr_from_expr(ctx, cstmt->condition);

	tac->coperator_t = oper_equal;
	tac->clabel = label;
	tac->crhs = malloc(sizeof(tac_operand));
	char buf[16];
	snprintf(buf, 16, "0");
	tac->crhs->constant_value = strdup(buf);
	tac->crhs->tac_type = TAC_CONST;

	return tac;
}

tac_instr *tac_from_itstmt(block_ctx *ctx, t_iterative_stmt *itstmt, int label)
{
	tac_instr *tac = malloc(sizeof(tac_instr));

	tac->tac_type = TAC_CONDJMP;

	tac->clhs = tac_opr_from_expr(ctx, itstmt->cond);
	if (!tac->clhs) {
		tac->tac_type = TAC_GOTO;
		tac->label = label;
	} else {
		tac->coperator_t = oper_equal;
		tac->clabel = label;
		tac->crhs = malloc(sizeof(tac_operand));
		char buf[16];
		snprintf(buf, 16, "0");
		tac->crhs->constant_value = strdup(buf);
		tac->crhs->tac_type = TAC_CONST;
	}

	return tac;
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

void t_stmt_convert(block_ctx *ctx, t_stmt *statement)
{
	if (!statement) return;
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
			t_expr_convert(ctx, statement->cstmt->condition);
			tac_instr *condjmp = tac_from_cstmt(ctx, statement->cstmt, label1->label);
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
				block_ctx_apphend_instr(ctx, compares);
				t_expr_convert(ctx, statement->itstmt->iter);
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
		if (expr->unop->term->type != 2 && expr->unop->term->type != 4) {
			tac = tac_from_unop(ctx, expr->unop);
			block_ctx_apphend_instr(ctx, tac);
		} else {
			t_expr_convert(ctx, expr->unop->term);
			tac = tac_from_unop(ctx, expr->unop);
			block_ctx_apphend_instr(ctx, tac);
		}
		expr->virt_reg = block_ctx_last_register(ctx);
	}
}