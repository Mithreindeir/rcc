lex -o tokens.c tokens.lex
bison -d -o parser.c parser.y
gcc -o parser parser.c tokens.c ast.c rerr.c threecode.c symtable.c main.c -lfl -ggdb3
