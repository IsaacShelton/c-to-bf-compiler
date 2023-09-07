
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

static u0 print_nth_argument_label(u32 number){
    switch(number){
    case 0:
        printf("Zeroth");
        break;
    case 1:
        printf("First");
        break;
    case 2:
        printf("Second");
        break;
    case 3:
        printf("Third");
        break;
    case 4:
        printf("Fourth");
        break;
    case 5:
        printf("Fifth");
        break;
    case 6:
        printf("Sixth");
        break;
    case 7:
        printf("Seventh");
        break;
    case 8:
        printf("Eighth");
        break;
    case 9:
        printf("Ninth");
        break;
    case 10:
        printf("Tenth");
    default:
        printf("%d-th", number);
    }
}

static ErrorCode grow_type(u32 from_type_index, u32 to_type_index){
    Type from_type = types[from_type_index];
    Type to_type = types[to_type_index];

    if(!aux_cstr_equals(from_type.name, to_type.name)){
        return 1;
    }

    if(from_type.dimensions == 0 || to_type.dimensions == 0){
        return 1;
    }

    u32 f[4];
    u32 t[4];

    memcpy(f, &dimensions[from_type.dimensions], sizeof f);
    memcpy(t, &dimensions[to_type.dimensions], sizeof t);

    u8 num_f_dims = 1;
    u8 num_t_dims = 1;

    while(num_f_dims < 4 && f[num_f_dims] != 0){
        num_f_dims++;
    }
    while(num_t_dims < 4 && t[num_t_dims] != 0){
        num_t_dims++;
    }

    if(num_f_dims != num_t_dims){
        return 1;
    }

    u32 t_len = t[num_t_dims - 1];
    u32 f_len = f[num_f_dims - 1];

    if(t_len > f_len){
        // Zero extend array
        while(f_len < t_len){
            printf("[-]>");
            emit_context.current_cell_index++;
            f_len++;
        }

        return 0;
    }

    return 1;
}

static u32 expression_get_type_for_call(Expression expression){
    u32 name = operands[expression.ops];
    int function_index = find_function(name);

    if(function_index >= FUNCTIONS_CAPACITY){
        return TYPES_CAPACITY;
    }

    return functions[function_index].return_type;
}

static u32 expression_get_type_for_string(Expression expression){
    u32 str = expression.ops;
    u32 len = aux_cstr_len(str);

    u32 dims[4];
    memset(dims, 0, sizeof dims);
    dims[0] = len;

    u32 type_dimensions = add_dimensions(dims);
    if(type_dimensions >= UNIQUE_DIMENSIONS_CAPACITY) return TYPES_CAPACITY;

    return add_type((Type){
        .name = types[u8_type].name,
        .dimensions = type_dimensions,
    });
}

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
            if(grow_type(type, expected_type)){
                printf("\nerror on line %d: ", u24_unpack(expression.line));
                print_nth_argument_label(i + 1);
                printf(" argument to function '");
                print_aux_cstr(function.name);
                printf("' should be '");
                type_print(types[expected_type]);
                printf("', but got type '");
                type_print(types[type]);
                printf("'\n");
                return TYPES_CAPACITY;
            }

            type = expected_type;
        }
    }

    if(function_emit(function_index, start_function_cell_index, emit_context.current_cell_index)){
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

static u32 get_item_type(Type type, u1 show_error_message){
    u32 dims[4];
    memcpy(dims, &dimensions[type.dimensions], sizeof(u32) * 4);

    if(dims[0] == 0){
        if(show_error_message){
            printf("\nerror: Cannot index into non-array type '");
            type_print(type);
            printf("'\n");
        }
        return TYPES_CAPACITY;
    }

    // Remove dimension
    for(int i = 0; i < 3; i++){
        dims[i] = dims[i + 1];
    }
    dims[3] = 0;

    u32 type_dimensions = add_dimensions(dims);
    if(type_dimensions >= UNIQUE_DIMENSIONS_CAPACITY) return TYPES_CAPACITY;

    Type item_type = (Type){
        .name = type.name,
        .dimensions = type_dimensions,
    };

    return add_type(item_type);
}

static u1 is_destination(Expression expression){
    switch(expression.kind){
    case EXPRESSION_VARIABLE:
    case EXPRESSION_INDEX:
        return true;
    case EXPRESSION_MEMBER:
        // Is a destination only if subject is a destination
        return is_destination(expressions[operands[expression.ops]]);
    default:
        return false;
    }
}

static Destination destination_member(Destination destination, u32 target_field_name, u24 line_on_error){
    if(destination.offset_size > 0){
        printf("\nerror on line %d: Cannot member into destination that already has an offset yet (not supported)\n", u24_unpack(line_on_error));
        return (Destination) { .type = TYPES_CAPACITY };
    }

    Type type = types[destination.type];
    u1 is_struct_type = true;
    TypeDef def;

    // Ensure not array
    if(type.dimensions != 0){
        is_struct_type = false;
    }

    // Ensure typedef exists
    if(is_struct_type){
        // Get typedef
        u32 def_index = find_typedef(types[destination.type].name);
        if(def_index >= TYPES_CAPACITY){
            is_struct_type = false;
        } else {
            def = typedefs[def_index];
        }
    }

    // Ensure typedef is a struct
    if(is_struct_type){
        if(def.kind != TYPEDEF_STRUCT){
            is_struct_type = false;
        }
    }

    if(!is_struct_type){
        printf("\nerror on line %d: Cannot get '", u24_unpack(line_on_error));
        print_aux_cstr(target_field_name);
        printf("' of non-struct type '");
        type_print(type);
        printf("'\n");
        return (Destination) { .type = TYPES_CAPACITY };
    }

    // Result destination type is invalid unless we find the member
    destination.type = TYPES_CAPACITY;

    for(u32 i = 0; i < def.num_fields; i++){
        Expression expression = expressions[statements[def.begin + i]];
        if(expression.kind != EXPRESSION_DECLARE) continue;

        u32 field_name = operands[expression.ops + 1];
        u32 field_type = operands[expression.ops];

        if(aux_cstr_equals(target_field_name, field_name)){
            destination.type = field_type;
            break;
        }

        u32 field_size = type_sizeof_or_max(field_type);
        if(field_size == -1) return (Destination){ .type = TYPES_CAPACITY };

        destination.tape_location += field_size;
    }

    // If successful, then .type will be less than TYPES_CAPACITY
    return destination;
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

        u32 item_type = get_item_type(types[array_destination.type], true);
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
    } else if(expression.kind == EXPRESSION_MEMBER){
        // Calculate subject destination
        Expression subject_expression = expressions[operands[expression.ops]];
        Destination destination = expression_get_destination(subject_expression, tape_anchor);
        if(destination.type >= TYPES_CAPACITY) return (Destination) { .type = TYPES_CAPACITY };

        return destination_member(destination, operands[expression.ops + 1], expression.line);
    } else {
        printf("\nerror on line %d: Cannot assign value to that destination\n", u24_unpack(expression.line));
        return (Destination){ .type = TYPES_CAPACITY };
    }
}

static u32 write_destination(u32 new_value_type, Destination destination, u24 error_line_number){
    if(new_value_type != destination.type){
        if(grow_type(new_value_type, destination.type)){
            printf("\nerror on line %d: Cannot assign '", u24_unpack(error_line_number));
            type_print(types[new_value_type]);
            printf("' to '");
            type_print(types[destination.type]);
            printf("'\n");
            return TYPES_CAPACITY;
        }

        new_value_type = destination.type;
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
        printf("\nerror on line %d: Cannot assign to destination with u%d offset\n", u24_unpack(error_line_number), 8*destination.offset_size);
        return TYPES_CAPACITY;
    }
}

static u32 write_destination_expression(u32 new_value_type, Expression destination_expression, u24 error_line_number){
    Destination destination = expression_get_destination(destination_expression, emit_context.current_cell_index);
    if(destination.type >= TYPES_CAPACITY) return TYPES_CAPACITY;

    return write_destination(new_value_type, destination, error_line_number);
}

static u32 expression_emit_assign(Expression expression){
    u32 new_value = operands[expression.ops + 1];
    u32 new_value_type = expression_emit(expressions[new_value]);
    if(new_value_type >= TYPES_CAPACITY) return TYPES_CAPACITY;

    return write_destination_expression(new_value_type, expressions[operands[expression.ops]], expression.line);
}

u32 expression_emit_variable(Expression expression){
    u32 name = expression.ops;

    Variable variable = variable_find(name);

    if(!variable.defined){
        printf("\nerror on line %d: Variable '", u24_unpack(expression.line));
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

static u32 expression_get_type_for_variable(Expression expression){
    u32 name = expression.ops;
    Variable variable = variable_find(name);

    if(variable.defined){
        return variable.type;
    }

    printf("\nerror on line %d: Variable '", u24_unpack(expression.line));
    print_aux_cstr(name);
    printf("' is not defined\n");
    return TYPES_CAPACITY;
}

u32 expression_emit_cast(Expression expression){
    u32 from_type = expression_emit(expressions[operands[expression.ops + 1]]);
    if(from_type >= TYPES_CAPACITY) return TYPES_CAPACITY;

    u32 to_type = operands[expression.ops];

    if((from_type == u1_type && to_type == u8_type) || from_type == to_type){
        return to_type;
    }

    if(from_type == u8_type && to_type == u1_type){
        emit_u8(0);
        emit_neq_u8();
        return to_type;
    }

    printf("\nerror on line %d: Cannot cast '", u24_unpack(expression.line));
    type_print(types[from_type]);
    printf("' to '");
    type_print(types[to_type]);
    printf("'\n");
    return TYPES_CAPACITY;
}

static u32 emit_math(ExpressionKind kind, u32 operand_type){
    switch(kind){
    case EXPRESSION_ADD:
        emit_additive_u8(true);
        return operand_type;
    case EXPRESSION_SUBTRACT:
        emit_additive_u8(false);
        return operand_type;
    case EXPRESSION_MULTIPLY:
        emit_multiply_u8();
        return operand_type;
    case EXPRESSION_DIVIDE:
        emit_divide_u8();
        return operand_type;
    case EXPRESSION_MOD:
        emit_mod_u8();
        return operand_type;
    case EXPRESSION_EQUALS:
        emit_eq_u8();
        return u1_type;
    case EXPRESSION_NOT_EQUALS:
        emit_neq_u8();
        return u1_type;
    case EXPRESSION_LESS_THAN:
        emit_lt_u8();
        return u1_type;
    case EXPRESSION_GREATER_THAN:
        emit_gt_u8();
        return u1_type;
    case EXPRESSION_LESS_THAN_OR_EQUAL:
        emit_lte_u8();
        return u1_type;
    case EXPRESSION_GREATER_THAN_OR_EQUAL:
        emit_gte_u8();
        return u1_type;
    case EXPRESSION_LSHIFT:
        emit_lshift_u8();
        return operand_type;
    case EXPRESSION_RSHIFT:
        emit_rshift_u8();
        return operand_type;
    case EXPRESSION_BIT_AND:
        emit_bit_and_u8();
        return operand_type;
    case EXPRESSION_BIT_OR:
        emit_bit_or_u8();
        return operand_type;
    case EXPRESSION_BIT_XOR:
        emit_bit_xor_u8();
        return operand_type;
    default:
        printf("\nerror: Could not perform unknown math operation for expression kind %d\n", kind);
        return 1;
    }

    return 0;
}

u32 expression_emit_unary(Expression expression){
    u32 type = expression_emit(expressions[expression.ops]);
    if(type >= TYPES_CAPACITY) return TYPES_CAPACITY;

    switch(expression.kind){
    case EXPRESSION_NOT:
        if(type == u1_type){
            emit_not_u1();
            return type;
        }
        break;
    case EXPRESSION_NEGATE:
        if(type == u8_type){
            emit_negate_u8();
            return type;
        }
        break;
    case EXPRESSION_BIT_COMPLEMENT:
        if(type == u8_type || type == u1_type){
            emit_bit_complement_u8();
            return type;
        }
        break;
    case EXPRESSION_PRE_INCREMENT:
    case EXPRESSION_PRE_DECREMENT:
        if(type == u8_type){
            emit_u8(1);
            emit_additive_u8(expression.kind == EXPRESSION_PRE_INCREMENT);
            dupe_cell();
            write_destination_expression(u8_type, expressions[expression.ops], expression.line);
            return type;
        }
        break;
    case EXPRESSION_POST_INCREMENT:
    case EXPRESSION_POST_DECREMENT:
        if(type == u8_type){
            dupe_cell();
            emit_u8(1);
            emit_additive_u8(expression.kind == EXPRESSION_POST_INCREMENT);
            write_destination_expression(u8_type, expressions[expression.ops], expression.line);
            return type;
        }
        break;
    case EXPRESSION_NO_RESULT_INCREMENT:
    case EXPRESSION_NO_RESULT_DECREMENT:
        if(type == u8_type){
            emit_u8(1);
            emit_additive_u8(expression.kind == EXPRESSION_NO_RESULT_INCREMENT);
            write_destination_expression(u8_type, expressions[expression.ops], expression.line);
            return u0_type;
        }
        break;
    }

    printf("\nerror on line %d: Cannot ", u24_unpack(expression.line));
    expression_print_operation_name(expression.kind);
    printf(" value of type '");
    type_print(types[type]);
    printf("'\n");
    return TYPES_CAPACITY;
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
        return emit_math(expression.kind, a_type);
    }

    // Allow bitwise operations on u1 types
    if(a_type == u1_type && (
        expression.kind == EXPRESSION_BIT_AND
     || expression.kind == EXPRESSION_BIT_OR
     || expression.kind == EXPRESSION_BIT_XOR
    )){
        return emit_math(expression.kind, a_type);
    }

    printf("\nerror on line %d: Cannot ", u24_unpack(expression.line));
    expression_print_operation_name(expression.kind);
    printf(" values of type '");
    type_print(types[a_type]);
    printf("'\n");
    return TYPES_CAPACITY;
}

static u32 expression_emit_and(Expression expression){
    // Allocate result cell
    printf("[-]>");
    emit_context.current_cell_index++;

    // Evaluate left hand side
    u32 a_type = expression_emit(expressions[operands[expression.ops]]);
    if(a_type >= TYPES_CAPACITY) return TYPES_CAPACITY;

    // Convert to boolean
    if(a_type != u1_type){
        printf("\nerror on line %d: Expected first operand of '&&' to be 'u1'", u24_unpack(expression.line));
        printf(", got '");
        type_print(types[a_type]);
        printf("'\n");
        return TYPES_CAPACITY;
    }

    // Go to first operand
    printf("<");
    emit_context.current_cell_index--;

    // If first operand is non-zero
    printf("[");

    // Zero first operand
    printf("[-]");

    // Evaluate right hand side
    u32 b_type = expression_emit(expressions[operands[expression.ops + 1]]);
    if(b_type >= TYPES_CAPACITY) return TYPES_CAPACITY;

    // Convert to boolean
    if(b_type != u1_type){
        printf("\nerror on line %d: Expected second operand of '&&' to be 'u1'", u24_unpack(expression.line));
        printf(", got '");
        type_print(types[b_type]);
        printf("'\n");
        return TYPES_CAPACITY;
    }

    // Go to second operand
    printf("<");
    emit_context.current_cell_index--;

    // If second operand is non-zero
    printf("[");

    // Set result to 1
    printf("<+>");

    // Zero second operand
    printf("[-]");

    // End if
    printf("]");

    // End if
    printf("]");

    // Remain pointing to the next available cell
    // (nothing to do)

    return u1_type;
}

static u32 expression_emit_or(Expression expression){
    // Allocate result cell
    printf("[-]>");
    emit_context.current_cell_index++;

    // Set whether to evaluate second operand to true
    printf("[-]+");

    // Point to next available cell
    printf(">");
    emit_context.current_cell_index++;

    // Evaluate left hand side
    u32 a_type = expression_emit(expressions[operands[expression.ops]]);
    if(a_type >= TYPES_CAPACITY) return TYPES_CAPACITY;

    // Convert to boolean
    if(a_type != u1_type){
        printf("\nerror on line %d: First operand to '&&' must be a 'u1'\n", u24_unpack(expression.line));
        return TYPES_CAPACITY;
    }

    // Go to first operand
    printf("<");
    emit_context.current_cell_index--;

    // If first operand is non-zero
    printf("[");

    // Set result to 1
    printf("<<+");

    // Set whether to check second operand to false
    printf(">-");

    // Zero first operand
    printf(">[-]");

    // End if
    printf("]");

    // Go to 'whether to check second operand'
    printf("<");
    emit_context.current_cell_index--;

    // If 'whether to check second operand' is non-zero
    printf("[");

    // Evaluate right hand side
    u32 b_type = expression_emit(expressions[operands[expression.ops + 1]]);
    if(b_type >= TYPES_CAPACITY) return TYPES_CAPACITY;

    // Convert to boolean
    if(a_type != u1_type){
        printf("\nerror on line %d: Second operand to '&&' must be a 'u1'\n", u24_unpack(expression.line));
        return TYPES_CAPACITY;
    }

    // Go to second operand
    printf("<");
    emit_context.current_cell_index--;

    // If second operand is non-zero
    printf("[");

    // Set result to 1
    printf("<+>");

    // Zero second operand
    printf("[-]");

    // End if
    printf("]");

    // End if
    printf("]");

    // Remain pointing to the next available cell
    // (nothing to do)

    return u1_type;
}

static u32 read_destination(Destination destination, u24 line_on_error){
    if(destination.on_stack){
        printf("\nerror on line %d: Cannot index into destination on the stack yet (not supported)\n", u24_unpack(line_on_error));
        return TYPES_CAPACITY;
    }

    u32 item_type_size = type_sizeof_or_max(destination.type);
    if(item_type_size == -1) return TYPES_CAPACITY;

    if(destination.offset_size == 0){
        copy_cells_static(destination.tape_location, item_type_size);
        return destination.type;
    } else if(destination.offset_size == 1){
        // u8 indexing
        copy_cells_dynamic_u8(destination.tape_location, item_type_size);
        return destination.type;
    } else {
        printf("\nerror on line %d: Cannot use index of type 'u%d'\n", u24_unpack(line_on_error), destination.offset_size * 8);
        return TYPES_CAPACITY;
    }
}

static u32 expression_emit_read_destination(Expression expression){
    u32 tape_anchor = emit_context.current_cell_index;

    // Calculate subject destination
    Destination destination = expression_get_destination(expression, tape_anchor);
    if(destination.type >= TYPES_CAPACITY) return TYPES_CAPACITY;

    return read_destination(destination, expression.line);
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

static u32 expression_get_type(Expression expression){
    switch(expression.kind){
    case EXPRESSION_DECLARE:
    case EXPRESSION_PRINT_LITERAL:
    case EXPRESSION_PRINT_ARRAY:
    case EXPRESSION_IMPLEMENT_PUT:
    case EXPRESSION_IMPLEMENT_PRINTU1:
    case EXPRESSION_IMPLEMENT_PRINTU8:
        return u0_type;
    case EXPRESSION_IMPLEMENT_GET:
        return u8_type;
    case EXPRESSION_CALL:
        return expression_get_type_for_call(expression);
    case EXPRESSION_U1:
        return u1_type;
    case EXPRESSION_INT:
    case EXPRESSION_U8:
        return u8_type;
    case EXPRESSION_ASSIGN:
        return u0_type;
    case EXPRESSION_VARIABLE:
        return expression_get_type_for_variable(expression);
    case EXPRESSION_CAST:
        return operands[expression.ops];
    case EXPRESSION_ADD:
    case EXPRESSION_SUBTRACT:
    case EXPRESSION_MULTIPLY:
    case EXPRESSION_DIVIDE:
    case EXPRESSION_MOD:
    case EXPRESSION_LSHIFT:
    case EXPRESSION_RSHIFT:
    case EXPRESSION_BIT_AND:
    case EXPRESSION_BIT_OR:
    case EXPRESSION_BIT_XOR:
        return expression_get_type(expressions[operands[expression.ops]]);
    case EXPRESSION_EQUALS:
    case EXPRESSION_NOT_EQUALS:
    case EXPRESSION_LESS_THAN:
    case EXPRESSION_GREATER_THAN:
    case EXPRESSION_LESS_THAN_OR_EQUAL:
    case EXPRESSION_GREATER_THAN_OR_EQUAL:
    case EXPRESSION_AND:
    case EXPRESSION_OR:
        return u1_type;
    case EXPRESSION_INDEX: {
            u32 array_type = expression_get_type(expressions[operands[expression.ops]]);
            if(array_type >= TYPES_CAPACITY) return array_type;
            return get_item_type(types[array_type], false);
        }
    case EXPRESSION_MEMBER: {
            u32 subject_type_index = expression_get_type(expressions[operands[expression.ops]]);
            if(subject_type_index >= TYPES_CAPACITY) return TYPES_CAPACITY;

            Type subject_type = types[subject_type_index];

            if(subject_type.dimensions != 0) return TYPES_CAPACITY;

            u32 def_index = find_typedef(subject_type.name);
            if(def_index >= TYPEDEFS_CAPACITY) return TYPES_CAPACITY;

            TypeDef def = typedefs[def_index];
            if(def.kind != TYPEDEF_STRUCT) return TYPES_CAPACITY;

            u32 target_field_name = operands[expression.ops + 1];

            for(u32 i = 0; i < def.num_fields; i++){
                Expression field = expressions[statements[def.begin + i]];
                if(field.kind != EXPRESSION_DECLARE) continue;

                u32 field_name = operands[field.ops + 1];

                if(aux_cstr_equals(field_name, target_field_name)){
                    u32 field_type = operands[field.ops];
                    return field_type;
                }
            }

            return TYPES_CAPACITY;
        }
    case EXPRESSION_NEGATE:
    case EXPRESSION_NOT:
    case EXPRESSION_BIT_COMPLEMENT:
    case EXPRESSION_PRE_INCREMENT:
    case EXPRESSION_PRE_DECREMENT:
    case EXPRESSION_POST_INCREMENT:
    case EXPRESSION_POST_DECREMENT:
        return expression_get_type(expressions[expression.ops]);
    case EXPRESSION_TERNARY:
        return expression_get_type(expressions[operands[expression.ops + 1]]);
    case EXPRESSION_STRING:
        return expression_get_type_for_string(expression);
    }
    return TYPES_CAPACITY;
}

static u32 expression_emit_ternary(Expression expression){
    // result should_do_else condition

    u32 condition = operands[expression.ops];
    u32 when_true = operands[expression.ops + 1];
    u32 when_false = operands[expression.ops + 2];

    u32 result_type = expression_get_type(expressions[when_true]);
    u32 result_size;

    if(result_type < TYPES_CAPACITY){
        result_size = type_sizeof_or_max(result_type);
        if(result_size == -1) return TYPES_CAPACITY;
    } else {
        printf("\nerror on line %d: Could not determine result size for ternary expression\n", u24_unpack(expression.line));
        return TYPES_CAPACITY;
    }

    // Allocate room for result
    printf("%d>", result_size);
    emit_context.current_cell_index += result_size;

    // Allocate 'should_do_else' cell and initialize to 1
    printf("[-]+>");
    emit_context.current_cell_index++;

    // Evaluate condition
    u32 condition_type = expression_emit(expressions[condition]);
    if(condition_type >= TYPES_CAPACITY) return TYPES_CAPACITY;
    
    if(condition_type != u1_type){
        printf("\nerror on line %d: Expected ternary condition to be 'u1', got '", u24_unpack(expression.line));
        type_print(types[condition_type]);
        printf("'\n");
        return TYPES_CAPACITY;
    }

    // Go to 'condition' cell
    printf("<");
    emit_context.current_cell_index--;

    // If 'condition' cell
    printf("[");

    // Evaluate true branch
    printf("%d<", result_size + 1);
    emit_context.current_cell_index -= result_size + 1;

    u32 when_true_type = expression_emit(expressions[when_true]);
    if(when_true_type == TYPES_CAPACITY) return TYPES_CAPACITY;

    if(when_true_type != result_type){
        printf("\nerror on line %d: Expected true branch of ternary expression to be '", u24_unpack(expression.line));
        type_print(types[result_type]);
        printf("', got '");
        type_print(types[when_true_type]);
        printf("'\n");
        return TYPES_CAPACITY;
    }

    // Zero 'should_do_else' cell and 'condition' cell,
    printf("[-]>[-]");
    emit_context.current_cell_index++;

    // End if
    printf("]");

    // Go to 'should_do_else' cell
    printf("<");
    emit_context.current_cell_index--;

    // If 'should_do_else' cell
    printf("[");

    // Evaluate false branch
    printf("%d<", result_size);
    emit_context.current_cell_index -= result_size;

    u32 when_false_type = expression_emit(expressions[when_false]);
    if(when_false_type == TYPES_CAPACITY) return TYPES_CAPACITY;

    if(when_false_type != result_type){
        printf("\nerror on line %d: Expected false branch of ternary expression to be '", u24_unpack(expression.line));
        type_print(types[result_type]);
        printf("', got '");
        type_print(types[when_false_type]);
        printf("'\n");
        return TYPES_CAPACITY;
    }

    // Zero 'should_do_else' cell,
    // and end if
    printf("[-]");
    printf("]");

    return result_type;
}

static u32 expression_emit_string(Expression expression){
    u32 str = expression.ops;
    u32 len = aux_cstr_len(str);

    for(u32 i = 0; i < len; i++){
        printf("[-]%d+", aux[str + i]);
        printf(">");
    }

    emit_context.current_cell_index += len;

    return expression_get_type_for_string(expression);
}

static u32 expression_emit_return(Expression expression){
    u32 return_type = functions[emit_context.function].return_type;

    u32 value_type = expression_emit(expressions[expression.ops]);
    if(value_type >= TYPES_CAPACITY) return TYPES_CAPACITY;

    if(value_type != return_type){
        if(grow_type(value_type, return_type)){
            printf("\nerror on line %d: ", u24_unpack(expression.line));
            printf("Expected '");
            type_print(types[return_type]);
            printf("' return value, but got type '");
            type_print(types[value_type]);
            printf("'\n");
            return TYPES_CAPACITY;
        }

        value_type = return_type;
    }

    u32 return_type_size = type_sizeof_or_max(return_type);
    if(return_type_size == -1) return TYPES_CAPACITY;

    u32 return_value_location = emit_context.function_cell_index - return_type_size;

    // Point to last cell of data value
    printf("<");
    emit_context.current_cell_index--;

    // Move data value into return value location
    move_cells_static(return_value_location, return_type_size, true);

    // Unmark 'incomplete' cell if function can early return
    if(emit_context.can_function_early_return){
        u32 offset = emit_context.current_cell_index - emit_context.incomplete_cell;
        printf("%d<[-]%d>", offset, offset);
    }

    return u0_type;
}

static u32 expression_emit_break(Expression expression){
    if(!emit_context.can_break){
        printf("\nerror on line %d: Cannot break, nowhere to go\n", u24_unpack(expression.line));
        return TYPES_CAPACITY;
    }

    // Zero 'didnt_break_cell'
    u32 offset = emit_context.current_cell_index - emit_context.didnt_break_cell;
    printf("%d<[-]%d>", offset, offset);
    return u0_type;
}

static u32 expression_emit_continue(Expression expression){
    if(!emit_context.can_continue){
        printf("\nerror on line %d: Cannot continue, nowhere to go\n", u24_unpack(expression.line));
        return TYPES_CAPACITY;
    }

    // Zero 'didnt_continue_cell'
    u32 offset = emit_context.current_cell_index - emit_context.didnt_continue_cell;
    printf("%d<[-]%d>", offset, offset);
    return u0_type;
}

static u32 expression_emit_sizeof_type_u8(Expression expression){
    u32 size = type_sizeof_or_max(expression.ops);

    if(size > 255){
        printf("\nerror on line %d: Cannot get size of '", u24_unpack(expression.line));
        type_print(types[expression.ops]);
        printf("' as 'u8', because its size exceeds 255 (is %d)\n", size);
        return TYPES_CAPACITY;
    }

    emit_u8(size);
    return u8_type;
}

static u32 expression_emit_sizeof_value_u8(Expression expression){
    u32 expression_type = expression_get_type(expressions[expression.ops]);

    if(expression_type > TYPES_CAPACITY){
        printf("\nerror on line %d: Could not determine size of expression, perhaps you have undeclared variables?", u24_unpack(expression.line));
        return TYPES_CAPACITY;
    }

    u32 size = type_sizeof_or_max(expression_type);

    if(size > 255){
        printf("\nerror on line %d: Cannot get size of '", u24_unpack(expression.line));
        type_print(types[expression.ops]);
        printf("' as 'u8', because its size exceeds 255 (is %d)\n", size);
        return TYPES_CAPACITY;
    }

    emit_u8(size);
    return u8_type;
}

static u32 expression_emit_array_initializer(Expression expression){
    u32 element_type = u0_type;
    u32 length = operands[expression.ops];

    for(u32 i = 0; i < length; i++){
        u32 item_type = expression_emit(expressions[operands[expression.ops + 1 + i]]);
        if(item_type >= TYPES_CAPACITY) return item_type;

        if(i == 0){
            element_type = item_type;
        } else if(item_type != element_type){
            printf("\nerror on line %d: Element %d of array initializer has different type than first element\n", u24_unpack(expression.line), i + 1);
            printf("  Expected '");
            type_print(types[element_type]);
            printf("',");
            printf(" got '");
            type_print(types[item_type]);
            printf("'\n");
            return TYPES_CAPACITY;
        }
    }

    u32 new_dims[4];
    memcpy(new_dims, dimensions[types[element_type].dimensions], sizeof new_dims);

    u8 empty_i = 0;
    for(empty_i = 0; empty_i < 4; empty_i++){
        if(new_dims[empty_i] == 0){
            break;
        }
    }

    if(empty_i >= 4){
        printf("\nerror on line %d: Cannot create array intializer with element type that already has maximum number of dimensions\n", u24_unpack(expression.line));
        return TYPES_CAPACITY;
    }

    new_dims[empty_i] = length;

    u32 initializer_type_dimensions = add_dimensions(new_dims);
    if(initializer_type_dimensions >= UNIQUE_DIMENSIONS_CAPACITY) return TYPES_CAPACITY;

    Type initializer_type = (Type){
        .name = types[element_type].name,
        .dimensions = initializer_type_dimensions,
    };

    return add_type(initializer_type);
}

static u32 expression_emit_struct_initializer(Expression expression){
    u32 type = operands[expression.ops];

    u32 type_size = type_sizeof_or_max(type);
    if(type_size == -1) return TYPES_CAPACITY;

    // Allocate space for result
    for(u32 i = 0; i < type_size; i++){
        printf("[-]>");
        emit_context.current_cell_index++;
    }

    // Set fields
    u32 length = operands[expression.ops + 1];

    for(u32 i = 0; i < length; i++){
        Expression field_initializer = expressions[operands[expression.ops + 2 + i]];

        // Assume expression is field initializer expression
        u32 field_name = operands[field_initializer.ops];
        u32 field_value = operands[field_initializer.ops + 1];

        u32 field_value_type = expression_emit(expressions[field_value]);
        if(field_value_type >= TYPES_CAPACITY) return TYPES_CAPACITY;

        Destination destination = (Destination){
            .type = type,
            .on_stack = false,
            .offset_size = 0,
            .tape_location = type_size,
        };
        destination = destination_member(destination, field_name, field_initializer.line);
        if(destination.type >= TYPES_CAPACITY) return TYPES_CAPACITY;

        u32 result = write_destination(field_value_type, destination, field_initializer.line);
        if(result >= TYPES_CAPACITY) return TYPES_CAPACITY;
    }

    return type;
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

                if(get_item_type(types[destination.type], true) != u8_type){
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

                if(get_item_type(types[array_type], true) != u8_type){
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
    case EXPRESSION_IMPLEMENT_PRINTU1:
        emit_printu1();
        return u0_type;
    case EXPRESSION_IMPLEMENT_PRINTU8:
        emit_printu8();
        return u0_type;
    case EXPRESSION_IMPLEMENT_GET:
        printf("<,>");
        return u0_type;
    case EXPRESSION_U1:
        emit_u1(expression.ops);
        return u1_type;
    case EXPRESSION_INT:
    case EXPRESSION_U8:
        emit_u8(expression.ops);
        return u8_type;
    case EXPRESSION_ASSIGN:
        return expression_emit_assign(expression);
    case EXPRESSION_VARIABLE:
        return expression_emit_variable(expression);
    case EXPRESSION_CAST:
        return expression_emit_cast(expression);
    case EXPRESSION_ADD:
    case EXPRESSION_SUBTRACT:
    case EXPRESSION_MULTIPLY:
    case EXPRESSION_DIVIDE:
    case EXPRESSION_MOD:
    case EXPRESSION_EQUALS:
    case EXPRESSION_NOT_EQUALS:
    case EXPRESSION_LESS_THAN:
    case EXPRESSION_GREATER_THAN:
    case EXPRESSION_LESS_THAN_OR_EQUAL:
    case EXPRESSION_GREATER_THAN_OR_EQUAL:
    case EXPRESSION_LSHIFT:
    case EXPRESSION_RSHIFT:
    case EXPRESSION_BIT_AND:
    case EXPRESSION_BIT_OR:
    case EXPRESSION_BIT_XOR:
        return expression_emit_math(expression);
    case EXPRESSION_AND:
        return expression_emit_and(expression);
    case EXPRESSION_OR:
        return expression_emit_or(expression);
    case EXPRESSION_INDEX:
    case EXPRESSION_MEMBER:
        return expression_emit_read_destination(expression);
    case EXPRESSION_NEGATE:
    case EXPRESSION_NOT:
    case EXPRESSION_BIT_COMPLEMENT:
    case EXPRESSION_PRE_INCREMENT:
    case EXPRESSION_PRE_DECREMENT:
    case EXPRESSION_POST_INCREMENT:
    case EXPRESSION_POST_DECREMENT:
    case EXPRESSION_NO_RESULT_INCREMENT:
    case EXPRESSION_NO_RESULT_DECREMENT:
        return expression_emit_unary(expression);
    case EXPRESSION_TERNARY:
        return expression_emit_ternary(expression);
    case EXPRESSION_STRING:
        return expression_emit_string(expression);
    case EXPRESSION_RETURN:
        return expression_emit_return(expression);
    case EXPRESSION_BREAK:
        return expression_emit_break(expression);
    case EXPRESSION_CONTINUE:
        return expression_emit_continue(expression);
    case EXPRESSION_SIZEOF_TYPE:
    case EXPRESSION_SIZEOF_TYPE_U8:
        return expression_emit_sizeof_type_u8(expression);
    case EXPRESSION_SIZEOF_VALUE:
    case EXPRESSION_SIZEOF_VALUE_U8:
        return expression_emit_sizeof_value_u8(expression);
    case EXPRESSION_ARRAY_INITIALIZER:
        return expression_emit_array_initializer(expression);
    case EXPRESSION_STRUCT_INITIALIZER:
        return expression_emit_struct_initializer(expression);
    default:
        printf("\nerror: Unknown expression kind %d during expression_emit\n", expression.kind);
        return TYPES_CAPACITY;
    }
}

