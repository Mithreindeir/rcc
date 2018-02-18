#include <stdio.h>
#include "../include/ast.h"
#include "../include/symtable.h"
#include "../include/typecheck.h"
#include "../include/irgen.h"

extern t_block *main_block;
extern symbol_table *global_table;

extern int yyparse();

int main(int argc, char **argv)
{
	global_table = symbol_table_init();
	yyparse();
	printf("\n");
	t_block_check(global_table, main_block);
	printf("\n");
	quad_gen *gen = quad_gen_init(global_table);
	block_gen(gen, main_block);
	quad_gen_print(gen);
	//t_block_convert(NULL, main_block);
	symbol_table_destroy(global_table);
	quad_gen_destroy(gen);
	t_block_destroy(main_block);
	return 0;
}
