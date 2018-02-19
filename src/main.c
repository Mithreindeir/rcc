#include <stdio.h>
#include "../include/ast.h"
#include "../include/symtable.h"
#include "../include/typecheck.h"
#include "../include/irgen.h"

extern t_trans_unit *tunit;
extern symbol_table *global_table;

extern int yyparse();

int main(int argc, char **argv)
{
	global_table = symbol_table_init(NULL);
	yyparse();
	t_trans_unit_check(global_table, tunit);

	quad_gen *gen = quad_gen_init(global_table);
	symbol_table_reset(global_table);
	unit_gen(gen, tunit);
	quad_gen_print(gen);

	symbol_table_destroy(global_table);
	quad_gen_destroy(gen);
	t_trans_unit_destroy(tunit);
	return 0;
}
