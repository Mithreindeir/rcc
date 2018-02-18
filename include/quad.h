#ifndef QUAD_H
#define QUAD_H

#include "ast.h"
#include "symtable.h"
#include <limits.h>

typedef enum quadr_t
{
	Q_NONE,
	Q_ARITH,
	Q_BOOL,
	Q_PTR,
	Q_LABEL,
	Q_GOTO,
	Q_CGOTO,
	Q_PARAM
} quadr_t;

typedef enum quad_op
{
	quad_none,
	quad_jmp,
	quad_jeq,
	quad_jneq,
	quad_jge,
	quad_jg,
	quad_jle,
	quad_jl,
	quad_add,
	quad_sub,
	quad_mul,
	quad_div,
	quad_neg,
	quad_and,
	quad_or,
	quad_bor,
	quad_band,
	quad_xor,
	quad_assign,
	quad_ptrasn,
	quad_call
} quad_op;

typedef enum quad_operand_t
{
	Q_NOTYPE,
	Q_TEMP,
	Q_SYM,
	Q_CONST
} quad_operand_t;

typedef struct quad_operand
{
	union {
		int temp;
		symbol *sym;
		long constant;
	};
	quad_operand_t type;
	//Indirect of 0 = opr, 1 = *opr, 2 = &opr
	int indirect;
} quad_operand;

/*Quadruple IR structure*/
typedef struct quadruple
{
	quadr_t type;
	quad_op operation;

	quad_operand *result;
	quad_operand *arg1;
	quad_operand *arg2;

	int label;
	//Holding index in quad array helps with backpatching
	int idx;
} quadruple;

typedef struct quad_gen
{
	quadruple **quads;
	int num_quads;
	symbol_table *symt;

	int num_temps;
	int num_labels;
} quad_gen;

quad_operand *quad_operand_init();
void quad_operand_destroy(quad_operand *opr);

quadruple *quad_init();
void quad_destroy(quadruple *quad);

quadruple *quad_general(quad_op op, quad_operand *result, quad_operand *arg1, quad_operand *arg2);
quadruple *quad_label(int label);
quadruple *quad_jump(quad_op op, int label);
void quad_implicit_type(quadruple *quad);

quad_gen *quad_gen_init(symbol_table *symt);
void quad_gen_destroy(quad_gen *gen);

int quad_gen_request_temp(quad_gen *gen);
int quad_gen_last_temp(quad_gen *gen);

int quad_gen_request_label(quad_gen *gen);
int quad_gen_last_label(quad_gen *gen);

void quad_gen_add(quad_gen *gen, quadruple *quad);
int quad_gen_next(quad_gen *gen);
void quad_gen_insert(quad_gen *gen, int idx, quadruple *quad);
void quad_gen_print(quad_gen *gen);

/*General*/
void quad_operation_print(quad_op oper);
void quad_opr_print(quad_operand *opr);
void quad_print(quadruple *quad);

typedef struct quad_list
{
	quadruple ** quad_list;
	int num_quads;
} quad_list;

/*Backpatching interface*/
void backpatch(quad_list *list, int label);
quad_list *merge(quad_list *l1, quad_list *l2);
quad_list *make_list(quadruple *quad);
void list_destroy(quad_list *list);
int quad_list_min(quad_list *list);
int quad_list_max(quad_list *list);
void quad_list_replace(quad_list *list);
void quad_list_print(quad_list *list);

#endif
