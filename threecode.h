#ifndef QUAD_H
#define QUAD_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ast.h"
#include "symtable.h"

/* Lower Level IR converted from AST
   Naive Three address Code
*/

typedef struct block_ctx block_ctx;
typedef struct tac_instr tac_instr;

enum tac_instr_types
{
	TAC_BASN,
	TAC_UASN,
	TAC_COPY,
	TAC_GOTO,
	TAC_CONDJMP,
	TAC_CALL,
	TAC_ARR_ASN,
	TAC_PTR_ASN,
	TAC_LABEL
};

enum tac_operand_types
{
	TAC_TEMP,
	TAC_IDENT,
	TAC_CONST
};

typedef struct tac_operand
{
	union {
		char * identifier;
		char * constant_value;
		int temp_register;
	};
	int ptr_access;
	
	int tac_type;
} tac_operand;

struct tac_instr
{
	int tac_type;

	union {
		struct {
			tac_operand *temp_dst;

			tac_operand *lhs;
			int operator_t;
			tac_operand *rhs;
		};//Used for TAC_BASN, TAC_UASN, AND TAC_COPY
		struct {
			tac_operand *clhs;
			int coperator_t;
			tac_operand *crhs;
			int clabel;
		};

		int label;
	};
	//The next instruction
	tac_instr * next;
};

//Block Context
struct block_ctx
{
	//Parent block context
	block_ctx *parent;
	//Identifier and type
	t_decl_spec **declarations;
	int num_decl;
	//How many virtual registers have been 
	int num_registers;
	//A basic block of tac_instr
	tac_instr *start;
	//Current label number
	int num_labels;
};


void tac_operand_print(tac_operand *opr);

tac_operand *tac_opr_from_expr(block_ctx *ctx, t_expr *expr);
tac_operand *tac_opr_temp(block_ctx *ctx);

tac_instr *tac_goto(block_ctx * ctx, int label);
tac_instr *tac_new_label(block_ctx *ctx);
tac_instr *tac_from_binop(block_ctx *ctx, t_binop *binop);
tac_instr *tac_from_unop(block_ctx *ctx, t_unop *unop);
tac_instr *tac_from_cstmt(block_ctx *ctx, t_conditional_stmt *cstmt, int label);
tac_instr *tac_from_itstmt(block_ctx *ctx, t_iterative_stmt *itstmt, int label);

block_ctx *block_ctx_init(block_ctx *parent);
void block_ctx_add_decl(block_ctx *ctx, t_decl_spec *decl);
t_decl_spec *block_ctx_get_decl(block_ctx *ctx, char * ident);
char * get_decl_name(t_decl_spec *decl);

void block_ctx_apphend_instr(block_ctx *ctx, tac_instr *instr);

int block_ctx_get_label(block_ctx *ctx);
int block_ctx_last_label(block_ctx *ctx);

int block_ctx_get_register(block_ctx *ctx);
int block_ctx_last_register(block_ctx *ctx);

char *oper_string(int oper);
char *expr_string(block_ctx *ctx, t_expr *expr);

void tac_instr_print(tac_instr *instr);

void t_block_convert(block_ctx *p, t_block *block);
void t_stmt_convert(block_ctx *ctx, t_stmt *statement);
void t_expr_convert(block_ctx *ctx, t_expr *expr);

#endif