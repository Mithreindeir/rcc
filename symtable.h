#ifndef SYMTABLE_H
#define SYMTABLE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ast.h"

typedef struct type_info
{
	int type_name;
	int num_ptr;
} type_info;

typedef struct symbol
{
	char * ident;
	type_info type;
	long hash;
	int scope;

	struct symbol * next;
} symbol;

/* Hash Table holding type and symbol information*/
typedef struct symbol_table
{
	symbol **symbols;
	int num_buckets;
} symbol_table;


symbol_table *symbol_table_init();
symbol *symbol_init(char * ident, type_info type, long hash);

//Returns 0 for success, 1 for already exists
int symbol_table_insert(symbol_table *symt, char *ident, type_info type);
int symbol_table_insert_decl_spec(symbol_table *symt, t_decl_spec *decl);

symbol *symbol_table_lookup(symbol_table *symt, char *ident);
void symbol_table_destroy(symbol_table *symt);

//Helper functions
long symbol_table_hash(char *ident);
void symbol_apphend(symbol **head, symbol *new);
symbol *symbol_find(symbol *head, long hash);


#endif