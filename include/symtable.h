#ifndef SYMTABLE_H
#define SYMTABLE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "ast.h"

//Temporarily pointers are hardcoded as 4 bytes
#define PTR_SIZE 4

static int type_size[] = {
	1, sizeof(char), sizeof(char), sizeof(short), sizeof(short),
	sizeof(int), sizeof(int), sizeof(long), sizeof(long), sizeof(float),
	sizeof(double), sizeof(long double)
};

typedef struct type_info {
	int type_name;
	int num_ptr;

	int size;
} type_info;

typedef struct symbol {
	char *ident;
	type_info type;
	long hash;
	int scope;
	int temp;

	struct symbol *next;
} symbol;

/* Hash Table holding type and symbol information*/
/* Symbol Table Holding Type and Identifier Information
 * Populated by Bison, the symbol table aids in type checking
 * and in generating the executables symbol table
 * */
typedef struct symbol_table {
	symbol **symbols;
	int num_buckets;

	//Each code block gets a new symbol table for scope.
	struct symbol_table **children, *parent;
	int num_children;

	//To later retrieve the correct symbol table for the right block
	//This iterator is used. Assumes the typecheck, and later
	//traversal are both in the same order
	int block_iter;
} symbol_table;

/*Allocates and Initializes a symbol table with 100 buckets*/
symbol_table *symbol_table_init();
/* Returns Symbol Holding Information*/
symbol *symbol_init(char *ident, type_info type, long hash);

symbol_table *symbol_table_push(symbol_table * symt);
symbol_table *symbol_table_pop(symbol_table * symt);

void symbol_table_add(symbol_table * parent, symbol_table * child);
void symbol_table_reset(symbol_table * table);
symbol_table *symbol_table_next(symbol_table * parent);
/*Inserts a symbol into the symtable. If a symbol already exists,
 * function will return one, because it probably means there are
 * duplicate or conflicting declarations*/
int symbol_table_insert(symbol_table * symt, char *ident, type_info type);

/*Automatically generates type info from AST decl_spec struct*/
int symbol_table_insert_decl_spec(symbol_table * symt, t_decl_spec * decl);

/* Returns symbol given identifier. Null if not present*/
symbol *symbol_table_lookup(symbol_table * symt, char *ident);
/*Frees all memory*/
void symbol_table_destroy(symbol_table * symt);

//Helper functions

/*Simple hashing function on the identifier*/
long symbol_table_hash(char *ident);
/*Apphends a symbol to the linked list of symbols at a bucket*/
void symbol_apphend(symbol ** head, symbol * new);
/*Finds a symbol in a bucket linked list*/
symbol *symbol_find(symbol * head, long hash);
/* Prints string given type identifier number*/
void print_type(int type);
/*Finds name in decl_spec struct*/
char *get_decl_name(t_decl_spec * decl);

#endif
