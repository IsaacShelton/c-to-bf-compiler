
#include <stdio.h>
#include "../include/expression_emit.h"
#include "../include/expression.h"
#include "../include/storage.h"
#include "../include/emit.h"
#include "../include/builtin_types.h"
#include "../include/type_emit.h"
#include "../include/type_print.h"
#include "../include/function_emit.h"

static u32 expression_emit_call(Expression expression, u32 current_cell_index){
    u32 name = operands[expression.ops];
    u32 arity = operands[expression.ops + 1];

    int function_index = find_function(name);

    if(function_index >= FUNCTIONS_CAPACITY){
        printf("\nerror: Undeclared function '");
        print_aux_cstr(name);
        printf("'\n");
        return TYPES_CAPACITY;
    }

    Function function = functions[function_index];

    if(function.arity != arity){
        printf("\nerror: Incorrect number of arguments when calling function '");
        print_aux_cstr(name);
        printf("', provided %d but expected %d\n", arity, function.arity);
        return TYPES_CAPACITY;
    }

    // Make room for return value
    u32 return_size = type_sizeof_or_max(function.return_type);
    if(return_size == -1) return TYPES_CAPACITY;

    for(u32 i = 0; i < return_size; i++){
        printf("[-]>");
    }
    current_cell_index += return_size;

    // Push Arguments
    for(u32 i = 0; i < arity; i++){
        u32 type = expression_emit(expressions[operands[expression.ops + 2 + i]], current_cell_index);
        if(type == TYPES_CAPACITY) return TYPES_CAPACITY;

        u32 type_size = type_sizeof_or_max(type);
        if(type_size == -1) return TYPES_CAPACITY;

        current_cell_index += type_size;

        // Ensure type matches expected type
        u32 expected_type = operands[expressions[statements[function.begin + i]].ops];

        if(type != expected_type){
            printf("\nerror: Function '");
            print_aux_cstr(function.name);
            printf("' expects value of type '");
            type_print(types[expected_type]);
            printf("' for argument #%d, but got type '", i + 1);
            type_print(types[type]);
            printf("'\n");
            return TYPES_CAPACITY;
        }
    }

    if(function_emit(function, current_cell_index)){
        return TYPES_CAPACITY;
    }

    return function.return_type;
}

u32 expression_emit(Expression expression, u32 current_cell_index){
    switch(expression.kind){
    case EXPRESSION_DECLARE:
        // Nothing to do
        return u0_type;
    case EXPRESSION_PRINT:
        emit_print_aux_cstr(expression.ops);
        return u0_type;
    case EXPRESSION_CALL:
        return expression_emit_call(expression, current_cell_index);
    case EXPRESSION_IMPLEMENT_PUT:
        printf("<.>");
        return u0_type;
    case EXPRESSION_INT:
        printf("[-]%d+>", expression.ops % 256);
        return u8_type;
    case EXPRESSION_U8:
        printf("[-]%d+>", expression.ops % 256);
        return u8_type;
    default:
        printf("\nerror: Unknown expression kind %d\n", expression.kind);
        return TYPES_CAPACITY;
    }
}

