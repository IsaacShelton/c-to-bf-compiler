
#include <stdio.h>
#include "../include/parse_expression.h"
#include "../include/parse_context.h"
#include "../include/storage.h"

static Expression parse_secondary_expression(u8 precedence, Expression lhs);

static Expression parse_expression_print(u24 line_number){
    Expression expression = (Expression){0};

    if(is_token(TOKEN_STRING)){
        expression = (Expression){
            .kind = EXPRESSION_PRINT_LITERAL,
            .line = line_number,
            .ops = eat_string(),
        };
    } else {
        Expression array = parse_expression();
        if(had_parse_error) return expression;

        u32 array_value = add_expression(array);
        if(array_value >= EXPRESSIONS_CAPACITY){
            stop_parsing();
            return expression;
        }
        
        expression = (Expression){
            .kind = EXPRESSION_PRINT_ARRAY,
            .line = line_number,
            .ops = array_value,
        };
    }

    if(!eat_token(TOKEN_CLOSE)){
        printf("error on line %d: Expected ')' after expression in print statement\n", current_line());
        instead_got();
        stop_parsing();
        return expression;
    }

    return expression;
}

static Expression parse_expression_call(u32 name, u24 line_number){
    Expression expression = (Expression){
        .kind = EXPRESSION_CALL,
        .line = line_number,
        .ops = 0,
    };

    u32 arguments[ARITY_CAPACITY];
    u8 arity = 0;

    while(!is_token(TOKEN_CLOSE)){
        Expression argument_expression = parse_expression();
        if(had_parse_error) return expression;

        u32 argument = add_expression(argument_expression);
        if(argument >= EXPRESSIONS_CAPACITY){
            stop_parsing();
            return expression;
        }

        if(arity >= ARITY_CAPACITY){
            printf("error on line %d: Exceeded maximum number of arguments to function call (limit is %d)\n", current_line(), ARITY_CAPACITY);
            stop_parsing();
            return expression;
        }

        arguments[arity++] = argument;

        if(!eat_token(TOKEN_NEXT)){
            if(!is_token(TOKEN_CLOSE)){
                printf("error on line %d: Expected ',' or ')' after argument in call\n", current_line());
                instead_got();
                stop_parsing();
                return expression;
            }
        }
    }

    if(!eat_token(TOKEN_CLOSE)){
        printf("error on line %d: Expected ')' after argument in call\n", current_line());
        instead_got();
        stop_parsing();
        return expression;
    }

    /* Operands layout */
    /* { name, arity, arg1, arg2, ..., argN } */
    u32 ops = add_operand(name);
    if(ops >= OPERANDS_CAPACITY){
        stop_parsing();
        return expression;
    }

    u32 arity_location = add_operand(arity);
    if(arity_location >= OPERANDS_CAPACITY){
        stop_parsing();
        return expression;
    }

    for(u8 i = 0; i < arity; i++){
        if(add_operand(arguments[i]) >= OPERANDS_CAPACITY){
            stop_parsing();
            return expression;
        }
    }

    expression.ops = ops;
    return expression;
}

static Expression parse_primary_expression(){
    u24 line_number = current_line_packed();

    if(is_token(TOKEN_WORD)){
        u32 name = eat_word();

        if(eat_token(TOKEN_OPEN)){
            if(aux_cstr_equals_print(name)){
                // Print call?
                return parse_expression_print(line_number);
            }

            // Regular call
            return parse_expression_call(name, line_number);
        }

        // Else, normal variable
        return (Expression){
            .kind = EXPRESSION_VARIABLE,
            .line = line_number,
            .ops = name,
        };
    }

    if(is_token(TOKEN_INT)){
        return (Expression){
            .kind = EXPRESSION_INT,
            .line = line_number,
            .ops = eat_int(),
        };
    }
    
    printf("error on line %d: Expected expression\n", current_line());
    stop_parsing();
    return (Expression){0};
}

static u1 is_terminating_token(TokenKind token_kind){
    switch(token_kind){
    case TOKEN_NEXT:
    case TOKEN_CLOSE:
    case TOKEN_SEMICOLON:
    case TOKEN_CLOSE_BRACKET:
        return true;
    default:
        return false;
    }
}

static u8 parse_get_precedence(u32 token_kind){
    switch(token_kind){
    case TOKEN_OPEN_BRACKET:
        return 16;
    case TOKEN_MULTIPLY:
    case TOKEN_DIVIDE:
    case TOKEN_MOD:
        return 12;
    case TOKEN_ADD:
    case TOKEN_SUBTRACT:
        return 11;
    case TOKEN_LSHIFT:
    case TOKEN_RSHIFT:
        return 10;
    case TOKEN_LESS_THAN:
    case TOKEN_GREATER_THAN:
    case TOKEN_LESS_THAN_OR_EQUAL:
    case TOKEN_GREATER_THAN_OR_EQUAL:
        return 9;
    case TOKEN_EQUALS:
    case TOKEN_NOT_EQUALS:
        return 8;
    case TOKEN_ASSIGN:
        return 1;
    default:
        return 0;
    }
}

static u1 is_right_associative(TokenKind operator){
    switch(operator){
    case TOKEN_ASSIGN:
        return true;
    default:
        return false;
    }
}

static Expression parse_rhs(u32 operator_precedence){
    // Returns right hand side of expression

    // Skip over operator token
    if(++parse_i >= num_tokens){
        printf("\nerror: Expected right hand side of expression\n");
        stop_parsing();
        return (Expression){0};
    }

    Expression rhs = parse_primary_expression();
    if(had_parse_error) return rhs;

    TokenKind next_operator = tokens[parse_i].kind;
    u8 next_precedence = parse_get_precedence(next_operator);

    if(!(next_precedence + is_right_associative(next_operator) < operator_precedence)){
        rhs = parse_secondary_expression(operator_precedence + 1, rhs);
        if(had_parse_error) return rhs;
    }

    return rhs;
}

static Expression parse_math(
    Expression lhs,
    TokenKind operator,
    u24 line_number,
    u8 operator_precedence
){
    Expression rhs = parse_rhs(operator_precedence);
    if(had_parse_error) return lhs;

    u32 a = add_expression(lhs);
    if(a >= EXPRESSIONS_CAPACITY){
        stop_parsing();
        return lhs;
    }

    u32 b = add_expression(rhs);
    if(b >= EXPRESSIONS_CAPACITY){
        stop_parsing();
        return lhs;
    }

    u32 ops = add_operands2(a, b);
    if(ops >= OPERANDS_CAPACITY){
        stop_parsing();
        return lhs;
    }

    ExpressionKind expression_kind = expression_kind_from_token_kind(operator);
    if(expression_kind == EXPRESSION_NONE){
        printf("\nerror on line %d: Could not get math expression kind from token kind\n", u24_unpack(line_number));
        stop_parsing();
        return lhs;
    }

    return (Expression){
        .kind = expression_kind,
        .line = line_number,
        .ops = ops,
    };
}

static Expression parse_array_index(Expression array, u24 line_number){
    // Skip over '[' operator
    parse_i++;

    Expression index = parse_expression();
    if(had_parse_error) return array;

    if(!eat_token(TOKEN_CLOSE_BRACKET)){
        printf("error: Expected ']' after index expression\n");
        instead_got();
        stop_parsing();
        return array;
    }

    u32 a = add_expression(array);
    if(a >= EXPRESSIONS_CAPACITY){
        stop_parsing();
        return array;
    }

    u32 b = add_expression(index);
    if(b >= EXPRESSIONS_CAPACITY){
        stop_parsing();
        return array;
    }

    u32 ops = add_operands2(a, b);
    if(ops >= OPERANDS_CAPACITY){
        stop_parsing();
        return array;
    }

    return (Expression){
        .kind = EXPRESSION_INDEX,
        .line = line_number,
        .ops = ops,
    };
}

static Expression parse_secondary_expression(u8 precedence, Expression lhs){
    while(true){
        if(parse_i >= num_tokens){
            return lhs;
        }

        TokenKind operator = tokens[parse_i].kind;
        u8 next_precedence = parse_get_precedence(operator);
        u24 line_number = current_line_packed();

        if(is_terminating_token(operator) || next_precedence + is_right_associative(operator) < precedence){
            return lhs;
        }

        switch(operator){
        case TOKEN_ASSIGN:
        case TOKEN_ADD:
        case TOKEN_SUBTRACT:
        case TOKEN_MULTIPLY:
        case TOKEN_DIVIDE:
        case TOKEN_MOD:
        case TOKEN_LESS_THAN:
        case TOKEN_GREATER_THAN:
        case TOKEN_LESS_THAN_OR_EQUAL:
        case TOKEN_GREATER_THAN_OR_EQUAL:
        case TOKEN_LSHIFT:
        case TOKEN_RSHIFT:
        case TOKEN_EQUALS:
        case TOKEN_NOT_EQUALS:
            lhs = parse_math(lhs, operator, line_number, next_precedence);
            break;
        case TOKEN_OPEN_BRACKET:
            lhs = parse_array_index(lhs, line_number);
            break;
        default:
            return lhs;
        }

        if(had_parse_error) return lhs;
    }

    return lhs;
}

Expression parse_expression(){
    Expression primary = parse_primary_expression();
    if(had_parse_error) return primary;

    return parse_secondary_expression(0, primary);
}

