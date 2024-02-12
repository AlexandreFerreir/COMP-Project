#ifndef _SEMANTICS_H
#define _SEMANTICS_H

#include "ast.h"



struct symbol {
	char *identifier;
	int param;
	int temporary;
	enum category type;
	enum category return_type;
	struct node * paramList;
	struct symbol *next;
};

struct table {
	char *identifier;
	struct symbol *symbols;
	struct table *next;
};

struct symbol * insert_symbol(struct table *table, char *identifier, enum category type,int param,struct node * paramList);
struct table * search_table(struct table * table,char * identifier);
struct symbol *search_symbol(struct table *table, char *identifier);
void show_symbol_table();
void check_program(struct node *program,struct table *table,struct table *first_table);
void print_symbol_table(struct table * tables);
char * to_lower(const char *string);
void free_table(struct table *table);


#endif
