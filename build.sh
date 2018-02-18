lex -o src/tokens.c src/tokens.lex
bison -d -v --defines=include/parser.h --output=src/parser.c src/parser.y
if [ ! -d build ]; then
	mkdir build
fi

gcc -o build/rcc src/parser.c src/tokens.c src/ast.c src/typecheck.c src/irgen.c src/quad.c src/symtable.c src/main.c -lfl -ggdb3
