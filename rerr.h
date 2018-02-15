#ifndef RERR_H
#define RERR_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "node.h"
#include "rcc.h"

void syntax_error(char * message);
int compatible_decl(t_decl_spec *d1, t_decl_spec *d2);

//Traverses the AST and prints errors
void t_block_check(block_ctx *p, t_block *block);
void t_stmt_check(block_ctx *ctx, t_stmt *statement);
t_decl_spec *t_expr_check(block_ctx *ctx, t_expr *expr);

#endif
