
#ifndef _BRAINLOVE_STORAGE_H
#define _BRAINLOVE_STORAGE_H

#include <stdbool.h>
#include "config.h"
#include "token.h"
#include "type.h"
#include "function.h"
#include "expression.h"
#include "global.h"

extern char code_buffer[CODE_BUFFER_CAPACITY];
extern unsigned int code_buffer_length;

extern Token tokens[TOKENS_CAPACITY];
extern unsigned int num_tokens;

extern char aux[AUX_CAPACITY];
extern unsigned int num_aux;

extern Type types[TYPES_CAPACITY];
extern unsigned int num_types;

extern unsigned int dimensions[UNIQUE_DIMENSIONS_CAPACITY][4];
extern unsigned int num_dimensions;

extern Function functions[FUNCTIONS_CAPACITY];
extern unsigned int num_functions;

extern Expression expressions[EXPRESSIONS_CAPACITY];
extern unsigned int num_expressions;

extern int statements[STATEMENTS_CAPACITY];
extern unsigned int num_statements;

extern unsigned int operands[OPERANDS_CAPACITY];
extern unsigned int num_operands;

extern Global globals[GLOBALS_CAPACITY];
extern unsigned int num_globals;

int add_type(Type type);
int add_expression(Expression expression);
int add_statement_from_existing(int expression_index);
int add_statement_from_new(Expression expression);
int add_global(Global global);
bool aux_cstr_equals(int a, int b);
void print_aux_cstr(int index);

#endif // _BRAINLOVE_STORAGE_H

