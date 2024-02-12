#ifndef _CODEGEN_H
#define _CODEGEN_H

#include "ast.h"


extern struct table *table;

void codegen_program(struct node *program);
int codegen_expression(struct node *expression,struct table * func_table);
int conversion_is_possible(enum category type1,enum category type2);

#endif