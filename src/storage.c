
#include <stdio.h>
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
u32 num_globals;

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

u32 add_function(Function function){
    if(num_functions >= FUNCTIONS_CAPACITY){
        printf("Out of memory: Exceeded maximum number of functions\n");
        return FUNCTIONS_CAPACITY;
    }

    functions[num_functions] = function;
    return num_functions++;
}

u32 find_function(u32 name){
    for(u32 i = 0; i < FUNCTIONS_CAPACITY; i++){
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

u32 add_global(Global global){
    if(num_globals < GLOBALS_CAPACITY){
        globals[num_globals] = global;
        return num_globals++;
    }

    printf("Out of memory: Exceeded maximum number of global variables\n");
    return GLOBALS_CAPACITY;
}

u32 aux_cstr_alloc(u8 null_terminated_name[16]){
    u8 length = 0;

    while(length < 16 && null_terminated_name[length]){
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

u1 aux_cstr_equals_u32(u32 a){
    return a + 3 < AUX_CAPACITY
        && aux[a    ] == 'u'
        && aux[a + 1] == '3'
        && aux[a + 2] == '2'
        && aux[a + 3] == '\0';
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

