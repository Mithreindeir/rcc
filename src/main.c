#include <stdio.h>
#include "../include/ast.h"
#include "../include/symtable.h"
#include "../include/typecheck.h"
#include "../include/irgen.h"

extern t_func_def *main_func;
extern symbol_table *global_table;

extern int yyparse();

int main(int argc, char **argv)
{
	global_table = symbol_table_init(NULL);
	yyparse();

	printf("\n");
	t_func_check(global_table, main_func);
	printf("\n");
	quad_gen *gen = quad_gen_init(global_table);
	symbol_table_reset(global_table);
	func_gen(gen, main_func);
	quad_gen_print(gen);
	//t_block_convert(NULL, main_block);
	symbol_table_destroy(global_table);
	quad_gen_destroy(gen);
	t_func_def_destroy(main_func);
	return 0;
}
