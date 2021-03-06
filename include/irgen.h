#ifndef IRGEN_H
#define IRGEN_H

#include "quad.h"
#include "ast.h"

/*Intermediate Representation Generation
 * Generates Quadruples from the AST
 * */

/*Post order traversal of the AST and assigns the interior nodes
 * Temporary variables, then generates a linear list of three address
 * code in the quadruple form
 * */
void unit_gen(quad_gen * gen, t_trans_unit * unit);
void func_gen(quad_gen * gen, t_func_def * func);
void block_gen(quad_gen * gen, t_block * block);
void stmt_gen(quad_gen * gen, t_stmt * stmt);
void expr_gen(quad_gen * gen, t_expr * expr);

quad_operand *quad_new_temp(quad_gen * gen);
quad_operand *quad_opr_from_expr(quad_gen * gen, t_expr * expr);
int quad_ptr_scale(quad_gen * gen, t_expr * expr);
void quad_from_binop(quad_gen * gen, t_expr * expr);
void quad_from_unop(quad_gen * gen, t_expr * expr);
t_expr *make_condition(t_expr * expr);
void quad_resolve_bool(quad_gen * gen, t_expr * expr);


#endif
