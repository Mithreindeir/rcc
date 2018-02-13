#include <stdio.h>
#include "node.h"

extern t_block *main_block;
extern int yyparse();

int main(int argc, char ** argv)
{
	yyparse();
	printf("\n");
	t_block_print(main_block);
	printf("\n");
	return 0;
}
