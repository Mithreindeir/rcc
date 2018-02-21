#include "../include/quad.h"

quad_operand *quad_operand_init()
{
	quad_operand *qopr = malloc(sizeof(quad_operand));

	qopr->type = Q_NOTYPE;
	qopr->indirect = 0;
	qopr->call = 0;

	return qopr;
}

void quad_operand_destroy(quad_operand * opr)
{
	if (!opr)
		return;

	free(opr);
}

quadruple *quad_init()
{
	quadruple *quad = malloc(sizeof(quadruple));

	quad->type = Q_NONE;
	quad->operation = quad_none;

	quad->label = -1;
	quad->result = NULL;
	quad->arg1 = NULL;
	quad->arg2 = NULL;
	quad->name = NULL;

	return quad;
}

void quad_destroy(quadruple * quad)
{
	if (!quad)
		return;

	if (quad->result)
		quad_operand_destroy(quad->result);
	if (quad->arg1)
		quad_operand_destroy(quad->arg1);
	if (quad->arg2)
		quad_operand_destroy(quad->arg2);

	free(quad);
}

void quad_implicit_type(quadruple * quad)
{
	quad_op op = quad->operation;
	quadr_t type = Q_NONE;

	if (op == quad_add || op == quad_sub || op == quad_mul
	    || op == quad_div)
		type = Q_ARITH;
	else if (op == quad_jmp)
		type = Q_GOTO;
	else if (op == quad_jeq || op == quad_jneq || op == quad_jge
		 || op == quad_jg || op == quad_jl || op == quad_jle)
		type = Q_CGOTO;
	else if (op == quad_ptrasn)
		type = Q_PTR;
	else if (op == quad_ret)
		type = Q_RET;
	quad->type = type;
}

quadruple *quad_general(quad_op op, quad_operand * result,
			quad_operand * arg1, quad_operand * arg2)
{
	quadruple *quad = quad_init();

	quad->operation = op;
	quad->result = result;
	quad->arg1 = arg1;
	quad->arg2 = arg2;
	quad_implicit_type(quad);
	return quad;
}

quadruple *quad_jump(quad_op op, int label)
{
	quadruple *quad = quad_init();

	quad->label = label;
	quad->operation = op;
	quad_implicit_type(quad);

	return quad;
}

quadruple *quad_label(int label)
{
	quadruple *quad = quad_init();

	quad->label = label;
	quad->type = Q_LABEL;

	return quad;
}

quadruple *quad_nlabel(char *name)
{
	quadruple *quad = quad_init();

	quad->name = name;
	quad->type = Q_LABEL;

	return quad;
}

/*Printing Functions For Debugging*/
void quad_operation_print(quad_op oper)
{
	switch (oper) {
	case quad_ret:
		printf("ret");
		break;
	case quad_jmp:
		printf("goto");
		break;
	case quad_jeq:
		printf("jeq");
		break;
	case quad_jneq:
		printf("jneq");
		break;
	case quad_jge:
		printf("jge");
		break;
	case quad_jg:
		printf("jg");
		break;
	case quad_jle:
		printf("jle");
		break;
	case quad_jl:
		printf("jl");
		break;
	case quad_add:
		printf("+");
		break;
	case quad_sub:
		printf("-");
		break;
	case quad_mul:
		printf("*");
		break;
	case quad_div:
		printf("/");
		break;
	case quad_assign:
		printf("=");
		break;
	case quad_bor:
		printf("|");
		break;
	case quad_band:
		printf("&");
		break;
	case quad_xor:
		printf("^");
		break;
	default:
		break;
	}
}

void quad_opr_print(quad_operand * opr)
{
	if (!opr)
		return;

	if (opr->indirect == 1)
		printf("*");
	else if (opr->indirect == 2)
		printf("&");

	if (opr->type == Q_TEMP)
		printf("tmp%d", opr->temp);
	else if (opr->type == Q_SYM)
		printf("%s", opr->sym->ident);
	else if (opr->type == Q_CONST)
		printf("%ld", opr->constant);
	else if (opr->type == Q_CSTR)
		printf("%s", opr->cstr);

	if (opr->call)
		printf("()");
}

void quad_print(quadruple * quad)
{
	if (quad->type == Q_GOTO || quad->type == Q_CGOTO) {
		printf("\t");
		quad_operation_print(quad->operation);
		printf(" ");
		quad_opr_print(quad->arg1);
		printf(" ");
		quad_opr_print(quad->arg2);
		printf(" L%d", quad->label);
	} else if (quad->type == Q_LABEL) {
		if (quad->name)
			printf("\n%s:", quad->name);
		else
			printf("L%d", quad->label);
	} else if (quad->type == Q_RET) {
		printf("\t");
		quad_operation_print(quad->operation);
		printf(" ");
		quad_opr_print(quad->arg1);
	} else if (quad->type == Q_PARAM) {
		printf("\t");
		printf("param ");
		quad_opr_print(quad->result);
	} else {
		printf("\t");
		quad_opr_print(quad->result);
		printf(" = ");
		quad_opr_print(quad->arg1);
		if (quad->operation != quad_assign)
			quad_operation_print(quad->operation);
		quad_opr_print(quad->arg2);
	}
}

/*Quad Generator Functions*/
quad_gen *quad_gen_init(symbol_table * symt)
{
	quad_gen *qgen = malloc(sizeof(quad_gen));

	qgen->symt = symt;
	qgen->quads = NULL;
	qgen->num_quads = 0;
	qgen->num_temps = 0;
	qgen->num_labels = 0;

	return qgen;
}

void quad_gen_insert(quad_gen * gen, int idx, quadruple * quad)
{
	if (!gen || gen->num_quads <= idx)
		return;

	gen->num_quads++;
	gen->quads = realloc(gen->quads, sizeof(quadruple *) * gen->num_quads);
	memmove(gen->quads + idx + 1, gen->quads + idx,
		sizeof(quadruple *) * (gen->num_quads - idx - 1));

	for (int i = idx; i < gen->num_quads; i++)
		gen->quads[i]->idx = i;
	gen->quads[idx] = quad;
}

void quad_gen_destroy(quad_gen * qgen)
{
	if (!qgen)
		return;

	for (int i = 0; i < qgen->num_quads; i++) {
		quad_destroy(qgen->quads[i]);
	}

	free(qgen->quads);
	free(qgen);
}

void quad_gen_print(quad_gen * gen)
{
	printf("\n");
	for (int i = 0; i < gen->num_quads; i++) {
		//printf("%d:\t", i);
		quad_print(gen->quads[i]);
		printf("\n");
	}
	printf("\n");
}

int quad_gen_request_temp(quad_gen * gen)
{
	gen->num_temps++;
	return gen->num_temps - 1;
}

int quad_gen_last_temp(quad_gen * gen)
{
	return gen->num_temps - 1;
}

int quad_gen_request_label(quad_gen * gen)
{
	gen->num_labels++;
	return gen->num_labels - 1;
}

int quad_gen_last_label(quad_gen * gen)
{
	return gen->num_labels - 1;
}

void quad_gen_add(quad_gen * gen, quadruple * quad)
{
	if (!gen || !quad)
		return;

	gen->num_quads++;
	if (gen->num_quads == 1) {
		gen->quads = malloc(sizeof(quadruple *));
	} else {
		gen->quads =
		    realloc(gen->quads, sizeof(quadruple *) * gen->num_quads);
	}
	quad->idx = gen->num_quads - 1;
	gen->quads[gen->num_quads - 1] = quad;
}

int quad_gen_next(quad_gen * gen)
{
	return gen->num_quads;
}

/*Backpatching Functions*/
void quad_list_replace(quad_list * list)
{
}

int quad_list_min(quad_list * list)
{
	int min = INT_MAX;
	for (int i = 0; i < list->num_quads; i++) {
		min =
		    list->quad_list[i]->idx <
		    min ? list->quad_list[i]->idx : min;
	}
	return min;
}

int quad_list_max(quad_list * list)
{
	int max = INT_MIN;;
	for (int i = 0; i < list->num_quads; i++) {
		max =
		    list->quad_list[i]->idx >
		    max ? list->quad_list[i]->idx : max;
	}
	return max;
}

void quad_list_print(quad_list * list)
{
	for (int i = 0; i < list->num_quads; i++) {
		printf("%d\n", list->quad_list[i]->idx);
	}
}

void backpatch(quad_list * list, int label)
{
	for (int i = 0; i < list->num_quads; i++) {
		list->quad_list[i]->label = label;
	}
	list_destroy(list);
}

quad_list *merge(quad_list * l1, quad_list * l2)
{
	if (l1 && !l2)
		return l1;
	if (l2 && !l1)
		return l2;
	if (!l2 && !l1)
		return NULL;

	int old_size = l1->num_quads;
	l1->num_quads += l2->num_quads;
	l1->quad_list =
	    realloc(l1->quad_list, sizeof(quadruple *) * l1->num_quads);
	for (int i = 0; i < l2->num_quads; i++) {
		l1->quad_list[i + old_size] = l2->quad_list[i];
	}

	list_destroy(l2);

	return l1;
}

quad_list *make_list(quadruple * quad)
{
	quad_list *list = malloc(sizeof(quad_list *));

	list->quad_list = malloc(sizeof(quadruple *));
	list->num_quads = 1;
	list->quad_list[0] = quad;

	return list;
}

void list_destroy(quad_list * list)
{
	if (!list)
		return;

	free(list->quad_list);
	free(list);
}
