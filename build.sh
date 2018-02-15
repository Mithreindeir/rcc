lex -o tokens.c tokens.lex
bison -d -o parser.c parser.y
gcc -o parser parser.c tokens.c node.c rerr.c rcc.c main.c -lfl -ggdb3
