#include "../include/symtable.h"

symbol *symbol_init(char *ident, type_info type, long hash)
{
	symbol *sym = malloc(sizeof(symbol));

	sym->ident = strdup(ident);
	sym->temp = 0;
	sym->type = type;
	sym->scope = 1;
	sym->hash = hash;
	sym->next = NULL;

	return sym;
}

void symbol_apphend(symbol ** head, symbol * new)
{
	if (!(*head)) {
		*head = new;
		return;
	}
	symbol *cur = *head;
	while (cur->next)
		cur = cur->next;
	cur->next = new;
}

symbol *symbol_find(symbol * head, long hash)
{
	while (head) {
		if (head->hash == hash)
			return head;
		head = head->next;
	}
	return NULL;
}

symbol_table *symbol_table_push(symbol_table * table)
{
	symbol_table *child = symbol_table_init(table);
	symbol_table_add(table, child);
	return child;
}

symbol_table *symbol_table_pop(symbol_table * table)
{
	return table->parent ? table->parent : table;
}

symbol_table *symbol_table_next(symbol_table * table)
{
	assert(table->block_iter < table->num_children);
	return table->children[table->block_iter++];
}

void symbol_table_reset(symbol_table * table)
{
	if (!table)
		return;
	table->block_iter = 0;
	for (int i = 0; i < table->num_children; i++) {
		symbol_table_reset(table->children[i]);
	}
}

void symbol_table_add(symbol_table * parent, symbol_table * child)
{
	parent->num_children++;
	if (parent->num_children == 1) {
		parent->children = malloc(sizeof(symbol_table *));
	} else {
		parent->children =
		    realloc(parent->children,
			    sizeof(symbol_table *) * parent->num_children);
	}

	parent->children[parent->num_children - 1] = child;
}

symbol_table *symbol_table_init(symbol_table * parent)
{
	symbol_table *symt = malloc(sizeof(symbol_table));
	symt->num_buckets = 100;
	symt->symbols = malloc(symt->num_buckets * sizeof(symbol *));
	symt->parent = parent;
	symt->children = NULL;
	symt->num_children = 0;
	symt->block_iter = 0;

	for (int i = 0; i < symt->num_buckets; i++) {
		symt->symbols[i] = NULL;
	}

	return symt;
}

long symbol_table_hash(char *ident)
{
	long hash = 0;

	int len = strlen(ident);
	for (int i = 0; i < len; i++) {
		hash = (hash << 4) + ident[i];
	}

	return hash;
}

int symbol_table_insert(symbol_table * symt, char *ident, type_info type)
{
	long hash = symbol_table_hash(ident);
	long idx = hash % symt->num_buckets;

	symbol *s = symbol_find(symt->symbols[hash % symt->num_buckets], hash);
	if (!s) {
		s = symbol_init(ident, type, hash);
		symbol_apphend(&symt->symbols[hash % symt->num_buckets], s);
		return 0;
	}
	return 1;
}

int symbol_table_insert_decl_spec(symbol_table * symt, t_decl_spec * decl)
{
	char *ident = NULL;

	type_info typen;
	typen.type_name = decl->type_name;
	typen.num_ptr = 0;

	t_declr *declor = NULL;
	t_dir_declr *ddecl = NULL;
	int type = 0;
	while (1) {
		if (type == 0) {
			declor = decl->declarator;
			typen.num_ptr += declor->ptr;
			type = 1;
		} else if (type == 1) {
			ddecl = declor->ddecl;
			type = 2;
		} else {
			if (ddecl->type) {
				declor = ddecl->decl;
				type = 1;
			} else {
				ident = ddecl->ident->ident;
				break;
			}
		}
	}

	return symbol_table_insert(symt, ident, typen);
}

symbol *symbol_table_lookup(symbol_table * symt, char *ident)
{
	long hash = symbol_table_hash(ident);

	symbol *sym =
	    symbol_find(symt->symbols[hash % symt->num_buckets], hash);
	if (!sym && symt->parent) {
		sym = symbol_table_lookup(symt->parent, ident);
	}
	return sym;
}

void symbol_table_destroy(symbol_table * symt)
{
	if (!symt)
		return;

	for (int i = 0; i < symt->num_children; i++) {
		symbol_table_destroy(symt->children[i]);
	}
	free(symt->children);

	printf("Variables\n");
	for (int i = 0; i < symt->num_buckets; i++) {
		symbol *head = symt->symbols[i];
		while (head) {
			symbol *n = head->next;
			printf("Symbol: %s Type: ", head->ident);
			print_type(head->type.type_name);
			printf(" nptr: %d\n", head->type.num_ptr);
			free(head->ident);
			free(head);
			head = n;
		}
	}
	printf("vEnd\n");

	free(symt->symbols);
	free(symt);
}

void print_type(int type)
{
	switch (type) {
	case type_signed_int:
		printf("int ");
		break;
	case type_unsigned_int:
		printf("unsigned int ");
		break;
	case type_signed_char:
		printf("char ");
		break;
	case type_unsigned_char:
		printf("unsigned char ");
		break;
	case type_unsigned_long:
		printf("unsigned long ");
		break;
	case type_signed_long:
		printf("long ");
		break;
	case type_signed_short:
		printf("short ");
		break;
	case type_unsigned_short:
		printf("unsigned short ");
		break;
	case type_float:
		printf("float ");
		break;
	case type_double:
		printf("double ");
		break;
	}
}

char *get_decl_name(t_decl_spec * decl)
{
	t_declr *declor = NULL;
	t_dir_declr *ddecl = NULL;
	int type = 0;
	while (1) {
		if (type == 0) {
			declor = decl->declarator;
			type = 1;
		} else if (type == 1) {
			ddecl = declor->ddecl;
			type = 2;
		} else {
			if (ddecl->type) {
				declor = ddecl->decl;
				type = 1;
			} else {
				return ddecl->ident->ident;
			}
		}
	}
}
