
#include <stdio.h>
#include "../include/storage.h"
#include "../include/parse_macro.h"
#include "../include/parse_context.h"
#include "../include/parse_expression.h"

ErrorCode parse_macro(){
    // # 
    //   ^

    u24 line_number = tokens[parse_i - 1].line;

    if(!is_token(TOKEN_WORD)){
        printf("\nerror on line %d: Expected macro name after '#'\n", u24_unpack(line_number));
        return 1;
    }

    u32 macro_name = eat_word();

    if(aux_cstr_equals_define(macro_name)){
        if(!is_token(TOKEN_WORD)){
            printf("\nerror on line %d: Expected definition name after '#define'\n", u24_unpack(line_number));
            return 1;
        }

        u32 name = eat_word();

        Expression expression = parse_expression();
        if(had_parse_error) return 1;

        Expression reduced_expression = perform_compile_time_math(expression);
        if(had_parse_error) return 1;

        u32 reduced = add_expression(reduced_expression);
        if(reduced >= EXPRESSIONS_CAPACITY) return 1;

        u32 define = add_define((Define){
            .name = name,
            .expression = reduced,
        });

        if(define >= DEFINES_CAPACITY){
            return 1;
        }
    } else {
        printf("\nerror on line %d: Unrecognized macro '#", u24_unpack(line_number));
        print_aux_cstr(macro_name);
        printf("'\n");
        return 1;
    }

    return 0;
}

static Expression perform_compile_time_add(Expression expression){
    Expression a = perform_compile_time_math(expressions[operands[expression.ops]]);
    Expression b = perform_compile_time_math(expressions[operands[expression.ops + 1]]);

    if(a.kind != b.kind){
        printf("\nerror on line %d: Cannot perform compile-time add on values of different types\n", u24_unpack(expression.line));
        stop_parsing();
        return (Expression){0};
    }

    if(a.kind == EXPRESSION_INT){
        return (Expression){
            .kind = EXPRESSION_INT,
            .ops = a.ops + b.ops,
            .line = expression.line,
        };
    }

    printf("\nerror on line %d: Cannot perform compile-time add for that type\n", u24_unpack(expression.line));
    stop_parsing();
    return (Expression){0};
}

static Expression perform_compile_time_subtract(Expression expression){
    Expression a = perform_compile_time_math(expressions[operands[expression.ops]]);
    Expression b = perform_compile_time_math(expressions[operands[expression.ops + 1]]);

    if(a.kind != b.kind){
        printf("\nerror on line %d: Cannot perform compile-time subtract on values of different types\n", u24_unpack(expression.line));
        stop_parsing();
        return (Expression){0};
    }

    if(a.kind == EXPRESSION_INT){
        return (Expression){
            .kind = EXPRESSION_INT,
            .ops = a.ops - b.ops,
            .line = expression.line,
        };
    }

    printf("\nerror on line %d: Cannot perform compile-time subtract for that type\n", u24_unpack(expression.line));
    stop_parsing();
    return (Expression){0};
}

static Expression perform_compile_time_multiply(Expression expression){
    Expression a = perform_compile_time_math(expressions[operands[expression.ops]]);
    Expression b = perform_compile_time_math(expressions[operands[expression.ops + 1]]);

    if(a.kind != b.kind){
        printf("\nerror on line %d: Cannot perform compile-time multiply on values of different types\n", u24_unpack(expression.line));
        stop_parsing();
        return (Expression){0};
    }

    if(a.kind == EXPRESSION_INT){
        return (Expression){
            .kind = EXPRESSION_INT,
            .ops = a.ops * b.ops,
            .line = expression.line,
        };
    }

    printf("\nerror on line %d: Cannot perform compile-time multiply for that type\n", u24_unpack(expression.line));
    stop_parsing();
    return (Expression){0};
}

static Expression perform_compile_time_divide(Expression expression){
    Expression a = perform_compile_time_math(expressions[operands[expression.ops]]);
    Expression b = perform_compile_time_math(expressions[operands[expression.ops + 1]]);

    if(a.kind != b.kind){
        printf("\nerror on line %d: Cannot perform compile-time divide on values of different types\n", u24_unpack(expression.line));
        stop_parsing();
        return (Expression){0};
    }

    if(a.kind == EXPRESSION_INT){
        return (Expression){
            .kind = EXPRESSION_INT,
            .ops = a.ops / b.ops,
            .line = expression.line,
        };
    }

    printf("\nerror on line %d: Cannot perform compile-time divide for that type\n", u24_unpack(expression.line));
    stop_parsing();
    return (Expression){0};
}

static Expression perform_compile_time_modulo(Expression expression){
    Expression a = perform_compile_time_math(expressions[operands[expression.ops]]);
    Expression b = perform_compile_time_math(expressions[operands[expression.ops + 1]]);

    if(a.kind != b.kind){
        printf("\nerror on line %d: Cannot perform compile-time modulo on values of different types\n", u24_unpack(expression.line));
        stop_parsing();
        return (Expression){0};
    }

    if(a.kind == EXPRESSION_INT){
        return (Expression){
            .kind = EXPRESSION_INT,
            .ops = a.ops % b.ops,
            .line = expression.line,
        };
    }

    printf("\nerror on line %d: Cannot perform compile-time modulo for that type\n", u24_unpack(expression.line));
    stop_parsing();
    return (Expression){0};
}

Expression perform_compile_time_math(Expression expression){
    switch(expression.kind){
    case EXPRESSION_INT:
    case EXPRESSION_STRING:
        return expression;
    case EXPRESSION_ADD:
        return perform_compile_time_add(expression);
    case EXPRESSION_SUBTRACT:
        return perform_compile_time_subtract(expression);
    case EXPRESSION_MULTIPLY:
        return perform_compile_time_multiply(expression);
    case EXPRESSION_DIVIDE:
        return perform_compile_time_divide(expression);
    case EXPRESSION_MOD:
        return perform_compile_time_modulo(expression);
    default:
        printf("\nerror on line %d: Cannot perform compile-time math on '#define' value\n", u24_unpack(expression.line));
        stop_parsing();
        return (Expression){0};
    }
}

