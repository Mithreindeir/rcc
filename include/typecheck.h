#ifndef TYPECHECK_H
#define TYPECHECK_H

#include "ast.h"
#include "symtable.h"

/*Performs Post Order Traversal on AST and fills in type information*/
void t_func_check(symbol_table * symt, t_func_def * func);
void t_block_check(symbol_table * symt, t_block * block);
void t_stmt_check(symbol_table * symt, t_stmt * statement);
void t_expr_check(symbol_table * symt, t_expr * expr);

void t_expr_set_typeinfo(symbol_table * symt, t_expr * leaf);

void print_expr_type(t_expr * expr);
void print_type(int type);

#endif
