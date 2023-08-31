
#include <stdio.h>
#include "../include/expression_infer.h"
#include "../include/expression.h"
#include "../include/type.h"
#include "../include/builtin_types.h"
#include "../include/builtin_types.h"

ExpressionKind expression_get_preferred_int_kind_or_none(u32 expression_index){
    Expression expression = expressions[expression_index];

    switch(expression.kind){
    case EXPRESSION_U8:
        return expression.kind;
    case EXPRESSION_CALL: {
            u32 name = operands[expression.ops];
            u32 return_type = find_function_return_type(name);

            if(return_type == u8_type){
                return EXPRESSION_U8;
            }
        }
        return EXPRESSION_NONE;
    case EXPRESSION_CAST:
        if(operands[expression.ops] == u1_type){
            return EXPRESSION_U1;
        } else if(operands[expression.ops] == u8_type){
            return EXPRESSION_U8;
        }
        break;
    case EXPRESSION_AND:
    case EXPRESSION_OR:
        return EXPRESSION_U1;
    case EXPRESSION_ASSIGN:
    case EXPRESSION_ADD:
    case EXPRESSION_SUBTRACT:
    case EXPRESSION_MULTIPLY:
    case EXPRESSION_DIVIDE:
    case EXPRESSION_MOD:
    case EXPRESSION_EQUALS:
    case EXPRESSION_NOT_EQUALS:
    case EXPRESSION_LESS_THAN:
    case EXPRESSION_GREATER_THAN:
    case EXPRESSION_LSHIFT:
    case EXPRESSION_RSHIFT:
    case EXPRESSION_BIT_AND:
    case EXPRESSION_BIT_OR:
    case EXPRESSION_BIT_XOR: {
            ExpressionKind result;

            result = expression_get_preferred_int_kind_or_none(operands[expression.ops]);
            if(result != EXPRESSION_NONE) return result;

            result = expression_get_preferred_int_kind_or_none(operands[expression.ops + 1]);
            if(result != EXPRESSION_NONE) return result;
        }
        break;
    case EXPRESSION_NOT:
        return EXPRESSION_U1;
    case EXPRESSION_NEGATE:
    case EXPRESSION_BIT_COMPLEMENT:
    case EXPRESSION_PRE_INCREMENT:
    case EXPRESSION_PRE_DECREMENT:
    case EXPRESSION_POST_INCREMENT:
    case EXPRESSION_POST_DECREMENT:
        return expression_get_preferred_int_kind_or_none(expression.ops);
    }
    
    return EXPRESSION_NONE;
}

static ExpressionKind function_argument_preferred_int_kind(u32 function_begin, u32 argument_i){
    Expression declare_statement = expressions[statements[function_begin + argument_i]];
    u32 argument_type = operands[declare_statement.ops];

    if(argument_type == u8_type){
        return EXPRESSION_U8;
    }

    return EXPRESSION_NONE;
}

static u0 expression_infer_call(Expression expression){
    u32 name = operands[expression.ops];
    u32 function_index = find_function(name);

    if(function_index >= FUNCTIONS_CAPACITY){
        return;
    }
    
    u32 arity = operands[expression.ops + 1];

    for(u32 i = 0; i < arity; i++){
        u32 argument = operands[expression.ops + 2 + i];
        expression_infer(argument, function_argument_preferred_int_kind(functions[function_index].begin, i));
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
    case EXPRESSION_INT:
        if(preferred_int_kind != EXPRESSION_NONE){
            expressions[expression_index].kind = preferred_int_kind;
        }
        break;
    case EXPRESSION_CALL:
        expression_infer_call(expression);
        break;
    case EXPRESSION_CAST:
        expression_infer(operands[expression.ops + 1], EXPRESSION_NONE);
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
        expression_infer_math(expression, preferred_int_kind);
        break;
    case EXPRESSION_EQUALS:
    case EXPRESSION_NOT_EQUALS:
    case EXPRESSION_LESS_THAN:
    case EXPRESSION_GREATER_THAN:
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
        expression_infer(operands[expression.ops + 1], EXPRESSION_U1);
        break;
    case EXPRESSION_MEMBER:
        expression_infer(operands[expression.ops], EXPRESSION_NONE);
        break;
    }
}

