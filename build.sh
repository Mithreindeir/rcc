lex -d -o tokens.c tokens.lex
bison -d --verbose -o parser.c parser.y
gcc -o parser parser.c tokens.c node.c main.c -lfl -ggdb3
