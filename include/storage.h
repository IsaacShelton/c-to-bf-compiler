
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
#include "define.h"
#include "utypes.h"

typedef enum {
    CLOSE_NEEDED_FOR_EARLY_RETURN_CHECK,
    CLOSE_NEEDED_FOR_BREAK_CHECK,
    CLOSE_NEEDED_FOR_CONTINUE_CHECK,
} CloseNeeded;

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

extern TypeAlias type_aliases[TYPE_ALIASES_CAPACITY];
extern u32 num_type_aliases;

extern Define defines[DEFINES_CAPACITY];
extern u32 num_defines;

extern CloseNeeded closes_needed[CLOSES_NEEDED_CAPACITY];
extern u32 num_closes_needed;

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
u32 add_operands5(u32 a, u32 b, u32 c, u32 d, u32 e);
u32 add_global(Global global);
u32 add_typedef(TypeDef def);
u32 find_typedef(u32 name);
u32 add_type_alias(TypeAlias alias);
u32 try_resolve_type_alias(u32 name);
u32 add_define(Define define);
u32 try_resolve_define(u32 name, u1 use_preferred_type);
u32 aux_cstr_alloc(u8 null_terminated_name[32]);
u1 aux_cstr_equals(u32 a, u32 b);
u1 aux_cstr_equals_string(u32 a, u8 string[32]);
u1 aux_cstr_equals_print(u32 a);
u1 aux_cstr_equals_main(u32 a);
u1 aux_cstr_equals_u1(u32 a);
u1 aux_cstr_equals_u0(u32 a);
u1 aux_cstr_equals_u8(u32 a);
u1 aux_cstr_equals_u16(u32 a);
u1 aux_cstr_equals_u32(u32 a);
u1 aux_cstr_equals_void(u32 a);
u1 aux_cstr_equals_bool(u32 a);
u1 aux_cstr_equals_char(u32 a);
u1 aux_cstr_equals_short(u32 a);
u1 aux_cstr_equals_int(u32 a);
u1 aux_cstr_equals_long(u32 a);
u1 aux_cstr_equals_panicloop(u32 a);
u1 aux_cstr_equals_define(u32 a);
u1 aux_cstr_equals_printf(u32 a);
u1 aux_cstr_equals_memcmp(u32 a);
u1 aux_cstr_equals_memcpy(u32 a);
u32 aux_cstr_len(u32 str);
u0 print_aux_cstr(u32 index);
u0 print_aux_cstr_err(u32 index);
u0 print_aux_cstr_escaped(u32 index);
u32 find_enum_from_type(u32 type_index);
u1 in_range_inclusive(u8 value, u8 min_inclusive, u8 max_inclusive);

#endif // _PECK_STORAGE_H

