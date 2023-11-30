
#include <stdio.h>
#include "../include/expression_infer.h"
#include "../include/expression.h"
#include "../include/type.h"
#include "../include/builtin_types.h"
#include "../include/builtin_types.h"
#include "../include/variable_find.h"
#include "../include/emit_context.h"
#include "../include/type_print.h"
#include "../include/expression_get_type.h"

ExpressionKind type_to_expression_kind(u32 type){
    if(type == u8_type){
        return EXPRESSION_U8;
    } else if(type == u16_type){
        return EXPRESSION_U16;
    } else if(type == u24_type){
        return EXPRESSION_U24;
    } else if(type == u32_type){
        return EXPRESSION_U32;
    }

    return EXPRESSION_NONE;
}


ExpressionKind expression_get_preferred_int_kind_or_none(u32 expression_index){
    Expression expression = expressions[expression_index];

    u32 type = expression_get_type(expression, EXPRESSION_GET_TYPE_MODE_INFER);

    if(type < TYPES_CAPACITY){
        return type_to_expression_kind(type);
    }
    
    return EXPRESSION_NONE;
}

static ExpressionKind function_argument_preferred_int_kind(u32 function_begin, u32 argument_i){
    Expression declare_statement = expressions[statements[function_begin + argument_i]];
    u32 argument_type = operands[declare_statement.ops];
    return type_to_expression_kind(argument_type);
}

static u0 expression_infer_call(Expression expression){
    u32 name = operands[expression.ops];
    u32 function_index = find_function(name);
    u1 has_function = function_index < FUNCTIONS_CAPACITY;

    u32 arity = operands[expression.ops + 1];

    for(u32 i = 0; i < arity; i++){
        u32 argument = operands[expression.ops + 2 + i];

        ExpressionKind preferred = has_function
            ? function_argument_preferred_int_kind(functions[function_index].begin, i)
            : EXPRESSION_NONE;

        expression_infer(argument, preferred);
    }
}

static u0 expression_infer_math(Expression expression, ExpressionKind preferred_int_kind){
    u32 a = operands[expression.ops];
    u32 b = operands[expression.ops + 1];

    if(preferred_int_kind == EXPRESSION_NONE){
        preferred_int_kind = expression_get_preferred_int_kind_or_none(a);
    }

    if(preferred_int_kind == EXPRESSION_NONE){
        preferred_int_kind = expression_get_preferred_int_kind_or_none(b);
    }

    expression_infer(a, preferred_int_kind);
    expression_infer(b, preferred_int_kind);
}

u0 expression_infer(u32 expression_index, ExpressionKind preferred_int_kind){
    Expression expression = expressions[expression_index];

    switch(expression.kind){
    case EXPRESSION_NONE:
    case EXPRESSION_DECLARE:
    case EXPRESSION_PRINT_LITERAL:
    case EXPRESSION_IMPLEMENT_PUT:
    case EXPRESSION_IMPLEMENT_PRINTU1:
    case EXPRESSION_IMPLEMENT_PRINTU8:
    case EXPRESSION_IMPLEMENT_GET:
    case EXPRESSION_IMPLEMENT_READU8:
    case EXPRESSION_U1:
    case EXPRESSION_U8:
    case EXPRESSION_U16:
    case EXPRESSION_U24:
    case EXPRESSION_U32:
    case EXPRESSION_VARIABLE:
    case EXPRESSION_STRING:
    case EXPRESSION_BREAK:
    case EXPRESSION_CONTINUE:
    case EXPRESSION_SIZEOF_TYPE_U8:
    case EXPRESSION_SIZEOF_TYPE_U16:
    case EXPRESSION_SIZEOF_TYPE_U24:
    case EXPRESSION_SIZEOF_TYPE_U32:
    case EXPRESSION_STRUCT_INITIALIZER:
    case EXPRESSION_FIELD_INITIALIZER:
    case EXPRESSION_ENUM_VARIANT:
    case EXPRESSION_PANICLOOP:
        break;

    case EXPRESSION_RETURN:
        if(expression.ops != EXPRESSIONS_CAPACITY){
            expression_infer(expression.ops, type_to_expression_kind(functions[emit_context.function].return_type));
        }
        break;
    case EXPRESSION_ASSIGN:
        if(preferred_int_kind == EXPRESSION_NONE){
            preferred_int_kind = expression_get_preferred_int_kind_or_none(operands[expression.ops]);
        }
        expression_infer(operands[expression.ops], preferred_int_kind);
        expression_infer(operands[expression.ops + 1], preferred_int_kind);
        break;
    case EXPRESSION_INDEX:
        expression_infer(operands[expression.ops], EXPRESSION_NONE);
        expression_infer(operands[expression.ops + 1], EXPRESSION_NONE);
        break;
    case EXPRESSION_PRINT_ARRAY:
        expression_infer(expression.ops, EXPRESSION_NONE);
        break;
    case EXPRESSION_INT:
        if(preferred_int_kind != EXPRESSION_NONE){
            expressions[expression_index].kind = preferred_int_kind;
        }
        break;
    case EXPRESSION_CALL:
    case EXPRESSION_PRINTF:
    case EXPRESSION_MEMCMP:
        expression_infer_call(expression);
        break;
    case EXPRESSION_CAST: {
            u32 value = operands[expression.ops + 1];

            if(expressions[value].kind == EXPRESSION_INT){
                expression_infer(value, type_to_expression_kind(operands[expression.ops]));
            } else {
                expression_infer(value, EXPRESSION_NONE);
            }
        }
        break;
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
        expression_infer_math(expression, preferred_int_kind);
        break;
    case EXPRESSION_EQUALS:
    case EXPRESSION_NOT_EQUALS:
    case EXPRESSION_LESS_THAN:
    case EXPRESSION_GREATER_THAN:
    case EXPRESSION_LESS_THAN_OR_EQUAL:
    case EXPRESSION_GREATER_THAN_OR_EQUAL:
        expression_infer_math(expression, EXPRESSION_NONE);
        break;
    case EXPRESSION_AND:
    case EXPRESSION_OR:
        expression_infer_math(expression, EXPRESSION_U1);
        break;
    case EXPRESSION_NOT:
        expression_infer(expression.ops, EXPRESSION_U1);
        break;
    case EXPRESSION_NEGATE:
    case EXPRESSION_BIT_COMPLEMENT:
    case EXPRESSION_PRE_INCREMENT:
    case EXPRESSION_PRE_DECREMENT:
    case EXPRESSION_POST_INCREMENT:
    case EXPRESSION_POST_DECREMENT:
    case EXPRESSION_NO_RESULT_INCREMENT:
    case EXPRESSION_NO_RESULT_DECREMENT:
        expression_infer(expression.ops, preferred_int_kind);
        break;
    case EXPRESSION_TERNARY: {
            expression_infer(operands[expression.ops], EXPRESSION_U1);
            u32 result_preferred_kind = expression_get_preferred_int_kind_or_none(operands[expression.ops + 1]);
            expression_infer(operands[expression.ops + 1], result_preferred_kind);
            expression_infer(operands[expression.ops + 2], result_preferred_kind);
        }
        break;
    case EXPRESSION_IF:
    case EXPRESSION_IF_ELSE:
    case EXPRESSION_WHILE:
    case EXPRESSION_DO_WHILE:
        expression_infer(operands[expression.ops], EXPRESSION_U1);
        break;
    case EXPRESSION_FOR:
        // Go to context past pre-statements
        emit_context.current_statement += operands[expression.ops];

        expression_infer(operands[expression.ops + 1], EXPRESSION_U1);

        // Return context to before pre-statements
        emit_context.current_statement -= operands[expression.ops];
    case EXPRESSION_SWITCH:
    case EXPRESSION_CASE:
    case EXPRESSION_MEMBER:
        expression_infer(operands[expression.ops], EXPRESSION_NONE);
        break;
    case EXPRESSION_SIZEOF_TYPE:
        switch(preferred_int_kind){
        case EXPRESSION_U8:
            expressions[expression_index].kind = EXPRESSION_SIZEOF_TYPE_U8;
            break;
        case EXPRESSION_U16:
            expressions[expression_index].kind = EXPRESSION_SIZEOF_TYPE_U16;
            break;
        case EXPRESSION_U24:
            expressions[expression_index].kind = EXPRESSION_SIZEOF_TYPE_U24;
            break;
        case EXPRESSION_U32:
            expressions[expression_index].kind = EXPRESSION_SIZEOF_TYPE_U32;
            break;
        default:
            break;
        }
        break;
    case EXPRESSION_SIZEOF_VALUE:
        switch(preferred_int_kind){
        case EXPRESSION_U8:
            expressions[expression_index].kind = EXPRESSION_SIZEOF_VALUE_U8;
            break;
        case EXPRESSION_U16:
            expressions[expression_index].kind = EXPRESSION_SIZEOF_VALUE_U16;
            break;
        case EXPRESSION_U24:
            expressions[expression_index].kind = EXPRESSION_SIZEOF_VALUE_U24;
            break;
        case EXPRESSION_U32:
            expressions[expression_index].kind = EXPRESSION_SIZEOF_VALUE_U32;
            break;
        default:
            break;
        }
        /* fallthrough */
    case EXPRESSION_SIZEOF_VALUE_U8:
    case EXPRESSION_SIZEOF_VALUE_U16:
    case EXPRESSION_SIZEOF_VALUE_U24:
    case EXPRESSION_SIZEOF_VALUE_U32:
        expression_infer(expression.ops, preferred_int_kind);
        break;
    case EXPRESSION_ARRAY_INITIALIZER: {
            u32 length = operands[expression.ops];
            for(u32 i = 0; i < length; i++){
                expression_infer(operands[expression.ops + 1 + i], EXPRESSION_NONE);
            }
        }
        break;
    }
}

