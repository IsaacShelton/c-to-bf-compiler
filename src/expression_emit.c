
#include <stdio.h>
#include <string.h>
#include "../include/io.h"
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
#include "../include/expression_print.h"
#include "../include/expression_get_type.h"
#include "../include/stack_driver.h"
#include "../include/expression_get_type.h"

static u32 expression_emit_string(Expression expression);

static u0 print_nth_argument_label(u8 number){
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

ErrorCode grow_type(u32 from_type_index, u32 to_type_index, u8 offset_size, u24 line_on_error){
    Type from_type = types[from_type_index];
    Type to_type = types[to_type_index];

    if(!aux_cstr_equals(from_type.name, to_type.name)){
        return 1;
    }

    if(from_type.dimensions == 0 || to_type.dimensions == 0){
        return 1;
    }

    u32 base_type_index = add_type((Type){
        .name = from_type.name,
        .dimensions = (u32) 0,
    });
    if(base_type_index >= TYPES_CAPACITY) return 1;

    u32 base_type_size = type_sizeof_or_max(base_type_index, line_on_error);
    if(base_type_size == -1) return 1;

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
        u32 difference = t_len - f_len;

        // If we have an offset, then move it forward
        if(offset_size > 0){
            u32 new_offset_position = emit_context.current_cell_index - (u32) offset_size + 2 + difference;
            move_cells_static(new_offset_position, (u32) offset_size);
        }

        // Zero extend array
        while(f_len < t_len){
            for(u32 i = 0; i < base_type_size; i++){
                printf("[-]>");
                emit_context.current_cell_index++;
            }
            f_len++;
        }

        if(offset_size > 0){
            printf("%d>", offset_size);
            emit_context.current_cell_index += (u32) offset_size;
        }

        return 0;
    }

    return 1;
}

static u32 expression_emit_printf(Expression expression){
    u8 arity = (u8) operands[expression.ops + 1];

    if(arity < 1){
        printf("\nerror on line %d: printf requires at least one argument\n", u24_unpack(expression.line));
        return TYPES_CAPACITY;
    }

    Expression format_argument = expressions[operands[expression.ops + 2]];

    if(format_argument.kind != EXPRESSION_STRING){
        printf("\nerror on line %d: First argument to 'printf' must be a compile-time known string\n", u24_unpack(expression.line));
        return TYPES_CAPACITY;
    }

    u32 format = format_argument.ops;
    u8 next_argument_i = 1;
    u8 c = aux[format];

    while(c != 0){
        // Print until a '%'

        printf("[-]");
        u8 previous_value = 0;

        while(c != 0){
            if(c == '%'){
                c = aux[++format];

                if(c == 'd'){
                    format++;
                    break;
                } else if(c == 'c'){
                    format++;
                    break;
                } else if(c != '%'){
                    printf("\nerror on line %d: Unknown printf format '%%", u24_unpack(expression.line));
                    put(c);
                    printf("'\n");
                    return TYPES_CAPACITY;
                }
            }

            set_cell_to_value(c, previous_value);
            printf(".");
            previous_value = c;
            c = aux[++format];
        }

        if(c != 0){
            if(next_argument_i == arity){
                printf("\nerror on line %d: No value specified for printf format '%%%c'", u24_unpack(expression.line), c);
                return TYPES_CAPACITY;
            }

            Expression argument = expressions[operands[expression.ops + 2 + (u32) next_argument_i++]];
            u32 argument_type = expression_emit(argument);
            if(argument_type >= TYPES_CAPACITY) return TYPES_CAPACITY;

            if(c == 'd'){
                if(argument_type == u1_type){
                    emit_printu1();
                } else if(argument_type == u8_type){
                    emit_printu8();
                } else if(argument_type == u16_type){
                    emit_printu16();
                } else if(argument_type == u32_type){
                    emit_printu32();
                } else {
                    printf("\nerror on line %d: Cannot print '", u24_unpack(expression.line));
                    type_print(types[argument_type]);
                    printf("' with printf format specifier '%%d'\n");
                    return TYPES_CAPACITY;
                }
                
                c = aux[format];
            } else if(c == 'c'){
                if(argument_type != u8_type){
                    printf("\nerror on line %d: Cannot print '", u24_unpack(expression.line));
                    type_print(types[argument_type]);
                    printf("' with printf format specifier '%%c'\n");
                    return TYPES_CAPACITY;
                }
                
                printf("<.>");
                c = aux[format];
            }
        }
    }

    if(next_argument_i != arity){
        printf("\nerror on line %d: printf was supplied with %d unused arguments\n", u24_unpack(expression.line), arity - next_argument_i);
        return TYPES_CAPACITY;
    }

    return u0_type;
}

typedef struct {
   u32 value;
   u1 ok;
} CompileTimeInteger;

static CompileTimeInteger as_compile_time_integer(Expression bytes_argument){
    u32 value;

    if(bytes_argument.kind == EXPRESSION_INT){
        // Integer
        value = bytes_argument.ops;
    } else if(bytes_argument.kind == EXPRESSION_SIZEOF_TYPE){
        // sizeof(Type)
        value = type_sizeof_or_max(bytes_argument.ops, bytes_argument.line);
        if(value == -1) return (CompileTimeInteger){ .ok = false };
    } else if(bytes_argument.kind == EXPRESSION_SIZEOF_VALUE){
        // sizeof value
        u32 type = expression_get_type(expressions[bytes_argument.ops], EXPRESSION_GET_TYPE_MODE_PRINT_ERROR);
        if(type >= TYPES_CAPACITY) return (CompileTimeInteger){ .ok = false };

        value = type_sizeof_or_max(type, bytes_argument.line);
        if(value == -1) return (CompileTimeInteger){ .ok = false };
    } else {
        // Invalid
        return (CompileTimeInteger){ .ok = false };
    }

    return (CompileTimeInteger){ .ok = true, .value = value };
}

static u32 expression_emit_memcmp(Expression expression){
    u8 arity = (u8) operands[expression.ops + 1];

    if(arity != 3){
        printf("\nerror on line %d: memcmp requires three arguments\n", u24_unpack(expression.line));
        return TYPES_CAPACITY;
    }

    Expression bytes_argument = expressions[operands[expression.ops + 2 + 2]];
    CompileTimeInteger integer = as_compile_time_integer(bytes_argument);

    if(!integer.ok){
        printf("\nerror on line %d: Third argument to memcmp must be a compile-time known integer\n", u24_unpack(bytes_argument.line));
        return TYPES_CAPACITY;
    }

    u32 bytes = integer.value;

    if(bytes == 0){
        emit_u8(0);
        return u8_type;
    }

    u32 type_dimensions[4] = { bytes };
    Type new_type_layout = (Type){
        .name = types[u8_type].name,
        .dimensions = add_dimensions(type_dimensions),
    };
    u32 new_type = add_type(new_type_layout);
    if(new_type >= TYPES_CAPACITY) return TYPES_CAPACITY;

    Destination destination;
    Expression argument;

    argument = expressions[operands[expression.ops + 2]];
    if(argument.kind == EXPRESSION_STRING){
        if(expression_emit_string(argument) >= TYPES_CAPACITY){
            return TYPES_CAPACITY;
        }
    } else {
        destination = expression_get_destination(argument, emit_context.current_cell_index);
        if(destination.type >= TYPES_CAPACITY) return 1;
        destination.type = new_type;
        if(read_destination(destination, expression.line) >= TYPES_CAPACITY){
            return TYPES_CAPACITY;
        }
    }

    argument = expressions[operands[expression.ops + 2 + 1]];
    if(argument.kind == EXPRESSION_STRING){
        if(expression_emit_string(argument) >= TYPES_CAPACITY){
            return TYPES_CAPACITY;
        }
    } else {
        destination = expression_get_destination(argument, emit_context.current_cell_index);
        if(destination.type >= TYPES_CAPACITY) return TYPES_CAPACITY;
        destination.type = new_type;
        if(read_destination(destination, expression.line) >= TYPES_CAPACITY){
            return TYPES_CAPACITY;
        }
    }

    // Allocate not done and result cell
    printf(">[-]+>[-]<");

    for(u32 j = 0; j < bytes; j++){
        u32 i = bytes - 1 - j;
        
        // If not done
        printf("[");

        // Zero initialize temporary cell
        printf("<[-]");

        // Go to Y
        printf("%d<", i + 1);

        // Set X to X - Y
        printf("[%d<-%d>+%d<-]", bytes, bytes + i + 1, i + 1);
        printf("%d>[%d<+%d>-]", i + 1, i + 1, i + 1);

        // Move X' to temporary cell
        u32 x_distance = bytes + i + 1;
        printf("%d<[-%d>+%d<]%d>", x_distance, x_distance, x_distance, x_distance);

        // If temporary cell is not zero
        printf("[");
        
        // Move temporary cell to result cell
        printf("[>>+<<-]");

        // Set 'not done' to false
        printf(">[-]<");

        printf("]");

        // Go to 'not done' cell
        printf(">");

        // Mark as done if out of bytes
        if(j + 1 == bytes){
            printf("[-]");
        }
    }

    // Close if's
    printf("%d]", bytes);

    // Go to result cell
    printf(">");

    // Copy result cell to final position
    u32 distance = 2 + 2 * bytes;
    printf("%d<[-]%d>[-%d<+%d>]", distance, distance, distance, distance);

    // Point to next available cell
    printf("%d<", distance - 1);
    emit_context.current_cell_index -= 2 * bytes - 1;
    return u8_type;
}

static u32 expression_emit_memcpy(Expression expression){
    u8 arity = (u8) operands[expression.ops + 1];

    if(arity != 3){
        printf("\nerror on line %d: memcpy requires three arguments\n", u24_unpack(expression.line));
        return TYPES_CAPACITY;
    }

    Expression bytes_argument = expressions[operands[expression.ops + 2 + 2]];
    CompileTimeInteger integer = as_compile_time_integer(bytes_argument);

    if(!integer.ok){
        printf("\nerror on line %d: Third argument to memcpy must be a compile-time known integer\n", u24_unpack(bytes_argument.line));
        return TYPES_CAPACITY;
    }

    u32 bytes = integer.value;

    if(bytes == 0){
        return u0_type;
    }

    u32 type_dimensions[4] = { bytes };
    Type new_type_layout = (Type){
        .name = types[u8_type].name,
        .dimensions = add_dimensions(type_dimensions),
    };
    u32 new_type = add_type(new_type_layout);
    if(new_type >= TYPES_CAPACITY) return TYPES_CAPACITY;

    // Read N bytes of source
    Destination destination;
    Expression argument;
    argument = expressions[operands[expression.ops + 2 + 1]];
    if(argument.kind == EXPRESSION_STRING){
        if(expression_emit_string(argument) >= TYPES_CAPACITY){
            return TYPES_CAPACITY;
        }
    } else {
        destination = expression_get_destination(argument, emit_context.current_cell_index);
        if(destination.type >= TYPES_CAPACITY) return TYPES_CAPACITY;
        destination.type = new_type;
        if(read_destination(destination, expression.line) >= TYPES_CAPACITY){
            return TYPES_CAPACITY;
        }
    }

    // Get destination
    argument = expressions[operands[expression.ops + 2]];
    destination = expression_get_destination(argument, emit_context.current_cell_index);
    if(destination.type >= TYPES_CAPACITY) return 1;
    destination.type = new_type;

    return write_destination_unsafe(new_type, destination, expression.line);
}

static u32 expression_emit_call(Expression expression){
    u32 name = operands[expression.ops];
    u8 arity = (u8) operands[expression.ops + 1];

    u32 function_index = find_function(name);

    if(function_index >= FUNCTIONS_CAPACITY){
        printf("\nerror on line %d: Undeclared function '", u24_unpack(expression.line));
        print_aux_cstr(name);
        printf("'\n");
        return TYPES_CAPACITY;
    }

    /*
        RETURN VALUE SPACE
        ------------------------ function start index
        ARGUMENTS
        EARLY RETURN CELL?
        LOCAL VARIABLES
    */

    Function function = functions[function_index];

    if(function.arity != arity){
        printf("\nerror on line %d: ", u24_unpack(expression.line));
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

    u32 return_size = type_sizeof_or_max(function.return_type, function.line);
    if(return_size == -1) return TYPES_CAPACITY;

    // Make room for return value
    for(u32 i = 0; i < return_size; i++){
        printf("[-]>");
    }
    emit_context.current_cell_index += return_size;

    u32 continuation_basicblock_id;

    if(function.is_recursive){
        continuation_basicblock_id = emit_settings.next_basicblock_id++;
        emit_u32(continuation_basicblock_id);
    }

    u32 start_function_cell_index = emit_context.current_cell_index;

    // Push Arguments
    for(u8 i = 0; i < arity; i++){
        Expression argument = expressions[operands[expression.ops + 2 + (u32) i]];

        u32 type = expression_emit(argument);
        if(type >= TYPES_CAPACITY) return TYPES_CAPACITY;

        u32 type_size = type_sizeof_or_max(type, argument.line);
        if(type_size == -1) return TYPES_CAPACITY;

        // Ensure type matches expected type
        u32 expected_type = operands[expressions[statements[function.begin + (u32) i]].ops];

        if(type != expected_type){
            if(grow_type(type, expected_type, 0, argument.line) != 0){
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

    if(function.is_recursive){
        u32 args_size = function_args_size(function);
        u32 pushed = emit_stack_driver_push_all();
        u32 continuation_memory_count = pushed - args_size - 4;

        emit_u32(basicblock_id_for_function(function_index));
        emit_stack_push_n(4);
        emit_end_basicblock();

        emit_start_basicblock(continuation_basicblock_id);
        emit_stack_pop_n(continuation_memory_count);
    } else {
        if(function_emit(function_index, start_function_cell_index, emit_context.current_cell_index) != 0){
            return TYPES_CAPACITY;
        }
    }

    return function.return_type;
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

static Destination destination_member(Destination destination, u32 name, u24 line_on_error){
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
        if(destination.type == u16_type && aux[name] == '_' && in_range_inclusive(aux[name + 1], '0', '1') && aux[name + 2] == '\0'){
            destination.type = u8_type;
            destination.tape_location += (u32) (aux[name + 1] - '0');
            return destination;
        } else if(destination.type == u32_type && aux[name] == '_' && in_range_inclusive(aux[name + 1], '0', '3') && aux[name + 2] == '\0'){
            destination.type = u8_type;
            destination.tape_location += (u32)(aux[name + 1] - '0');
            return destination;
        } else if(destination.type == u32_type && aux[name] == '_' && in_range_inclusive(aux[name + 1], '0', '1') && aux[name + 2] == 'u' && aux[name + 3] == '1' && aux[name + 4] == '6' && aux[name + 5] == '\0'){
            destination.type = u16_type;
            destination.tape_location += (u32) (2 * (aux[name + 1] - '0'));
            return destination;
        } else {
            printf("\nerror on line %d: Cannot get '", u24_unpack(line_on_error));
            print_aux_cstr(name);
            printf("' of non-struct type '");
            type_print(type);
            printf("'\n");
            return (Destination) { .type = TYPES_CAPACITY };
        }
    }

    // Result destination type is invalid unless we find the member
    destination.type = TYPES_CAPACITY;

    for(u32 i = 0; i < def.num_fields; i++){
        Expression expression = expressions[statements[def.begin + i]];
        if(expression.kind != EXPRESSION_DECLARE) continue;

        u32 field_name = operands[expression.ops + 1];
        u32 field_type = operands[expression.ops];

        if(aux_cstr_equals(field_name, name)){
            destination.type = field_type;
            break;
        }

        u32 field_size = type_sizeof_or_max(field_type, expression.line);
        if(field_size == -1) return (Destination){ .type = TYPES_CAPACITY };

        destination.tape_location += field_size;
    }

    if(destination.type == TYPES_CAPACITY){
        printf("\nerror on line %d: Field '", u24_unpack(line_on_error));
        print_aux_cstr(name);
        printf("' does not exist on type '");
        type_print(type);
        printf("'\n");
        /* fall through */
    }

    // If successful, then .type will be less than TYPES_CAPACITY
    return destination;
}

static Destination destination_index(
    Destination array_destination,
    u32 index_expression_index,
    u24 line_on_error
){
    // Ensure not on stack
    if(array_destination.on_stack){
        printf("\nerror on line %d: Cannot index into destination on the stack yet (not supported)\n", u24_unpack(line_on_error));
        return (Destination) { .type = TYPES_CAPACITY };
    }
    
    // Get type of new index
    Expression index_expression = expressions[index_expression_index];
    u32 index_type = expression_get_type(index_expression, EXPRESSION_GET_TYPE_MODE_NONE);

    if(index_type >= TYPES_CAPACITY){
        printf("\nerror on line %d: Could not predict type used for index\n", u24_unpack(line_on_error));
        return (Destination) { .type = TYPES_CAPACITY };
    }

    // Get size of new index
    u8 index_type_size = 0;

    if(index_type == u8_type || index_type == u16_type || index_type == u24_type || index_type == u32_type){
        index_type_size = (u8) type_sizeof_or_max(index_type, u24_pack(0));
        if(index_type_size == (u8) -1) return (Destination){ .type = TYPES_CAPACITY };
    } else {
        printf("\nerror on line %d: Cannot use index of type '", u24_unpack(index_expression.line));
        type_print(types[index_type]);
        printf("'\n");
        return (Destination) { .type = TYPES_CAPACITY };
    }

    // Get array item destination type
    u32 item_type = get_item_type(types[array_destination.type], line_on_error, true);
    if(item_type >= TYPES_CAPACITY) return (Destination) { .type = TYPES_CAPACITY };

    // Get size of array item destination type
    u32 item_size = type_sizeof_or_max(item_type, line_on_error);
    if(item_size == -1) return (Destination) { .type = TYPES_CAPACITY };

    // Determine target offset size
    u8 offset_size = index_type_size >= array_destination.offset_size
        ? index_type_size
        : array_destination.offset_size;

    // If we're going to need to multiply, increase
    // the offset size by 1 if we support it
    u8 max_offset_size = 4;
    if(item_size != 1 && array_destination.offset_size != 0 && offset_size < max_offset_size){
        offset_size++;
    }

    // Automatically upgrade u24 indexing to u32 indexing for now since u24 indexing is not supported yet
    if(offset_size == 3){
        u32 todo_this_code_add_u24_indexing_instead;
        offset_size = 4;
    }

    // Pad original index
    if(array_destination.offset_size != 0){
        while(array_destination.offset_size < offset_size){
            printf("[-]>");
            array_destination.offset_size++;
            emit_context.current_cell_index++;
        }
    }

    // Emit index
    u32 actual_index_type = expression_emit(index_expression);
    if(actual_index_type >= TYPES_CAPACITY) return (Destination) { .type = TYPES_CAPACITY };

    // Pad new index
    while(index_type_size < offset_size){
        printf("[-]>");
        index_type_size++;
        emit_context.current_cell_index++;
    }

    // Perform appropriate operation for offset size
    if(offset_size == 1){
        // Multiply index by item size (unless item size is 1)
        if(item_size != 1){
            emit_u8((u8) item_size);
            emit_multiply_u8();
        }

        if(array_destination.offset_size != 0){
            emit_additive_u8(true);
        }
    } else if(offset_size == 2){
        // Multiply index by item size (unless item size is 1)
        if(item_size != 1){
            emit_u16((u16) item_size);
            emit_multiply_u16();
        }

        if(array_destination.offset_size != 0){
            emit_additive_u16(true);
        }
    } else if(offset_size == 3){
        fprintf(stderr, "error: u24 indexing not supported yet, exiting...\n");
        return (Destination) { .type = TYPES_CAPACITY };
    } else if(offset_size == 4){
        // Multiply index by item size (unless item size is 1)
        if(item_size != 1){
            emit_u32((u32) item_size);
            emit_multiply_u32();
        }

        if(array_destination.offset_size != 0){
            emit_additive_u32(true);
        }
    } else {
        printf("\nerror on line %d: Cannot perform indexing for offset size ", u24_unpack(index_expression.line));
        printf("%d\n", offset_size);
        printf("\n");
        return (Destination) { .type = TYPES_CAPACITY };
    }

    return (Destination) {
        .tape_location = array_destination.tape_location,
        .on_stack = false,
        .type = item_type,
        .offset_size = offset_size,
    };
}

Destination expression_get_destination(Expression expression, u32 tape_anchor){
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
        // Emit subject destination
        Expression subject_expression = expressions[operands[expression.ops]];
        Destination destination = expression_get_destination(subject_expression, tape_anchor);
        if(destination.type >= TYPES_CAPACITY) return (Destination) { .type = TYPES_CAPACITY };

        // Emit destination modification
        return destination_index(destination, operands[expression.ops + 1], expression.line);
    } else if(expression.kind == EXPRESSION_MEMBER){
        // Emit subject destination
        Expression subject_expression = expressions[operands[expression.ops]];
        Destination destination = expression_get_destination(subject_expression, tape_anchor);
        if(destination.type >= TYPES_CAPACITY) return (Destination) { .type = TYPES_CAPACITY };

        // Emit destination modification
        return destination_member(destination, operands[expression.ops + 1], expression.line);
    } else {
        printf("\nerror on line %d: Cannot get destination for expression `", u24_unpack(expression.line));
        expression_print(expression);
        printf("`\n");

        return (Destination){ .type = TYPES_CAPACITY };
    }
}

u32 write_destination(u32 new_value_type, Destination destination, u24 line_on_error){
    if(new_value_type != destination.type){
        if(grow_type(new_value_type, destination.type, destination.offset_size, line_on_error) != 0){
            printf("\nerror on line %d: Cannot assign '", u24_unpack(line_on_error));
            type_print(types[new_value_type]);
            printf("' to '");
            type_print(types[destination.type]);
            printf("'\n");
            return TYPES_CAPACITY;
        }

        new_value_type = destination.type;
    }

    return write_destination_unsafe(new_value_type, destination, line_on_error);
}

u32 write_destination_unsafe(u32 new_value_type, Destination destination, u24 line_on_error){
    u32 type_size = type_sizeof_or_max(new_value_type, line_on_error);
    if(type_size == -1) return TYPES_CAPACITY;

    if(destination.offset_size == 0){
        move_cells_static(destination.tape_location, type_size);
        return u0_type;
    } else if(destination.offset_size == 1){
        move_cells_dynamic_u8(destination.tape_location, type_size);
        return u0_type;
    } else if(destination.offset_size == 2){
        move_cells_dynamic_u16(destination.tape_location, type_size);
        return u0_type;
    } else if(destination.offset_size == 3){
        move_cells_dynamic_u24(destination.tape_location, type_size);
        return u0_type;
    } else if(destination.offset_size == 4){
        move_cells_dynamic_u32(destination.tape_location, type_size);
        return u0_type;
    } else {
        printf("\nerror on line %d: Cannot assign to destination with u%d offset\n", u24_unpack(line_on_error), 8*destination.offset_size);
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

    u32 size = type_sizeof_or_max(variable.type, expression.line);
    if(size == -1) return TYPES_CAPACITY;

    if(variable.location.kind == VARIABLE_LOCATION_IMMUTABLE){
        if(size == 1){
            emit_u8((u8) variable.location.location);
        } else if(size == 2){
            emit_u16((u16) variable.location.location);
        } else if(size == 3){
            emit_u24(u24_pack(variable.location.location));
        } else if(size == 4){
            emit_u32((u32) variable.location.location);
        } else {
            printf("\nerror on line %d: Cannot emit immutable value of size %d\n", u24_unpack(expression.line), size);
            return TYPES_CAPACITY;
        }
        return variable.type;
    }

    if(variable.location.kind != VARIABLE_LOCATION_ON_TAPE){
        // Stack variable
        printf("\nerror: stack/immutable variable expression not supported yet\n");
        return TYPES_CAPACITY;
    }

    if(variable.depth == 0){
        // Global variable
        copy_cells_static(variable.location.location, size);
        return variable.type;
    }

    // Tape variable
    copy_cells_static(variable.location.location, size);
    return variable.type;
}

u32 expression_emit_cast(Expression expression){
    u32 from_type = expression_emit(expressions[operands[expression.ops + 1]]);
    if(from_type >= TYPES_CAPACITY) return TYPES_CAPACITY;

    u32 to_type = operands[expression.ops];

    /* Allow self-casting and casting between u1 and u8 */
    if((from_type == u1_type && to_type == u8_type) || from_type == to_type){
        return to_type;
    }

    /* Allow casting to u1 */
    if(to_type == u1_type){
        if(from_type == u8_type){
            emit_u8(0);
            emit_neq_u8();
            return to_type;
        }

        if(from_type == u16_type){
            emit_u16(0);
            emit_neq_u16();
            return to_type;
        }

        if(from_type == u32_type){
            emit_u32(0);
            emit_neq_u32();
            return to_type;
        }
    }

    /* Allow valid casting from 1 cell to 2 cell types */
    if((from_type == u1_type || from_type == u8_type) && to_type == u16_type){
        printf("[-]>");
        emit_context.current_cell_index++;
        return to_type;
    }

    /* Allow valid casting from 2 cell to 4 cell types */
    if(from_type == u16_type && to_type == u32_type){
        printf("[-]>[-]>");
        emit_context.current_cell_index += 2;
        return to_type;
    }

    /* Allow valid casting from 1 cell to 4 cell types */
    if((from_type == u1_type || from_type == u8_type) && to_type == u32_type){
        printf("[-]>[-]>[-]>");
        emit_context.current_cell_index += 3;
        return to_type;
    }

    u32 from_enum = find_enum_from_type(from_type);
    u32 to_enum = find_enum_from_type(to_type);

    // Try to cast from enum to integer
    if(from_enum != TYPEDEFS_CAPACITY){
        TypeDef def = typedefs[from_enum];

        if(to_type == u8_type && def.computed_size == 1){
            return to_type;
        } else if(to_type == u16_type && def.computed_size == 2){
            return to_type;
        } else if(to_type == u24_type && def.computed_size == 3){
            return to_type;
        } else if(to_type == u32_type && def.computed_size == 4){
            return to_type;
        }
    }

    if(to_enum != TYPEDEFS_CAPACITY){
        TypeDef def = typedefs[to_enum];

        if(from_type == u8_type && def.computed_size == 1){
            return to_type;
        } else if(from_type == u16_type && def.computed_size == 2){
            return to_type;
        } else if(from_type == u24_type && def.computed_size == 3){
            return to_type;
        } else if(from_type == u32_type && def.computed_size == 4){
            return to_type;
        }
    }

    if(to_type == u0_type){
        u32 size = type_sizeof_or_max(from_type, expression.line);
        if(size == -1) return TYPES_CAPACITY;

        printf("%d<", size);
        emit_context.current_cell_index -= size;
        return to_type;
    }

    if(to_type == u8_type){
        if(from_type == u16_type){
            printf("<");
            emit_context.current_cell_index -= 1;
            return to_type;
        } else if(from_type == u24_type){
            printf("2<");
            emit_context.current_cell_index -= 2;
            return to_type;
        } else if(from_type == u32_type){
            printf("3<");
            emit_context.current_cell_index -= 3;
            return to_type;
        }
    }

    if(to_type == u16_type){
        if(from_type == u24_type){
            printf("<");
            emit_context.current_cell_index -= 1;
            return to_type;
        } else if(from_type == u32_type){
            printf("2<");
            emit_context.current_cell_index -= 2;
            return to_type;
        }
    }

    printf("\nerror on line %d: Cannot cast '", u24_unpack(expression.line));
    type_print(types[from_type]);
    printf("' to '");
    type_print(types[to_type]);
    printf("'\n");
    return TYPES_CAPACITY;
}

static u32 emit_math_u8(ExpressionKind kind, u32 operand_type){
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
        printf("\nerror: Could not perform unknown math operation for expression kind %d\n", (u8) kind);
        return TYPES_CAPACITY;
    }

    return 0;
}

static u32 emit_math_u16(ExpressionKind kind, u32 operand_type){
    switch(kind){
    case EXPRESSION_ADD:
        emit_additive_u16(true);
        return operand_type;
    case EXPRESSION_SUBTRACT:
        emit_additive_u16(false);
        return operand_type;
    case EXPRESSION_MULTIPLY:
        emit_multiply_u16();
        return operand_type;
    case EXPRESSION_DIVIDE:
        emit_divide_u16();
        return operand_type;
    case EXPRESSION_MOD:
        emit_mod_u16();
        return operand_type;
    case EXPRESSION_EQUALS:
        emit_eq_u16();
        return u1_type;
    case EXPRESSION_NOT_EQUALS:
        emit_neq_u16();
        return u1_type;
    case EXPRESSION_LESS_THAN:
        emit_lt_u16();
        return u1_type;
    case EXPRESSION_GREATER_THAN:
        emit_gt_u16();
        return u1_type;
    case EXPRESSION_LESS_THAN_OR_EQUAL:
        emit_lte_u16();
        return u1_type;
    case EXPRESSION_GREATER_THAN_OR_EQUAL:
        emit_gte_u16();
        return u1_type;
    case EXPRESSION_LSHIFT:
        emit_lshift_u16();
        return operand_type;
    case EXPRESSION_RSHIFT:
        emit_rshift_u16();
        return operand_type;
    case EXPRESSION_BIT_AND:
        emit_bit_and_u16();
        return operand_type;
    case EXPRESSION_BIT_OR:
        emit_bit_or_u16();
        return operand_type;
    case EXPRESSION_BIT_XOR:
        emit_bit_xor_u16();
        return operand_type;
    default:
        printf("\nerror: Could not perform unknown math operation for expression kind %d\n", (u8) kind);
        return TYPES_CAPACITY;
    }

    return 0;
}

static u32 emit_math_u32(ExpressionKind kind, u32 operand_type){
    switch(kind){
    case EXPRESSION_ADD:
        emit_additive_u32(true);
        return operand_type;
    case EXPRESSION_SUBTRACT:
        emit_additive_u32(false);
        return operand_type;
    case EXPRESSION_MULTIPLY:
        emit_multiply_u32();
        return operand_type;
    case EXPRESSION_DIVIDE:
        emit_divide_u32();
        return operand_type;
    case EXPRESSION_MOD:
        emit_mod_u32();
        return operand_type;
    case EXPRESSION_EQUALS:
        emit_eq_u32();
        return u1_type;
    case EXPRESSION_NOT_EQUALS:
        emit_neq_u32();
        return u1_type;
    case EXPRESSION_LESS_THAN:
        emit_lt_u32();
        return u1_type;
    case EXPRESSION_GREATER_THAN:
        emit_gt_u32();
        return u1_type;
    case EXPRESSION_LESS_THAN_OR_EQUAL:
        emit_lte_u32();
        return u1_type;
    case EXPRESSION_GREATER_THAN_OR_EQUAL:
        emit_gte_u32();
        return u1_type;
    case EXPRESSION_LSHIFT:
        emit_lshift_u32();
        return operand_type;
    case EXPRESSION_RSHIFT:
        emit_rshift_u32();
        return operand_type;
    case EXPRESSION_BIT_AND:
        emit_bit_and_u32();
        return operand_type;
    case EXPRESSION_BIT_OR:
        emit_bit_or_u32();
        return operand_type;
    case EXPRESSION_BIT_XOR:
        emit_bit_xor_u32();
        return operand_type;
    default:
        printf("\nerror: Could not perform unknown math operation for expression kind %d\n", (u8) kind);
        return TYPES_CAPACITY;
    }

    return 0;
}

u32 expression_emit_unary_operation(Expression expression, u32 type){
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
        } else if(type == u16_type){
            emit_negate_u16();
            return type;
        } else if(type == u32_type){
            emit_negate_u32();
            return type;
        }
        break;
    case EXPRESSION_BIT_COMPLEMENT:
        if(type == u8_type || type == u1_type){
            emit_bit_complement_u8();
            return type;
        } else if(type == u16_type){
            emit_bit_complement_u16();
            return type;
        } else if(type == u32_type){
            emit_bit_complement_u32();
            return type;
        }
        break;
    case EXPRESSION_PRE_INCREMENT:
    case EXPRESSION_PRE_DECREMENT:
        if(type == u8_type){
            emit_u8(1);
            emit_additive_u8(expression.kind == EXPRESSION_PRE_INCREMENT);
            dupe_cell();
            if(write_destination_expression(u8_type, expressions[expression.ops], expression.line) == TYPES_CAPACITY){
                return TYPES_CAPACITY;
            }
            return type;
        } else if(type == u16_type){
            emit_u16(1);
            emit_additive_u16(expression.kind == EXPRESSION_PRE_INCREMENT);
            dupe_cells(2);
            if(write_destination_expression(u16_type, expressions[expression.ops], expression.line) == TYPES_CAPACITY){
                return TYPES_CAPACITY;
            }
            return type;
        } else if(type == u32_type){
            emit_u32(1);
            emit_additive_u32(expression.kind == EXPRESSION_PRE_INCREMENT);
            dupe_cells(4);
            if(write_destination_expression(u32_type, expressions[expression.ops], expression.line) == TYPES_CAPACITY){
                return TYPES_CAPACITY;
            }
            return type;
        }
        break;
    case EXPRESSION_POST_INCREMENT:
    case EXPRESSION_POST_DECREMENT:
        if(type == u8_type){
            dupe_cell();
            emit_u8(1);
            emit_additive_u8(expression.kind == EXPRESSION_POST_INCREMENT);
            if(write_destination_expression(u8_type, expressions[expression.ops], expression.line) == TYPES_CAPACITY){
                return TYPES_CAPACITY;
            }
            return type;
        } else if(type == u16_type){
            dupe_cells(2);
            emit_u16(1);
            emit_additive_u16(expression.kind == EXPRESSION_POST_INCREMENT);
            if(write_destination_expression(u16_type, expressions[expression.ops], expression.line) == TYPES_CAPACITY){
                return TYPES_CAPACITY;
            }
            return type;
        } else if(type == u32_type){
            dupe_cells(4);
            emit_u32(1);
            emit_additive_u32(expression.kind == EXPRESSION_POST_INCREMENT);
            if(write_destination_expression(u32_type, expressions[expression.ops], expression.line) == TYPES_CAPACITY){
                return TYPES_CAPACITY;
            }
            return type;
        }
        break;
    case EXPRESSION_NO_RESULT_INCREMENT:
    case EXPRESSION_NO_RESULT_DECREMENT:
        if(type == u8_type){
            emit_u8(1);
            emit_additive_u8(expression.kind == EXPRESSION_NO_RESULT_INCREMENT);
            if(write_destination_expression(u8_type, expressions[expression.ops], expression.line) == TYPES_CAPACITY){
                return TYPES_CAPACITY;
            }
            return u0_type;
        } else if(type == u16_type){
            emit_u16(1);
            emit_additive_u16(expression.kind == EXPRESSION_NO_RESULT_INCREMENT);
            if(write_destination_expression(u16_type, expressions[expression.ops], expression.line) == TYPES_CAPACITY){
                return TYPES_CAPACITY;
            }
            return u0_type;
        } else if(type == u32_type){
            emit_u32(1);
            emit_additive_u32(expression.kind == EXPRESSION_NO_RESULT_INCREMENT);
            if(write_destination_expression(u32_type, expressions[expression.ops], expression.line) == TYPES_CAPACITY){
                return TYPES_CAPACITY;
            }
            return u0_type;
        }
        break;
    default:
        break;
    }

    u32 enum_typedef = find_enum_from_type(type);

    if(enum_typedef < TYPEDEFS_CAPACITY){
        return expression_emit_unary_operation(expression, u8_type);
    }

    printf("\nerror on line %d: Cannot ", u24_unpack(expression.line));
    expression_print_operation_name(expression.kind);
    printf(" value of type '");
    type_print(types[type]);
    printf("'\n");
    return TYPES_CAPACITY;
}

u32 expression_emit_unary(Expression expression){
    u32 type = expression_emit(expressions[expression.ops]);
    if(type >= TYPES_CAPACITY) return TYPES_CAPACITY;

    return expression_emit_unary_operation(expression, type);
}

static u32 expression_emit_math_operation(Expression expression, u32 a_type){
    if(a_type == u8_type){
        return emit_math_u8(expression.kind, a_type);
    }

    if(a_type == u16_type){
        return emit_math_u16(expression.kind, a_type);
    }

    if(a_type == u32_type){
        return emit_math_u32(expression.kind, a_type);
    }

    // Allow bitwise operations on u1 types
    if(a_type == u1_type && (
        expression.kind == EXPRESSION_BIT_AND
     || expression.kind == EXPRESSION_BIT_OR
     || expression.kind == EXPRESSION_BIT_XOR
    )){
        return emit_math_u8(expression.kind, a_type);
    }

    u32 enum_typedef = find_enum_from_type(a_type);

    if(enum_typedef < TYPEDEFS_CAPACITY){
        return expression_emit_math_operation(expression, u8_type);
    }

    printf("\nerror on line %d: Cannot ", u24_unpack(expression.line));
    expression_print_operation_name(expression.kind);
    printf(" values of type '");
    type_print(types[a_type]);
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
        printf("\nerror on line %d: Cannot ", u24_unpack(expression.line));
        expression_print_operation_name(expression.kind);
        printf(" incompatible types '");
        type_print(types[a_type]);
        printf("' and '");
        type_print(types[b_type]);
        printf("'\n");
        return TYPES_CAPACITY;
    }

    return expression_emit_math_operation(expression, a_type);
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

u32 read_destination(Destination destination, u24 line_on_error){
    if(destination.on_stack){
        printf("\nerror on line %d: Cannot index into destination on the stack yet (not supported)\n", u24_unpack(line_on_error));
        return TYPES_CAPACITY;
    }

    u32 item_type_size = type_sizeof_or_max(destination.type, line_on_error);
    if(item_type_size == -1) return TYPES_CAPACITY;

    if(destination.offset_size == 0){
        copy_cells_static(destination.tape_location, item_type_size);
        return destination.type;
    } else if(destination.offset_size == 1){
        // u8 indexing
        copy_cells_dynamic_u8(destination.tape_location, item_type_size);
        return destination.type;
    } else if(destination.offset_size == 2){
        // u16 indexing
        copy_cells_dynamic_u16(destination.tape_location, item_type_size);
        return destination.type;
    } else if(destination.offset_size == 3){
        // u24 indexing
        copy_cells_dynamic_u24(destination.tape_location, item_type_size);
        return destination.type;
    } else if(destination.offset_size == 4){
        // u32 indexing
        copy_cells_dynamic_u32(destination.tape_location, item_type_size);
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

ErrorCode print_array_reference(Destination destination, u32 max_length, u24 line_on_error){
    if(destination.offset_size == 0){
        print_cells_static(destination.tape_location, max_length);
    } else {
        // TODO: Optimize this
        // Ignore reference and just copy and print the value

        if(read_destination(destination, line_on_error) == TYPES_CAPACITY){
            return 1;
        }

        emit_print_array_value(max_length);
    }

    return 0;
}

static u32 expression_emit_ternary_stack(Expression expression){
    u32 condition = operands[expression.ops];
    u32 when_true = operands[expression.ops + 1];
    u32 when_false = operands[expression.ops + 2];

    // Evaluate condition
    u32 condition_type = expression_emit(expressions[condition]);
    if(condition_type >= TYPES_CAPACITY) return TYPES_CAPACITY;

    if(condition_type != u1_type){
        printf("\nerror on line %d: Expected ternary condition to be 'u1', got '", u24_unpack(expression.line));
        type_print(types[condition_type]);
        printf("'\n");
        return TYPES_CAPACITY;
    }

    u32 then_block = emit_settings.next_basicblock_id++;
    u32 else_block = emit_settings.next_basicblock_id++;
    u32 continuation_block = emit_settings.next_basicblock_id++;

    u32 pushed = emit_end_basicblock_jump_conditional(then_block, else_block);

    // Then block
    emit_start_basicblock_landing(then_block, pushed);

    u32 when_true_type = expression_emit(expressions[when_true]);
    if(when_true_type >= TYPES_CAPACITY) return TYPES_CAPACITY;

    u32 result_size = type_sizeof_or_max(when_true_type, expression.line);
    if(result_size == -1) return TYPES_CAPACITY;

    if(emit_settings.in_basicblock){
        emit_end_basicblock_jump_compatible(continuation_block, pushed + result_size);
    }

    // Else block
    emit_start_basicblock_landing(else_block, pushed);

    u32 when_false_type = expression_emit(expressions[when_false]);
    if(when_false_type >= TYPES_CAPACITY) return TYPES_CAPACITY;

    if(when_false_type != when_true_type){
        printf("\nerror on line %d: Expected false branch of ternary expression to be '", u24_unpack(expression.line));
        type_print(types[when_true_type]);
        printf("', got '");
        type_print(types[when_false_type]);
        printf("'\n");
        return TYPES_CAPACITY;
    }

    if(emit_settings.in_basicblock){
        emit_end_basicblock_jump_compatible(continuation_block, pushed + result_size);
    }

    // Continuation block
    emit_start_basicblock_landing(continuation_block, pushed + result_size);
    return when_true_type;
}

static u32 expression_emit_ternary_tape(Expression expression){
    // result should_do_else condition

    u32 condition = operands[expression.ops];
    u32 when_true = operands[expression.ops + 1];
    u32 when_false = operands[expression.ops + 2];

    u32 result_type = expression_get_type(expressions[when_true], EXPRESSION_GET_TYPE_MODE_PRINT_ERROR);
    u32 result_size;

    if(result_type < TYPES_CAPACITY){
        result_size = type_sizeof_or_max(result_type, expression.line);
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
    if(when_true_type >= TYPES_CAPACITY) return TYPES_CAPACITY;

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
    if(when_false_type >= TYPES_CAPACITY) return TYPES_CAPACITY;

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

static u32 expression_emit_ternary(Expression expression){
    if(emit_context.in_recursive_function){
        return expression_emit_ternary_stack(expression);
    } else {
        return expression_emit_ternary_tape(expression);
    }
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
    u32 value_type;

    if(expression.ops == EXPRESSIONS_CAPACITY){
        // `return;`
        value_type = u0_type;
    } else {
        // `return value;`
        value_type = expression_emit(expressions[expression.ops]);
        if(value_type >= TYPES_CAPACITY) return TYPES_CAPACITY;
    }

    if(value_type != return_type){
        if(grow_type(value_type, return_type, 0, expression.line) != 0){
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

    u32 return_type_size = type_sizeof_or_max(return_type, expression.line);
    if(return_type_size == -1) return TYPES_CAPACITY;

    if(emit_context.in_recursive_function){
        emit_stack_pointer();
        emit_u32(4 + return_type_size);
        emit_additive_u32(false);

        // Move data value into return value location
        move_cells_dynamic_u32(emit_settings.stack_begin, return_type_size);

        // Remove working memory
        printf("%d<", emit_context.current_cell_index - emit_settings.stack_driver_position);
        emit_context.current_cell_index = emit_settings.stack_driver_position;

        // End basicblock
        emit_end_basicblock();
    } else {
        u32 return_value_location = emit_context.function_cell_index - return_type_size;

        // Move data value into return value location
        move_cells_static(return_value_location, return_type_size);

        // Unmark 'incomplete' cell if function can early return
        if(emit_context.can_function_early_return){
            u32 offset = emit_context.current_cell_index - emit_context.incomplete_cell;
            printf("%d<[-]%d>", offset, offset);
        }
    }

    return u0_type;
}

static u32 expression_emit_break(Expression expression){
    if(!emit_context.can_break){
        printf("\nerror on line %d: Cannot break, nowhere to go\n", u24_unpack(expression.line));
        return TYPES_CAPACITY;
    }

    if(emit_context.in_recursive_function){
        emit_end_basicblock_jump_to(emit_settings.break_basicblock_context);
    } else {
        // Zero 'didnt_break_cell'
        u32 offset = emit_context.current_cell_index - emit_context.didnt_break_cell;
        printf("%d<[-]%d>", offset, offset);
    }

    return u0_type;
}

static u32 expression_emit_continue(Expression expression){
    if(!emit_context.can_continue){
        printf("\nerror on line %d: Cannot continue, nowhere to go\n", u24_unpack(expression.line));
        return TYPES_CAPACITY;
    }

    if(emit_context.in_recursive_function){
        emit_end_basicblock_jump_to(emit_settings.continue_basicblock_context);
    } else {
        // Zero 'didnt_continue_cell'
        u32 offset = emit_context.current_cell_index - emit_context.didnt_continue_cell;
        printf("%d<[-]%d>", offset, offset);
    }

    return u0_type;
}

static u32 emit_sizeof_size(u32 type_index, u8 integer_cells, u24 line_on_error){
    u32 size = type_sizeof_or_max(type_index, line_on_error);
    if(size == -1) return TYPES_CAPACITY;

    u32 max_values[5] = {
        (u32) 0,
        (u32) 255,
        (u32) 65535,
        (u32) 16777215,
        (u32) 4294967295,
    };

    if(!in_range_inclusive(integer_cells, 1, 4) || size > max_values[integer_cells]){
        printf("\nerror on line %d: Cannot emit size of '", u24_unpack(line_on_error));
        type_print(types[type_index]);
        printf("' as 'u%d' (maximum size exceeded)\n", integer_cells * 8);
        return TYPES_CAPACITY;
    }

    switch(integer_cells){
    case 1:
        emit_u8((u8) size);
        return u8_type;
    case 2:
        emit_u16((u16) size);
        return u16_type;
    case 3:
        emit_u24(u24_pack(size));
        return u24_type;
    case 4:
        emit_u32(size);
        return u32_type;
    }

    printf("\nerror on line %d: Cannot emit size of '", u24_unpack(line_on_error));
    type_print(types[type_index]);
    printf("' as 'u%d' (maximum size exceeded)\n", integer_cells * 8);
    return TYPES_CAPACITY;
}

static u32 expression_emit_sizeof_type(Expression expression, u8 integer_cells){
    return emit_sizeof_size(expression.ops, integer_cells, expression.line);
}

static u32 expression_emit_sizeof_value(Expression expression, u8 integer_cells){
    u32 expression_type = expression_get_type(expressions[expression.ops], EXPRESSION_GET_TYPE_MODE_PRINT_ERROR);

    if(expression_type >= TYPES_CAPACITY){
        printf("\nerror on line %d: Could not determine size of expression, perhaps you have undeclared variables?", u24_unpack(expression.line));
        return TYPES_CAPACITY;
    }

    return emit_sizeof_size(expression_type, integer_cells, expression.line);
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

    u32 type_size = type_sizeof_or_max(type, expression.line);
    if(type_size == -1) return TYPES_CAPACITY;

    // Allocate space for result
    for(u32 i = 0; i < type_size; i++){
        printf("[-]>");
        emit_context.current_cell_index++;
    }

    // Set fields
    u32 length = operands[expression.ops + 1];
    u32 out_struct_location = emit_context.current_cell_index - type_size;

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
            .tape_location = out_struct_location,
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
    case EXPRESSION_NONE:
    case EXPRESSION_IF:
    case EXPRESSION_IF_ELSE:
    case EXPRESSION_WHILE:
    case EXPRESSION_DO_WHILE:
    case EXPRESSION_CONDITIONLESS_BLOCK:
    case EXPRESSION_FOR:
    case EXPRESSION_SWITCH:
        break;
    case EXPRESSION_DECLARE: {
            u32 variable_size = type_sizeof_or_max(operands[expression.ops], expression.line);
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

                if(get_item_type(types[destination.type], expression.line, false) != u8_type){
                    printf("\nerror on line %d: Can only print 'u8[]' but got '", u24_unpack(expression.line));
                    type_print(types[destination.type]);
                    printf("'\n");
                    return TYPES_CAPACITY;
                }

                u32 max_length = dimensions[types[destination.type].dimensions][0];
                if(print_array_reference(destination, max_length, expression.line) != 0){
                    return TYPES_CAPACITY;
                }
            } else {
                // Print value
                u32 array_type = expression_emit(expressions[expression.ops]);
                if(array_type >= TYPES_CAPACITY) return TYPES_CAPACITY;

                if(get_item_type(types[array_type], expression.line, false) != u8_type){
                    printf("\nerror on line %d: Can only print 'u8[]' but got '", u24_unpack(expression.line));
                    type_print(types[array_type]);
                    printf("'\n");
                    return TYPES_CAPACITY;
                }

                u32 max_length = dimensions[types[array_type].dimensions][0];
                emit_print_array_value(max_length);
            }
        }
        return u0_type;
    case EXPRESSION_PRINTF:
        return expression_emit_printf(expression);
    case EXPRESSION_MEMCMP:
        return expression_emit_memcmp(expression);
    case EXPRESSION_MEMCPY:
        return expression_emit_memcpy(expression);
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
    case EXPRESSION_IMPLEMENT_READU8:
        printf("<[-]>[-]+[[-]>[-],[+[11-[>[-]6+[<6->-]<--<<[->>10+<<]>>[-<<+>>]<+>]]]<]");
        return u0_type;
    case EXPRESSION_U1:
        emit_u1((u1) expression.ops);
        return u1_type;
    case EXPRESSION_INT:
        if(expression.ops >= 256){
            printf("\nerror on line %d: Cannot fit integer literal value into assumed type 'u8', must be explicitly casted to correct type\n", u24_unpack(expression.line));
            return TYPES_CAPACITY;
        }
    case EXPRESSION_U8:
        emit_u8((u8) expression.ops);
        return u8_type;
    case EXPRESSION_U16:
        emit_u16((u16) expression.ops);
        return u16_type;
    case EXPRESSION_U24:
        emit_u24(u24_pack(expression.ops));
        return u24_type;
    case EXPRESSION_U32:
        emit_u32(expression.ops);
        return u32_type;
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
        return expression_emit_sizeof_type(expression, 1);
    case EXPRESSION_SIZEOF_TYPE_U16:
        return expression_emit_sizeof_type(expression, 2);
    case EXPRESSION_SIZEOF_TYPE_U24:
        return expression_emit_sizeof_type(expression, 3);
    case EXPRESSION_SIZEOF_TYPE_U32:
        return expression_emit_sizeof_type(expression, 4);
    case EXPRESSION_SIZEOF_VALUE:
    case EXPRESSION_SIZEOF_VALUE_U8:
        return expression_emit_sizeof_value(expression, 1);
    case EXPRESSION_SIZEOF_VALUE_U16:
        return expression_emit_sizeof_value(expression, 2);
    case EXPRESSION_SIZEOF_VALUE_U24:
        return expression_emit_sizeof_value(expression, 3);
    case EXPRESSION_SIZEOF_VALUE_U32:
        return expression_emit_sizeof_value(expression, 4);
    case EXPRESSION_ARRAY_INITIALIZER:
        return expression_emit_array_initializer(expression);
    case EXPRESSION_STRUCT_INITIALIZER:
        return expression_emit_struct_initializer(expression);
    case EXPRESSION_CASE:
    case EXPRESSION_FIELD_INITIALIZER:
    case EXPRESSION_ENUM_VARIANT:
        printf("\nerror: Invalid expression kind %d during expression_emit\n", (u8) expression.kind);
        return TYPES_CAPACITY;
    case EXPRESSION_PANICLOOP:
        printf("[-]+[]");
        return u0_type;
    }

    printf("\nerror: Unknown expression kind %d during expression_emit\n", (u8) expression.kind);
    return TYPES_CAPACITY;
}

