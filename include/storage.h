
#ifndef _PECK_STORAGE_H
#define _PECK_STORAGE_H

#include <stdbool.h>
#include "config.h"
#include "token.h"
#include "type.h"
#include "function.h"
#include "expression.h"
#include "global.h"
#include "typedef.h"
#include "utypes.h"

extern u8 code_buffer[CODE_BUFFER_CAPACITY];
extern u32 code_buffer_length;

extern Token tokens[TOKENS_CAPACITY];
extern u32 num_tokens;

extern u8 aux[AUX_CAPACITY];
extern u32 num_aux;

extern Type types[TYPES_CAPACITY];
extern u32 num_types;

extern u32 dimensions[UNIQUE_DIMENSIONS_CAPACITY][4];
extern u32 num_dimensions;

extern Function functions[FUNCTIONS_CAPACITY];
extern u32 num_functions;

extern Expression expressions[EXPRESSIONS_CAPACITY];
extern u32 num_expressions;

extern u32 statements[STATEMENTS_CAPACITY];
extern u32 num_statements;

extern u32 operands[OPERANDS_CAPACITY];
extern u32 num_operands;

extern Global globals[GLOBALS_CAPACITY];
extern u32 num_globals;

extern TypeDef typedefs[TYPEDEFS_CAPACITY];
extern u32 num_typedefs;

u32 add_type(Type type);
u32 add_dimensions(u32 type_dimensions[4]);
u32 add_function(Function function);
u32 find_function(u32 name);
u32 find_function_return_type(u32 name);
u32 add_expression(Expression expression);
u32 add_statement_from_existing(u32 expression_index);
u32 add_statement_from_new(Expression expression);
u32 add_operand(u32 a);
u32 add_operands2(u32 a, u32 b);
u32 add_operands3(u32 a, u32 b, u32 c);
u32 add_global(Global global);
u32 add_typedef(TypeDef def);
u32 find_typedef(u32 name);
u32 aux_cstr_alloc(u8 null_terminated_name[16]);
u1 aux_cstr_equals(u32 a, u32 b);
u1 aux_cstr_equals_print(u32 a);
u1 aux_cstr_equals_main(u32 a);
u1 aux_cstr_equals_u1(u32 a);
u1 aux_cstr_equals_u0(u32 a);
u1 aux_cstr_equals_u8(u32 a);
u1 aux_cstr_equals_u16(u32 a);
u1 aux_cstr_equals_u32(u32 a);
u32 aux_cstr_len(u32 str);
u0 print_aux_cstr(u32 index);
u0 print_aux_cstr_escaped(u32 index);

#endif // _PECK_STORAGE_H

