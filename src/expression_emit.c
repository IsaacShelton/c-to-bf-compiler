
#include <stdio.h>
#include "../include/expression_emit.h"
#include "../include/expression.h"
#include "../include/storage.h"
#include "../include/emit.h"
#include "../include/builtin_types.h"
#include "../include/type_emit.h"
#include "../include/type_print.h"
#include "../include/function_emit.h"
#include "../include/variable_find.h"
#include "../include/emit_context.h"

static u32 expression_emit_call(Expression expression){
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
        printf("\nerror: ");
        if(arity < function.arity){
            printf("Not enough");
        } else {
            printf("Too many");
        }
        printf(" arguments for '");
        print_aux_cstr(name);
        printf("', expected %d but got %d\n", function.arity, arity);
        return TYPES_CAPACITY;
    }

    // Make room for return value
    u32 return_size = type_sizeof_or_max(function.return_type);
    if(return_size == -1) return TYPES_CAPACITY;

    for(u32 i = 0; i < return_size; i++){
        printf("[-]>");
    }
    emit_context.current_cell_index += return_size;

    u32 start_function_cell_index = emit_context.current_cell_index;

    // Push Arguments
    for(u32 i = 0; i < arity; i++){
        u32 type = expression_emit(expressions[operands[expression.ops + 2 + i]]);
        if(type == TYPES_CAPACITY) return TYPES_CAPACITY;

        u32 type_size = type_sizeof_or_max(type);
        if(type_size == -1) return TYPES_CAPACITY;

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

    if(function_emit(function, start_function_cell_index, emit_context.current_cell_index)){
        return TYPES_CAPACITY;
    }

    return function.return_type;
}

static u32 expression_emit_assign(Expression expression){
    u32 name = operands[expression.ops];
    Variable variable = variable_find(name);

    u32 new_value = operands[expression.ops + 1];

    if(!variable.defined){
        printf("\nerror: Variable '");
        print_aux_cstr(name);
        printf("' is not defined\n");
        return TYPES_CAPACITY;
    }

    u32 new_value_type = expression_emit(expressions[new_value]);
    if(new_value_type >= TYPES_CAPACITY) return TYPES_CAPACITY;

    if(new_value_type != variable.type){
        printf("\nerror: '");
        print_aux_cstr(name);
        printf("' is '");
        type_print(types[variable.type]);
        printf("' and cannot be assigned to value of type '");
        type_print(types[new_value_type]);
        printf("'\n");
        return TYPES_CAPACITY;
    }

    u32 type_size = type_sizeof_or_max(new_value_type);
    if(type_size == -1) return TYPES_CAPACITY;

    // Point to last cell of data
    printf("<");
    move_cells_static(variable.location.location, type_size, true);
    return u0_type;
}

u32 expression_emit_variable(Expression expression){
    u32 name = expression.ops;

    Variable variable = variable_find(name);

    if(!variable.defined){
        printf("\nerror: Variable '");
        print_aux_cstr(name);
        printf("' is not defined\n");
        return TYPES_CAPACITY;
    }

    u32 size = type_sizeof_or_max(variable.type);
    if(size == -1) return TYPES_CAPACITY;

    if(variable.depth == 0){
        // Global variable
        printf("\nerror: global variable expression not supported yet\n");
        return TYPES_CAPACITY;
    }

    if(variable.location.on_stack){
        // Stack variable
        printf("\nerror: stack variable expression not supported yet\n");
        return TYPES_CAPACITY;
    }

    // Tape variable
    copy_cells_static(variable.location.location, size);
    return variable.type;
}

u32 expression_emit(Expression expression){
    switch(expression.kind){
    case EXPRESSION_DECLARE: {
            u32 variable_size = type_sizeof_or_max(operands[expression.ops]);
            if(variable_size == -1) return 1;

            for(u32 j = 0; j < variable_size; j++){
                printf("[-]>");
            }

            emit_context.current_cell_index += variable_size;
        }
        return u0_type;
    case EXPRESSION_PRINT:
        emit_print_aux_cstr(expression.ops);
        return u0_type;
    case EXPRESSION_CALL:
        return expression_emit_call(expression);
    case EXPRESSION_IMPLEMENT_PUT:
        printf("<.>");
        return u0_type;
    case EXPRESSION_INT:
        printf("[-]%d+>", expression.ops % 256);
        emit_context.current_cell_index++;
        return u8_type;
    case EXPRESSION_U8:
        printf("[-]%d+>", expression.ops % 256);
        emit_context.current_cell_index++;
        return u8_type;
    case EXPRESSION_ASSIGN:
        return expression_emit_assign(expression);
    case EXPRESSION_VARIABLE:
        return expression_emit_variable(expression);
    default:
        printf("\nerror: Unknown expression kind %d during expression_emit\n", expression.kind);
        return TYPES_CAPACITY;
    }
}

