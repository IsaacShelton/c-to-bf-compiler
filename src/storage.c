
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include "../include/config.h"
#include "../include/storage.h"
#include "../include/type.h"
#include "../include/function.h"
#include "../include/expression.h"
#include "../include/global.h"
#include "../include/type_equals.h"
#include "../include/builtin_types.h"

u8 code_buffer[CODE_BUFFER_CAPACITY];
u32 code_buffer_length = 0;

Token tokens[TOKENS_CAPACITY];
u32 num_tokens = 0;

u8 aux[AUX_CAPACITY];
u32 num_aux = 0;

Type types[TYPES_CAPACITY];
u32 num_types = 0;

u32 dimensions[UNIQUE_DIMENSIONS_CAPACITY][4];
u32 num_dimensions = 1; // First is reserved for all zeros

Function functions[FUNCTIONS_CAPACITY];
u32 num_functions = 0;

Expression expressions[EXPRESSIONS_CAPACITY];
u32 num_expressions = 0;

u32 statements[STATEMENTS_CAPACITY];
u32 num_statements = 0;

u32 operands[OPERANDS_CAPACITY];
u32 num_operands = 0;

Global globals[GLOBALS_CAPACITY];
u32 num_globals = 0;

TypeDef typedefs[TYPEDEFS_CAPACITY];
u32 num_typedefs = 0;

TypeAlias type_aliases[TYPE_ALIASES_CAPACITY];
u32 num_type_aliases = 0;

Define defines[DEFINES_CAPACITY];
u32 num_defines;

CloseNeeded closes_needed[CLOSES_NEEDED_CAPCAITY];
u32 num_closes_needed = 0;

u32 add_type(Type type){
    for(u32 i = 0; i < num_types; i++){
        if(type_equals(type, types[i])){
            return i;
        }
    }

    if(num_types < TYPES_CAPACITY){
        types[num_types] = type;
        return num_types++;
    }

    printf("Out of memory: Exceeded maximum number of types\n");
    return TYPES_CAPACITY;
}

u32 add_dimensions(u32 type_dimensions[4]){
    // Try to find existing slot with same value
    for(u32 i = 0; i < UNIQUE_DIMENSIONS_CAPACITY; i++){
        u1 match = memcmp(dimensions[i], type_dimensions, sizeof(u32[4])) == 0;

        if(match){
            return i;
        }
    }

    // Insert if not found
    if(num_dimensions + 1 >= UNIQUE_DIMENSIONS_CAPACITY){
        return UNIQUE_DIMENSIONS_CAPACITY;
    }

    for(u32 i = 0; i < 4; i++){
        dimensions[num_dimensions][i] = type_dimensions[i];
    }

    return num_dimensions++;
}


u32 add_function(Function function){
    if(num_functions >= FUNCTIONS_CAPACITY){
        printf("Out of memory: Exceeded maximum number of functions\n");
        return FUNCTIONS_CAPACITY;
    }

    functions[num_functions] = function;
    return num_functions++;
}

u32 find_function(u32 name){
    for(u32 i = 0; i < num_functions; i++){
        if(aux_cstr_equals(functions[i].name, name)){
            return i;
        }
    }

    return FUNCTIONS_CAPACITY;
}

u32 find_function_return_type(u32 name){
    u32 function_index = find_function(name);

    if(function_index >= FUNCTIONS_CAPACITY){
        return TYPES_CAPACITY;
    } else {
        return functions[function_index].return_type;
    }
}

u32 add_expression(Expression expression){
    if(num_expressions < EXPRESSIONS_CAPACITY){
        expressions[num_expressions] = expression;
        return num_expressions++;
    } else {
        printf("Out of memory: Exceeded maximum number of total expressions\n");
        return EXPRESSIONS_CAPACITY;
    }
}

u32 add_statement_from_existing(u32 expression_index){
    if(num_statements < STATEMENTS_CAPACITY){
        statements[num_statements] = expression_index;
        return num_statements++;
    } else {
        printf("Out of memory: Exceeded maximum number of total statements\n");
        return STATEMENTS_CAPACITY;
    }
}

u32 add_statement_from_new(Expression expression){
    u32 expression_index = add_expression(expression);

    if(expression_index < EXPRESSIONS_CAPACITY){
        return add_statement_from_existing(expression_index);
    } else {
        return STATEMENTS_CAPACITY;
    }
}

u32 add_operand(u32 a){
    if(num_operands < OPERANDS_CAPACITY){
        operands[num_operands] = a;
        return num_operands++;
    } else {
        printf("Out of memory: Exceeded maximum number of total expression operands\n");
        return OPERANDS_CAPACITY;
    }
}

u32 add_operands2(u32 a, u32 b){
    if(num_operands < OPERANDS_CAPACITY - 1){
        u32 result = num_operands;
        operands[num_operands] = a;
        operands[num_operands + 1] = b;
        num_operands += 2;
        return result;
    } else {
        printf("Out of memory: Exceeded maximum number of total expression operands\n");
        return OPERANDS_CAPACITY;
    }
}

u32 add_operands3(u32 a, u32 b, u32 c){
    if(num_operands < OPERANDS_CAPACITY - 2){
        u32 result = num_operands;
        operands[num_operands] = a;
        operands[num_operands + 1] = b;
        operands[num_operands + 2] = c;
        num_operands += 3;
        return result;
    } else {
        printf("Out of memory: Exceeded maximum number of total expression operands\n");
        return OPERANDS_CAPACITY;
    }
}

u32 add_operands5(u32 a, u32 b, u32 c, u32 d, u32 e){
    if(num_operands < OPERANDS_CAPACITY - 4){
        u32 result = num_operands;
        operands[num_operands] = a;
        operands[num_operands + 1] = b;
        operands[num_operands + 2] = c;
        operands[num_operands + 3] = d;
        operands[num_operands + 4] = e;
        num_operands += 5;
        return result;
    } else {
        printf("Out of memory: Exceeded maximum number of total expression operands\n");
        return OPERANDS_CAPACITY;
    }
}

u32 add_global(Global global){
    if(num_globals < GLOBALS_CAPACITY){
        globals[num_globals] = global;
        return num_globals++;
    }

    printf("Out of memory: Exceeded maximum number of global variables\n");
    return GLOBALS_CAPACITY;
}

u32 add_typedef(TypeDef def){
    if(num_typedefs < TYPEDEFS_CAPACITY){
        typedefs[num_typedefs] = def;
        return num_typedefs++;
    }

    printf("Out of memory: Exceeded maximum number of type definitions\n");
    return TYPEDEFS_CAPACITY;
}

u32 find_typedef(u32 name){
    for(u32 i = 0; i < num_typedefs; i++){
        if(aux_cstr_equals(typedefs[i].name, name)){
            return i;
        }
    }
    return TYPEDEFS_CAPACITY;
}

u32 add_type_alias(TypeAlias alias){
    if(num_type_aliases < TYPE_ALIASES_CAPACITY){
        type_aliases[num_type_aliases] = alias;
        return num_type_aliases++;
    }

    printf("Out of memory: Exceeded maximum number of type aliases\n");
    return TYPE_ALIASES_CAPACITY;
}

u32 try_resolve_type_alias(u32 name){
    for(u32 i = 0; i < num_type_aliases; i++){
        if(aux_cstr_equals(type_aliases[i].name, name)){
            return type_aliases[i].rewritten_type;
        }
    }
    
    return TYPES_CAPACITY;
}

u32 add_define(Define define){
    if(num_defines < DEFINES_CAPACITY){
        defines[num_defines] = define;
        return num_defines++;
    }

    printf("Out of memory: Exceeded maximum number of #define's\n");
    return DEFINES_CAPACITY;
}

u32 try_resolve_define(u32 name){
    for(u32 i = 0; i < num_defines; i++){
        if(aux_cstr_equals(defines[i].name, name)){
            return defines[i].expression;
        }
    }

    return EXPRESSIONS_CAPACITY;
}

u32 aux_cstr_alloc(u8 null_terminated_name[32]){
    u8 length = 0;

    while(length < 32 && null_terminated_name[length]){
        length++;
    }

    if(num_aux + length + 1 >= AUX_CAPACITY){
        printf("Out of memory: Auxiliary memory used up\n");
        return AUX_CAPACITY;
    }

    u32 name = num_aux;
    for(u8 i = 0; i < length; i++){
        aux[num_aux++] = null_terminated_name[i];
    }

    aux[num_aux++] = '\0';
    return name;
}

u1 aux_cstr_equals(u32 a, u32 b){
    for(u32 i = 0; aux[a + i] == aux[b + i]; i++){
        if(aux[a + i] == '\0'){
            return true;
        }
    }
    return false;
}

u1 aux_cstr_equals_string(u32 a, u8 string[32]){
    for(u32 i = 0; i < 32 && aux[a + i] == string[i]; i++){
        if(aux[a + i] == '\0'){
            return true;
        }
    }
    return false;
}

u1 aux_cstr_equals_print(u32 a){
    return a + 5 < AUX_CAPACITY
        && aux[a    ] == 'p'
        && aux[a + 1] == 'r'
        && aux[a + 2] == 'i'
        && aux[a + 3] == 'n'
        && aux[a + 4] == 't'
        && aux[a + 5] == '\0';
}

u1 aux_cstr_equals_main(u32 a){
    return a + 4 < AUX_CAPACITY
        && aux[a    ] == 'm'
        && aux[a + 1] == 'a'
        && aux[a + 2] == 'i'
        && aux[a + 3] == 'n'
        && aux[a + 4] == '\0';
}

u1 aux_cstr_equals_put(u32 a){
    return a + 3 < AUX_CAPACITY
        && aux[a    ] == 'p'
        && aux[a + 1] == 'u'
        && aux[a + 2] == 't'
        && aux[a + 3] == '\0';
}

u1 aux_cstr_equals_u0(u32 a){
    return a + 2 < AUX_CAPACITY
        && aux[a    ] == 'u'
        && aux[a + 1] == '0'
        && aux[a + 2] == '\0';
}

u1 aux_cstr_equals_u1(u32 a){
    return a + 2 < AUX_CAPACITY
        && aux[a    ] == 'u'
        && aux[a + 1] == '1'
        && aux[a + 2] == '\0';
}

u1 aux_cstr_equals_u8(u32 a){
    return a + 2 < AUX_CAPACITY
        && aux[a    ] == 'u'
        && aux[a + 1] == '8'
        && aux[a + 2] == '\0';
}

u1 aux_cstr_equals_u16(u32 a){
    return a + 3 < AUX_CAPACITY
        && aux[a    ] == 'u'
        && aux[a + 1] == '1'
        && aux[a + 2] == '6'
        && aux[a + 3] == '\0';
}

u32 aux_cstr_len(u32 str){
    u32 len = 0;

    while(aux[str + len]){
        len++;
    }

    return len;
}

u1 aux_cstr_equals_u32(u32 a){
    return a + 3 < AUX_CAPACITY
        && aux[a    ] == 'u'
        && aux[a + 1] == '3'
        && aux[a + 2] == '2'
        && aux[a + 3] == '\0';
}

u1 aux_cstr_equals_void(u32 a){
    return a + 4 < AUX_CAPACITY
        && aux[a    ] == 'v'
        && aux[a + 1] == 'o'
        && aux[a + 2] == 'i'
        && aux[a + 3] == 'd'
        && aux[a + 4] == '\0';
}

u1 aux_cstr_equals_bool(u32 a){
    return a + 4 < AUX_CAPACITY
        && aux[a    ] == 'b'
        && aux[a + 1] == 'o'
        && aux[a + 2] == 'o'
        && aux[a + 3] == 'l'
        && aux[a + 4] == '\0';
}

u1 aux_cstr_equals_char(u32 a){
    return a + 4 < AUX_CAPACITY
        && aux[a    ] == 'c'
        && aux[a + 1] == 'h'
        && aux[a + 2] == 'a'
        && aux[a + 3] == 'r'
        && aux[a + 4] == '\0';
}

u1 aux_cstr_equals_short(u32 a){
    return a + 5 < AUX_CAPACITY
        && aux[a    ] == 's'
        && aux[a + 1] == 'h'
        && aux[a + 2] == 'o'
        && aux[a + 3] == 'r'
        && aux[a + 4] == 't'
        && aux[a + 5] == '\0';
}

u1 aux_cstr_equals_int(u32 a){
    return a + 3 < AUX_CAPACITY
        && aux[a    ] == 'i'
        && aux[a + 1] == 'n'
        && aux[a + 2] == 't'
        && aux[a + 3] == '\0';
}

u1 aux_cstr_equals_long(u32 a){
    return a + 4 < AUX_CAPACITY
        && aux[a    ] == 'l'
        && aux[a + 1] == 'o'
        && aux[a + 2] == 'n'
        && aux[a + 3] == 'g'
        && aux[a + 4] == '\0';
}

u1 aux_cstr_equals_panicloop(u32 a){
    return a + 9 < AUX_CAPACITY
        && aux[a    ] == 'p'
        && aux[a + 1] == 'a'
        && aux[a + 2] == 'n'
        && aux[a + 3] == 'i'
        && aux[a + 4] == 'c'
        && aux[a + 5] == 'l'
        && aux[a + 6] == 'o'
        && aux[a + 7] == 'o'
        && aux[a + 8] == 'p'
        && aux[a + 9] == '\0';
}

u1 aux_cstr_equals_define(u32 a){
    return a + 6 < AUX_CAPACITY
        && aux[a    ] == 'd'
        && aux[a + 1] == 'e'
        && aux[a + 2] == 'f'
        && aux[a + 3] == 'i'
        && aux[a + 4] == 'n'
        && aux[a + 5] == 'e'
        && aux[a + 6] == '\0';
}

u1 aux_cstr_equals_printf(u32 a){
    return a + 6 < AUX_CAPACITY
        && aux[a    ] == 'p'
        && aux[a + 1] == 'r'
        && aux[a + 2] == 'i'
        && aux[a + 3] == 'n'
        && aux[a + 4] == 't'
        && aux[a + 5] == 'f'
        && aux[a + 6] == '\0';
}

u1 aux_cstr_equals_memcmp(u32 a){
    return a + 6 < AUX_CAPACITY
        && aux[a    ] == 'm'
        && aux[a + 1] == 'e'
        && aux[a + 2] == 'm'
        && aux[a + 3] == 'c'
        && aux[a + 4] == 'm'
        && aux[a + 5] == 'p'
        && aux[a + 6] == '\0';
}

u1 aux_cstr_equals_memcpy(u32 a){
    return a + 6 < AUX_CAPACITY
        && aux[a    ] == 'm'
        && aux[a + 1] == 'e'
        && aux[a + 2] == 'm'
        && aux[a + 3] == 'c'
        && aux[a + 4] == 'p'
        && aux[a + 5] == 'y'
        && aux[a + 6] == '\0';
}

u0 print_aux_cstr(u32 index){
    for(u32 i = index; aux[i]; i++){
        putchar(aux[i]);
    }
}

u0 print_aux_cstr_escaped(u32 index){
    for(u32 i = index; aux[i]; i++){
        u8 c = aux[i];
        if(c == '\n'){
            printf("\\n");
        } else if(c == '\\'){
            printf("\\\\");
        } else if(c == '\t'){
            printf("\\t");
        } else if(c == '\0'){
            printf("\\0");
        } else {
            putchar(aux[i]);
        }
    }
}

u32 find_enum_from_type(u32 type_index){
    Type type = types[type_index];

    if(type.dimensions != 0){
        return TYPEDEFS_CAPACITY;
    }

    for(u32 i = 0; i < num_typedefs; i++){
        TypeDef def = typedefs[i];

        if(def.kind == TYPEDEF_ENUM && aux_cstr_equals(def.name, type.name)){
            return i;
        }
    }

    return TYPEDEFS_CAPACITY;
}

u1 in_range_inclusive(u8 value, u8 min_inclusive, u8 max_inclusive){
    return value >= min_inclusive && value <= max_inclusive;
}

