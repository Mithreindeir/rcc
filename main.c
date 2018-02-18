#include <stdio.h>
#include "ast.h"
#include "symtable.h"
#include "typecheck.h"
#include "irgen.h"

extern t_block *main_block;
extern symbol_table *global_table;

extern int yyparse();

int main(int argc, char ** argv)
{
	global_table = symbol_table_init();
	yyparse();
	printf("\n");
	t_block_check(global_table, main_block);
	printf("\n");
	block_gen(quad_gen_init(global_table), main_block);

	//t_block_convert(NULL, main_block);
	symbol_table_destroy(global_table);

	t_block_destroy(main_block);
	return 0;
}
