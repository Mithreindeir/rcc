#include <stdio.h>
#include "ast.h"
#include "rerr.h"
#include "threecode.h"

extern t_block *main_block;
extern int yyparse();

int main(int argc, char ** argv)
{
	yyparse();
	printf("\n");
	//t_block_check(NULL, main_block);
	printf("\n");
	t_block_convert(NULL, main_block);
	return 0;
}
