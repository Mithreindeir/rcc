#include <stdio.h>
#include "node.h"
#include "rerr.h"
#include "rcc.h"

extern t_block *main_block;
extern int yyparse();

int main(int argc, char ** argv)
{
	yyparse();
	printf("\n");
	//t_block_print(main_block);
	//t_block_check(NULL, main_block);
	printf("\n");
	t_block_3pass(NULL, main_block);
	return 0;
}
