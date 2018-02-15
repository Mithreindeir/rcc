#include <stdio.h>
#include "ast.h"
#include "threecode.h"
#include "symtable.h"
#include "typecheck.h"

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
	t_block_convert(NULL, main_block);
	symbol_table_destroy(global_table);

	return 0;
}