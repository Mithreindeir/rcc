#include "symtable.h"

symbol *symbol_init(char * ident, type_info type, long hash)
{
	symbol *sym = malloc(sizeof(symbol));

	sym->ident = strdup(ident);
	sym->type = type;
	sym->scope = 1;
	sym->hash = hash;
	sym->next = NULL;

	return sym;
}

void symbol_apphend(symbol **head, symbol *new)
{
	if (!(*head)) {
		*head = new;
		return;
	}
	symbol * cur = *head;
	while (cur->next) cur = cur->next;
	cur->next = new;
}

symbol *symbol_find(symbol *head, long hash)
{	
	while (head) {
		if (head->hash == hash) return head;
		head = head->next;
	}
	return NULL;
}

symbol_table *symbol_table_init()
{
	symbol_table *symt = malloc(sizeof(symbol_table));
	symt->num_buckets = 100;
	symt->symbols = malloc(symt->num_buckets * sizeof(symbol*));
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

int symbol_table_insert(symbol_table *symt, char *ident, type_info type)
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

int symbol_table_insert_decl_spec(symbol_table *symt, t_decl_spec *decl)
{
	char * ident = NULL;

	type_info typen;
	typen.type_name = decl->type_name;
	typen.num_ptr = 0;

	t_declr *declor = NULL;
	t_dir_declr *ddecl = NULL;
	int type = 0;
	while (1) {
		if (type==0) {
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

symbol *symbol_table_lookup(symbol_table *symt, char *ident)
{
	long hash = symbol_table_hash(ident);

	return symbol_find(symt->symbols[hash % symt->num_buckets], hash);
}

void symbol_table_destroy(symbol_table *symt)
{
	if (!symt) return;

	printf("Dumping Symbol Table\n");
	for (int i = 0; i < symt->num_buckets; i++) {
		symbol *head = symt->symbols[i];
		while (head) {
			symbol *n = head->next;
			printf("Symbol: %s\n", head->ident);
			free(head->ident);
			free(head);
			head = n;
		}
	}

	free(symt->symbols);
	free(symt);
}