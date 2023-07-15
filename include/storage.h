
#ifndef _BRAINLOVE_STORAGE_H
#define _BRAINLOVE_STORAGE_H

#include "config.h"
#include "token.h"
#include "type.h"
#include "function.h"
#include "expression.h"

extern char code_buffer[CODE_BUFFER_CAPACITY];
extern unsigned int code_buffer_length;

extern Token tokens[TOKENS_CAPACITY];
extern unsigned int num_tokens;

extern char aux[AUX_CAPACITY];
extern unsigned int num_aux;

extern Type ast_types[TYPES_CAPACITY];
extern unsigned int num_ast_types;

extern unsigned int dimensions[UNIQUE_DIMENSIONS_CAPACITY][4];
extern unsigned int num_dimensions;

extern Function functions[FUNCTIONS_CAPACITY];
extern unsigned int num_functions;

extern Expression expressions[EXPRESSIONS_CAPACITY];
extern unsigned int num_expressions;

extern unsigned int operands[OPERANDS_CAPACITY];
extern unsigned int num_operands;

void print_aux_cstr(int index);

#endif // _BRAINLOVE_STORAGE_H

