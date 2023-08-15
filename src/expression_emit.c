
#include <stdio.h>
#include <string.h>
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
#include "../include/parse_dimensions.h"

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

typedef struct {
    u32 tape_location;
    u8 offset_size;
    u1 on_stack;
    u32 type; // If >= TYPES_CAPACITY, then error occurred
} Destination;

static u32 get_item_type(Type type){
    u32 dims[4];
    memcpy(dims, &dimensions[type.dimensions], sizeof(u32) * 4);

    if(dims[0] == 0){
        printf("\nerror: Cannot index into non-array type '");
        type_print(type);
        printf("'\n");
        return TYPES_CAPACITY;
    }

    // Remove dimension
    for(int i = 0; i < 3; i++){
        dims[i] = dims[i + 1];
    }
    dims[3] = 0;

    Type item_type = (Type){
        .name = type.name,
        .dimensions = add_dimensions(dims),
    };

    return add_type(item_type);
}

static u1 is_destination(Expression expression){
    switch(expression.kind){
    case EXPRESSION_VARIABLE:
    case EXPRESSION_INDEX:
        return true;
    default:
        return false;
    }
}

static Destination expression_get_destination(Expression expression, u32 tape_anchor){
    // Creates a u32 value on the tape with the mutable location for an expression.
    // If result is `on_stack`, then the u32 offset will be relative to the stack pointer,
    // Otherwise, the u32 offset will be relative to the supplied anchor cell

    // Tape example
    // | 0 | 0 | 0 | 0 | 0 | 7 | 1 | ? | ? | ? | ? |
    //                           ^ chosen anchor cell
    //                           ^ target location is 1 cell away

    // Stack example
    // stack: | 0 | 0 | 0 | 0 | 0 | 7 | ? | ? | ? | ? | ? |
    //                                  ^ stack pointer
    // tape: | ? | ? | ? | ? | ? | ? | 1 | ? | ? | ? | ? |
    //                                 ^ target location is 1 stack cell away

    // A destination result with a type >= TYPES_CAPACITY means an error occurred.
    
    if(expression.kind == EXPRESSION_VARIABLE){
        u32 name = expression.ops;
        Variable variable = variable_find(name);

        if(!variable.defined){
            printf("\nerror on line %d: Variable '", u24_unpack(expression.line));
            print_aux_cstr(name);
            printf("' is not defined\n");
            return (Destination){ .type = TYPES_CAPACITY };
        }

        return (Destination){
            .on_stack = false,
            .tape_location = variable.location.location,
            .type = variable.type,
            .offset_size = 0,
        };
    } else if(expression.kind == EXPRESSION_INDEX){
        // Calculate index
        Expression index_expression = expressions[operands[expression.ops + 1]];
        u32 index_type = expression_emit(index_expression);
        if(index_type >= TYPES_CAPACITY) return (Destination) { .type = TYPES_CAPACITY };

        // Calculate array destination
        Expression array_expression = expressions[operands[expression.ops]];
        Destination array_destination = expression_get_destination(array_expression, tape_anchor);
        if(array_destination.type >= TYPES_CAPACITY) return (Destination) { .type = TYPES_CAPACITY };

        if(array_destination.on_stack){
            printf("\nerror on line %d: Cannot index into destination on the stack yet (not supported)\n", u24_unpack(expression.line));
            return (Destination) { .type = TYPES_CAPACITY };
        }

        if(array_destination.offset_size > 0){
            printf("\nerror on line %d: Cannot index into destination that already has an offset yet (not supported)\n", u24_unpack(expression.line));
            return (Destination) { .type = TYPES_CAPACITY };
        }

        u32 item_type = get_item_type(types[array_destination.type]);
        if(item_type >= TYPES_CAPACITY) return (Destination) { .type = TYPES_CAPACITY };

        if(index_type == u8_type){
            // u8 indexing
            
            return (Destination) {
                .tape_location = array_destination.tape_location,
                .on_stack = false,
                .type = item_type,
                .offset_size = 1,
            };
        } else {
            printf("\nerror on line %d: Cannot use index of type '", u24_unpack(expression.line));
            type_print(types[index_type]);
            printf("'\n");
            return (Destination) { .type = TYPES_CAPACITY };
        }

        return (Destination){ .type = TYPES_CAPACITY };
    } else {
        printf("\nerror on line %d: Cannot assign value to that destination\n", u24_unpack(expression.line));
        return (Destination){ .type = TYPES_CAPACITY };
    }
}

static u32 expression_emit_assign(Expression expression){
    u32 new_value = operands[expression.ops + 1];
    u32 new_value_type = expression_emit(expressions[new_value]);
    if(new_value_type >= TYPES_CAPACITY) return TYPES_CAPACITY;

    Expression destination_expression = expressions[operands[expression.ops]];
    Destination destination = expression_get_destination(destination_expression, emit_context.current_cell_index);
    if(destination.type >= TYPES_CAPACITY) return TYPES_CAPACITY;

    if(new_value_type != destination.type){
        printf("\nerror on line %d: Destination is '", u24_unpack(expression.line));
        type_print(types[destination.type]);
        printf("' and cannot be assigned to value of type '");
        type_print(types[new_value_type]);
        printf("'\n");
        return TYPES_CAPACITY;
    }

    u32 type_size = type_sizeof_or_max(new_value_type);
    if(type_size == -1) return TYPES_CAPACITY;

    // Point to last cell of new-value/index combination
    printf("<");
    emit_context.current_cell_index--;

    if(destination.offset_size == 0){
        move_cells_static(destination.tape_location, type_size, true);
        return u0_type;
    } else if(destination.offset_size == 1){
        move_cells_dynamic_u8(destination.tape_location, type_size);
        return u0_type;
    } else {
        printf("\nerror on line %d: Cannot assign to destination with u%d offset\n", u24_unpack(expression.line), 8*destination.offset_size);
        return TYPES_CAPACITY;
    }
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


static ErrorCode emit_math(ExpressionKind kind){
    switch(kind){
    case EXPRESSION_ADD:
        emit_additive_u8(true);
        return 0;
    case EXPRESSION_SUBTRACT:
        emit_additive_u8(false);
        return 0;
    case EXPRESSION_MULTIPLY:
        emit_multiply_u8();
        return 0;
    case EXPRESSION_DIVIDE:
        emit_divide_u8();
        return 0;
    case EXPRESSION_MOD:
        emit_mod_u8();
        return 0;
    default:
        printf("\nerror: Could not perform unknown math operation for expression kind %d\n", kind);
        return 1;
    }
}

static u32 expression_emit_math(Expression expression){
    u32 a = operands[expression.ops];
    u32 b = operands[expression.ops + 1];

    u32 a_type = expression_emit(expressions[a]);
    if(a_type >= TYPES_CAPACITY) return TYPES_CAPACITY;

    u32 b_type = expression_emit(expressions[b]);
    if(b_type >= TYPES_CAPACITY) return TYPES_CAPACITY;

    if(a_type != b_type){
        printf("\nerror: Cannot ");
        expression_print_operation_name(expression.kind);
        printf(" incompatible types '");
        type_print(types[a_type]);
        printf("' and '");
        type_print(types[b_type]);
        printf("'\n");
        return TYPES_CAPACITY;
    }

    if(a_type == u8_type){
        if(emit_math(expression.kind)) return TYPES_CAPACITY;
        return a_type;
    }

    printf("\nerror: Cannot ");
    expression_print_operation_name(expression.kind);
    printf(" values of type '");
    type_print(types[a_type]);
    printf("'\n");
    return TYPES_CAPACITY;
}

static u32 expression_emit_index(Expression expression){
    // Calculate index
    Expression index_expression = expressions[operands[expression.ops + 1]];
    u32 index_type = expression_emit(index_expression);
    if(index_type >= TYPES_CAPACITY) return TYPES_CAPACITY;

    u32 tape_anchor = emit_context.current_cell_index;

    // Calculate array destination
    Expression array_expression = expressions[operands[expression.ops]];
    Destination array_destination = expression_get_destination(array_expression, tape_anchor);
    if(array_destination.type >= TYPES_CAPACITY) return TYPES_CAPACITY;

    if(array_destination.on_stack){
        printf("\nerror on line %d: Cannot index into destination on the stack yet (not supported)\n", u24_unpack(expression.line));
        return TYPES_CAPACITY;
    }

    if(array_destination.offset_size > 0){
        printf("\nerror on line %d: Cannot index into destination that already has an offset yet (not supported)\n", u24_unpack(expression.line));
        return TYPES_CAPACITY;
    }

    u32 item_type = get_item_type(types[array_destination.type]);
    if(item_type >= TYPES_CAPACITY) return TYPES_CAPACITY;

    if(index_type == u8_type){
        // u8 indexing

        u32 item_type_size = type_sizeof_or_max(item_type);
        if(item_type_size == -1) return TYPES_CAPACITY;

        copy_cells_dynamic_u8(array_destination.tape_location, item_type_size);
        return item_type;
    } else {
        printf("\nerror on line %d: Cannot use index of type '", u24_unpack(expression.line));
        type_print(types[index_type]);
        printf("'\n");
        return TYPES_CAPACITY;
    }
}

ErrorCode print_array_reference(Destination destination, u32 max_length){
    if(destination.offset_size == 0){
        print_cells_static(destination.tape_location, max_length);
    } else {
        printf("\nerror: Cannot print array reference with u%d offset\n", 8*destination.offset_size);
        return 1;
    }

    return 0;
}

u32 expression_emit(Expression expression){
    switch(expression.kind){
    case EXPRESSION_DECLARE: {
            u32 variable_size = type_sizeof_or_max(operands[expression.ops]);
            if(variable_size == -1){
                printf("    In variable declaration on line %d\n", u24_unpack(expression.line));
                return TYPES_CAPACITY;
            }

            for(u32 j = 0; j < variable_size; j++){
                printf("[-]>");
            }

            emit_context.current_cell_index += variable_size;
        }
        return u0_type;
    case EXPRESSION_PRINT_LITERAL:
        emit_print_aux_cstr(expression.ops);
        return u0_type;
    case EXPRESSION_PRINT_ARRAY: {
            if(is_destination(expressions[expression.ops])){
                // Print reference
                u32 tape_anchor = emit_context.current_cell_index;
                Destination destination = expression_get_destination(expressions[expression.ops], tape_anchor);
                if(destination.type >= TYPES_CAPACITY) return TYPES_CAPACITY;

                if(get_item_type(types[destination.type]) != u8_type){
                    printf("\nerror on line %d: Cannot print non u8[] value\n", u24_unpack(expression.line));
                    return TYPES_CAPACITY;
                }

                u32 max_length = dimensions[types[destination.type].dimensions][0];
                if(print_array_reference(destination, max_length)){
                    return TYPES_CAPACITY;
                }
            } else {
                // Print value
                u32 array_type = expression_emit(expressions[expression.ops]);
                if(array_type >= TYPES_CAPACITY) return TYPES_CAPACITY;

                if(get_item_type(types[array_type]) != u8_type){
                    printf("\nerror on line %d: Cannot print non u8[] value\n", u24_unpack(expression.line));
                    return TYPES_CAPACITY;
                }

                u32 max_length = dimensions[types[array_type].dimensions][0];
                emit_print_array_value(max_length);
            }
        }
        return u0_type;
    case EXPRESSION_CALL:
        return expression_emit_call(expression);
    case EXPRESSION_IMPLEMENT_PUT:
        printf("<.>");
        return u0_type;
    case EXPRESSION_IMPLEMENT_PRINTU8:
        emit_printu8(); // Print value
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
    case EXPRESSION_ADD:
    case EXPRESSION_SUBTRACT:
    case EXPRESSION_MULTIPLY:
    case EXPRESSION_DIVIDE:
    case EXPRESSION_MOD:
        return expression_emit_math(expression);
    case EXPRESSION_INDEX:
        return expression_emit_index(expression);
    default:
        printf("\nerror: Unknown expression kind %d during expression_emit\n", expression.kind);
        return TYPES_CAPACITY;
    }
}

