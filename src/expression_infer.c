
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
    default:
        return EXPRESSION_NONE;
    }
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
    }
}
