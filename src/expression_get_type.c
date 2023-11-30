
#include <stdio.h>
#include <string.h>
#include "../include/storage.h"
#include "../include/expression.h"
#include "../include/builtin_types.h"
#include "../include/variable_find.h"
#include "../include/expression_get_type.h"
#include "../include/type_print.h"
#include "../include/expression_print.h"

u32 expression_get_type_for_call(Expression expression){
    u32 name = operands[expression.ops];
    int function_index = find_function(name);

    if(function_index >= FUNCTIONS_CAPACITY){
        return TYPES_CAPACITY;
    }

    return functions[function_index].return_type;
}

u32 expression_get_type_for_string(Expression expression){
    u32 str = expression.ops;
    u32 len = aux_cstr_len(str);

    u32 dims[4];
    dims[0] = len;
    dims[1] = 0;
    dims[2] = 0;
    dims[3] = 0;

    u32 type_dimensions = add_dimensions(dims);
    if(type_dimensions >= UNIQUE_DIMENSIONS_CAPACITY) return TYPES_CAPACITY;

    return add_type((Type){
        .name = types[u8_type].name,
        .dimensions = type_dimensions,
    });
}

u32 expression_get_type_for_variable(Expression expression, ExpressionGetTypeMode mode){
    u32 name = expression.ops;
    Variable variable = variable_find(name);

    if(variable.defined){
        return variable.type;
    }

    if(mode & EXPRESSION_GET_TYPE_MODE_PRINT_ERROR){
        printf("\nerror on line %d: Variable '", u24_unpack(expression.line));
        print_aux_cstr(name);
        printf("' is not defined\n");
    }
    return TYPES_CAPACITY;
}

u32 expression_get_type(Expression expression, ExpressionGetTypeMode mode){
    switch(expression.kind){
    case EXPRESSION_NONE:
    case EXPRESSION_RETURN:
    case EXPRESSION_NO_RESULT_INCREMENT:
    case EXPRESSION_NO_RESULT_DECREMENT:
    case EXPRESSION_IF:
    case EXPRESSION_IF_ELSE:
    case EXPRESSION_WHILE:
    case EXPRESSION_DO_WHILE:
    case EXPRESSION_BREAK:
    case EXPRESSION_CONTINUE:
    case EXPRESSION_FOR:
    case EXPRESSION_CONDITIONLESS_BLOCK:
    case EXPRESSION_SWITCH:
    case EXPRESSION_CASE:
    case EXPRESSION_ARRAY_INITIALIZER:
    case EXPRESSION_FIELD_INITIALIZER:
    case EXPRESSION_ENUM_VARIANT:
    case EXPRESSION_PANICLOOP:
        break;

    case EXPRESSION_STRUCT_INITIALIZER:
        return operands[expression.ops];
    case EXPRESSION_DECLARE:
    case EXPRESSION_PRINT_LITERAL:
    case EXPRESSION_PRINT_ARRAY:
    case EXPRESSION_PRINTF:
    case EXPRESSION_MEMCPY:
    case EXPRESSION_IMPLEMENT_PUT:
    case EXPRESSION_IMPLEMENT_PRINTU1:
    case EXPRESSION_IMPLEMENT_PRINTU8:
        return u0_type;
    case EXPRESSION_IMPLEMENT_GET:
    case EXPRESSION_IMPLEMENT_READU8:
    case EXPRESSION_MEMCMP:
        return u8_type;
    case EXPRESSION_CALL:
        return expression_get_type_for_call(expression);
    case EXPRESSION_U1:
        return u1_type;
    case EXPRESSION_INT:
        if(mode == EXPRESSION_GET_TYPE_MODE_INFER){
            return TYPES_CAPACITY;
        }
        /* fallthrough */
    case EXPRESSION_U8:
        return u8_type;
    case EXPRESSION_U16:
        return u16_type;
    case EXPRESSION_U24:
        return u24_type;
    case EXPRESSION_U32:
        return u32_type;
    case EXPRESSION_ASSIGN:
        return u0_type;
    case EXPRESSION_VARIABLE:
        return expression_get_type_for_variable(expression, mode);
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
    case EXPRESSION_BIT_XOR: {
            u32 type = expression_get_type(expressions[operands[expression.ops]], mode);
            if(type < TYPES_CAPACITY || !(mode & EXPRESSION_GET_TYPE_MODE_INFER)){
                return type;
            }

            return expression_get_type(expressions[operands[expression.ops + 1]], mode);
        }
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
            u32 array_type = expression_get_type(expressions[operands[expression.ops]], mode);
            if(array_type >= TYPES_CAPACITY) return TYPES_CAPACITY;
            return get_item_type(types[array_type], expression.line, false);
        }
    case EXPRESSION_MEMBER: {
            u32 subject_type_index = expression_get_type(expressions[operands[expression.ops]], mode);
            if(subject_type_index >= TYPES_CAPACITY) return TYPES_CAPACITY;

            Type subject_type = types[subject_type_index];
            if(subject_type.dimensions != 0) return TYPES_CAPACITY;

            u32 field_name = operands[expression.ops + 1];

            u32 def_index = find_typedef(subject_type.name);
            if(def_index >= TYPEDEFS_CAPACITY){
                if(subject_type_index == u16_type && aux[field_name] == '_' && in_range_inclusive(aux[field_name + 1], '0', '1') && aux[field_name + 2] == '\0'){
                    return u8_type;
                } else if(subject_type_index == u32_type && aux[field_name] == '_' && in_range_inclusive(aux[field_name + 1], '0', '3') && aux[field_name + 2] == '\0'){
                    return u8_type;
                } else if(subject_type_index == u32_type && aux[field_name] == '_' && in_range_inclusive(aux[field_name + 1], '0', '1') && aux[field_name + 2] == 'u' && aux[field_name + 3] == '1' && aux[field_name + 4] == '6' && aux[field_name + 5] == '\0'){
                    return u16_type;
                }

                return TYPES_CAPACITY;
            }

            TypeDef def = typedefs[def_index];
            if(def.kind != TYPEDEF_STRUCT) return TYPES_CAPACITY;

            for(u32 i = 0; i < def.num_fields; i++){
                Expression field = expressions[statements[def.begin + i]];
                if(field.kind != EXPRESSION_DECLARE) continue;

                u32 other_field_name = operands[field.ops + 1];

                if(aux_cstr_equals(field_name, other_field_name)){
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
        return expression_get_type(expressions[expression.ops], mode);
    case EXPRESSION_TERNARY:
        return expression_get_type(expressions[operands[expression.ops + 1]], mode);
    case EXPRESSION_STRING:
        return expression_get_type_for_string(expression);
    case EXPRESSION_SIZEOF_TYPE:
        if(mode & EXPRESSION_GET_TYPE_MODE_INFER){
            return TYPES_CAPACITY;
        } else {
            fprintf(stderr, "warning on line %d: assuming result type of sizeof_type to be u8\n", u24_unpack(expression.line));
        }
        /* fallthrough */
    case EXPRESSION_SIZEOF_TYPE_U8:
        return u8_type;
    case EXPRESSION_SIZEOF_TYPE_U16:
        return u16_type;
    case EXPRESSION_SIZEOF_TYPE_U24:
        return u24_type;
    case EXPRESSION_SIZEOF_TYPE_U32:
        return u32_type;
    case EXPRESSION_SIZEOF_VALUE:
        if(mode & EXPRESSION_GET_TYPE_MODE_INFER){
            return TYPES_CAPACITY;
        } else {
            fprintf(stderr, "warning on line %d: assuming result type of sizeof_value to be u8\n", u24_unpack(expression.line));
        }
        /* fallthrough */
    case EXPRESSION_SIZEOF_VALUE_U8:
        return u8_type;
    case EXPRESSION_SIZEOF_VALUE_U16:
        return u16_type;
    case EXPRESSION_SIZEOF_VALUE_U24:
        return u24_type;
    case EXPRESSION_SIZEOF_VALUE_U32:
        return u32_type;
    }

    printf("\nerror on line %d: Cannot get type of this expression\n", u24_unpack(expression.line));
    return TYPES_CAPACITY;
}

u32 get_item_type(Type type, u24 line_on_error, u1 show_error_message){
    u32 dims[4];
    memcpy(dims, &dimensions[type.dimensions], sizeof(u32) * 4);

    if(dims[0] == 0){
        if(show_error_message){
            printf("\nerror on line %d: Cannot index into non-array type '", u24_unpack(line_on_error));
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

