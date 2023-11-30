
#include <stdio.h>
#include "../include/parse_expression.h"
#include "../include/parse_context.h"
#include "../include/parse_type.h"
#include "../include/storage.h"

static Expression parse_operator_expression(u8 precedence, Expression lhs);
static Expression parse_unary_prefix(TokenKind operator, u24 line_number);
static Expression parse_unary_postfix(Expression expression);
static Expression parse_primary_base_expression();
static Expression parse_primary_post_expression(Expression expression);
static Expression parse_array_index_expression(Expression array, u24 line_number);
static Expression parse_member_expression(Expression array, u24 line_number);

static Expression parse_primary_expression(){
    Expression expression = parse_primary_base_expression();
    if(had_parse_error) return (Expression){0};

    expression = parse_primary_post_expression(expression);
    if(had_parse_error) return (Expression){0};

    return expression;
}

static Expression parse_primary_post_expression(Expression lhs){
    while(true){
        u24 line_number = tokens[parse_i].line;

        switch(tokens[parse_i].kind){
        case TOKEN_OPEN_BRACKET:
            lhs = parse_array_index_expression(lhs, line_number);
            break;
        case TOKEN_MEMBER:
            lhs = parse_member_expression(lhs, line_number);
            break;
        case TOKEN_INCREMENT:
        case TOKEN_DECREMENT:
            lhs = parse_unary_postfix(lhs);
            break;
        default:
            return lhs;
        }
    }

    return lhs;
}

static Expression parse_array_index_expression(Expression array, u24 line_number){
    // Skip over '[' operator
    parse_i++;

    Expression index = parse_expression();
    if(had_parse_error) return array;

    if(!eat_token(TOKEN_CLOSE_BRACKET)){
        printf("\nerror on line %d: Expected ']' after index expression\n", current_line());
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

static Expression parse_member_expression(Expression subject_expression, u24 line_number){
    // Skip over '.' operator
    parse_i++;

    u32 subject = add_expression(subject_expression);
    if(subject >= EXPRESSIONS_CAPACITY){
        stop_parsing();
        return subject_expression;
    }

    if(!is_token(TOKEN_WORD)){
        printf("\nerror on line %d: Expected member name after '.'\n", u24_unpack(line_number));
        instead_got();
        stop_parsing();
        return subject_expression;
    }

    u32 member_name = eat_word();

    u32 ops = add_operands2(subject, member_name);
    if(ops >= OPERANDS_CAPACITY){
        stop_parsing();
        return subject_expression;
    }

    return (Expression){
        .kind = EXPRESSION_MEMBER,
        .line = line_number,
        .ops = ops,
    };
}


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
        printf("\nerror on line %d: Expected ')' after expression in print statement\n", current_line());
        instead_got();
        stop_parsing();
        return expression;
    }

    return expression;
}

static Expression parse_expression_panicloop(u24 line_number){
    if(!eat_token(TOKEN_CLOSE)){
        printf("\nerror on line %d: Expected ')' after expression in print statement\n", current_line());
        instead_got();
        stop_parsing();
        return (Expression){0};
    }

    return (Expression){
        .kind = EXPRESSION_PANICLOOP,
        .line = line_number,
        .ops = 0,
    };
}

static Expression parse_expression_call(u32 name, u24 line_number){
    Expression expression = (Expression){
        .kind = EXPRESSION_CALL,
        .line = line_number,
        .ops = 0,
    };

    if(aux_cstr_equals_printf(name)){
        expression.kind = EXPRESSION_PRINTF;
    } else if(aux_cstr_equals_memcmp(name)){
        expression.kind = EXPRESSION_MEMCMP;
    } else if(aux_cstr_equals_memcpy(name)){
        expression.kind = EXPRESSION_MEMCPY;
    }

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
            printf("\nerror on line %d: Exceeded maximum number of arguments to function call (limit is %d)\n", current_line(), ARITY_CAPACITY);
            stop_parsing();
            return expression;
        }

        arguments[arity++] = argument;

        if(!eat_token(TOKEN_NEXT)){
            if(!is_token(TOKEN_CLOSE)){
                printf("\nerror on line %d: Expected ',' or ')' after argument in call\n", current_line());
                instead_got();
                stop_parsing();
                return expression;
            }
        }
    }

    if(!eat_token(TOKEN_CLOSE)){
        printf("\nerror on line %d: Expected ')' after argument in call\n", current_line());
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

static Expression parse_expression_sizeof(u24 line_number){
    // sizeof
    //        ^

    if(eat_token(TOKEN_OPEN)){
        Type type = parse_type();
        if(had_parse_error) return (Expression){0};

        if(!eat_token(TOKEN_CLOSE)){
            printf("\nerror on line %d: Expected ')' after type in 'sizeof'\n", current_line());
            instead_got();
            stop_parsing();
            return (Expression){0};
        }

        u32 type_index = add_type(type);
        if(type_index >= TYPES_CAPACITY){
            stop_parsing();
            return (Expression){0};
        }

        return (Expression){
            .kind = EXPRESSION_SIZEOF_TYPE,
            .line = line_number,
            .ops = type_index,
        };
    } else {
        Expression target_expression = parse_primary_expression();
        if(had_parse_error) return (Expression){0};

        u32 target = add_expression(target_expression);
        if(target >= EXPRESSIONS_CAPACITY){
            stop_parsing();
            return target_expression;
        }

        return (Expression){
            .kind = EXPRESSION_SIZEOF_VALUE,
            .line = line_number,
            .ops = target,
        };
    }
}

static u32 create_field_initializer(u32 field_name, u32 field_value, u24 line_number){
    // Creates and expression for `.field_name = field_value`

    u32 ops = add_operands2(field_name, field_value);
    if(ops >= OPERANDS_CAPACITY){
        stop_parsing();
        return EXPRESSIONS_CAPACITY;
    }

    Expression member_expression = (Expression){
        .kind = EXPRESSION_FIELD_INITIALIZER,
        .ops = ops,
        .line = line_number,
    };

    u32 member = add_expression(member_expression);
    if(member >= EXPRESSIONS_CAPACITY){
        stop_parsing();
    }

    return member;
}

static Expression parse_struct_initializer(Type type, u24 line_number){
    // (Type) { 
    //          ^

    u32 length = 0;
    u32 temporary_storage[MAX_INITIALIZER_LIST_LENGTH];

    u1 skip = false;

    if(is_token(TOKEN_INT)){
        u32 value = eat_int();

        if(value == 0 && eat_token(TOKEN_END)){
            skip = true;
        } else {
            printf("\nerror on line %d: Expected '{0}' for zero-initializer\n", u24_unpack(line_number));
            stop_parsing();
            return (Expression){0};
        }
    }

    if(!skip) while(!eat_token(TOKEN_END)){
        if(!eat_token(TOKEN_MEMBER)){
            printf("\nerror on line %d: Expected '.' for field of struct literal\n", u24_unpack(line_number));
            stop_parsing();
            return (Expression){0};
        }

        if(!is_token(TOKEN_WORD)){
            printf("\nerror on line %d: Expected name of field after '.' in struct literal\n", u24_unpack(line_number));
            stop_parsing();
            return (Expression){0};
        }

        u32 field = eat_word();
        u24 line_number = current_line_packed();

        if(!eat_token(TOKEN_ASSIGN)){
            printf("\nerror on line %d: Expected '=' after name of field in struct literal\n", u24_unpack(line_number));
            stop_parsing();
            return (Expression){0};
        }

        Expression field_value_expression = parse_expression();
        if(had_parse_error) return (Expression){0};

        u32 field_value = add_expression(field_value_expression);
        if(field_value >= EXPRESSIONS_CAPACITY){
            stop_parsing();
            return (Expression){0};
        }

        if(length + 1 >= MAX_INITIALIZER_LIST_LENGTH){
            printf("\nerror on line %d: Exceeded maximum struct initializer length\n", u24_unpack(line_number));
            stop_parsing();
            return (Expression){0};
        }

        u32 member = create_field_initializer(field, field_value, line_number);
        if(member >= EXPRESSIONS_CAPACITY) return (Expression){0};

        temporary_storage[length++] = member;

        if(!eat_token(TOKEN_NEXT)){
            if(!is_token(TOKEN_END)){
                printf("\nerror on line %d: Expected '}' or ',' after item of array initializer list\n", u24_unpack(line_number));
                instead_got();
                stop_parsing();
                return (Expression){0};
            }
        }
    }

    if(num_operands + length + 2 >= OPERANDS_CAPACITY){
        printf("\nOut of memory: Exceeded maximum number of total expression operands\n");
        stop_parsing();
        return (Expression){0};
    }

    u32 type_index = add_type(type);
    if(type_index >= TYPES_CAPACITY){
        stop_parsing();
        return (Expression){0};
    }

    u32 ops = num_operands;
    operands[num_operands++] = type_index;
    operands[num_operands++] = length;

    for(u32 i = 0; i < length; i++){
        operands[num_operands++] = temporary_storage[i];
    }

    return (Expression){
        .kind = EXPRESSION_STRUCT_INITIALIZER,
        .ops = ops,
        .line = line_number,
    };
}

static Expression parse_array_initializer_expression(u24 line_number){
    // {
    //   ^

    u32 length = 0;
    u32 temporary_storage[MAX_INITIALIZER_LIST_LENGTH];

    while(!eat_token(TOKEN_END)){
        Expression item_expression = parse_expression();
        if(had_parse_error) return (Expression){0};

        u32 item = add_expression(item_expression);
        if(item >= EXPRESSIONS_CAPACITY){
            stop_parsing();
            return (Expression){0};
        }

        if(length + 1 >= MAX_INITIALIZER_LIST_LENGTH){
            printf("\nerror on line %d: Exceeded maximum array initializer length\n", u24_unpack(line_number));
            stop_parsing();
            return (Expression){0};
        }

        temporary_storage[length++] = item;

        if(!eat_token(TOKEN_NEXT)){
            if(!is_token(TOKEN_END)){
                printf("\nerror on line %d: Expected '}' or ',' after item of array initializer list\n", u24_unpack(line_number));
                instead_got();
                stop_parsing();
                return (Expression){0};
            }
        }
    }

    if(num_operands + length + 1 >= OPERANDS_CAPACITY){
        printf("\nOut of memory: Exceeded maximum number of total expression operands\n");
        stop_parsing();
        return (Expression){0};
    }

    u32 ops = num_operands;
    operands[num_operands++] = length;

    for(u32 i = 0; i < length; i++){
        operands[num_operands++] = temporary_storage[i];
    }

    return (Expression){
        .kind = EXPRESSION_ARRAY_INITIALIZER,
        .ops = ops,
        .line = line_number,
    };
}

static Expression parse_primary_base_expression(){
    u24 line_number = current_line_packed();

    if(is_token(TOKEN_WORD)){
        u32 name = eat_word();

        if(eat_token(TOKEN_OPEN)){
            if(aux_cstr_equals_print(name)){
                // Print call?
                return parse_expression_print(line_number);
            }

            if(aux_cstr_equals_panicloop(name)){
                return parse_expression_panicloop(line_number);
            }

            // Regular call
            return parse_expression_call(name, line_number);
        }

        // Resolve '#define' constants
        u32 resolved = try_resolve_define(name);
        if(resolved < EXPRESSIONS_CAPACITY){
            return expressions[resolved];
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

    if(eat_token(TOKEN_TRUE)){
        return (Expression){
            .kind = EXPRESSION_U1,
            .line = line_number,
            .ops = 1,
        };
    }

    if(eat_token(TOKEN_FALSE)){
        return (Expression){
            .kind = EXPRESSION_U1,
            .line = line_number,
            .ops = 0,
        };
    }

    if(is_token(TOKEN_STRING)){
        return (Expression){
            .kind = EXPRESSION_STRING,
            .line = line_number,
            .ops = eat_string(),
        };
    }

    if(eat_token(TOKEN_OPEN)){
        if(is_type_followed_by(TOKEN_CLOSE)){
            // Cast

            Type type = parse_type();
            if(had_parse_error){
                printf("\nerror on line %d: Expected type for cast\n", current_line());
                return (Expression){0};
            }

            if(!eat_token(TOKEN_CLOSE)){
                printf("\nerror on line %d: Expected ')' to close cast\n", current_line());
                instead_got();
                stop_parsing();
                return (Expression){0};
            }

            if(eat_token(TOKEN_BEGIN)){
                return parse_struct_initializer(type, line_number);
            } else {
                Expression expression = parse_primary_expression();
                if(had_parse_error) return (Expression){0};

                u32 type_index = add_type(type);
                if(type_index >= TYPES_CAPACITY){
                    stop_parsing();
                    return (Expression){0};
                }

                u32 expression_index = add_expression(expression);
                if(expression_index >= EXPRESSIONS_CAPACITY){
                    stop_parsing();
                    return (Expression){0};
                }

                u32 ops = add_operands2(type_index, expression_index);
                if(ops >= OPERANDS_CAPACITY){
                    stop_parsing();
                    return (Expression){0};
                }

                return (Expression){
                    .kind = EXPRESSION_CAST,
                    .line = line_number,
                    .ops = ops,
                };
            }
        }

        // Normal nested expression
        Expression expression = parse_expression();
        if(had_parse_error) return (Expression){0};

        if(!eat_token(TOKEN_CLOSE)){
            printf("\nerror on line %d: Expected ')' to close cast\n", current_line());
            instead_got();
            stop_parsing();
            return (Expression){0};
        }

        return expression;
    }

    if(eat_token(TOKEN_BEGIN)){
        return parse_array_initializer_expression(line_number);
    }

    if( eat_token(TOKEN_SUBTRACT)
     || eat_token(TOKEN_NOT)
     || eat_token(TOKEN_BIT_COMPLEMENT)
     || eat_token(TOKEN_INCREMENT)
     || eat_token(TOKEN_DECREMENT)
    ){
        return parse_unary_prefix(tokens[parse_i - 1].kind, line_number);
    }

    if(eat_token(TOKEN_SIZEOF)){
        return parse_expression_sizeof(line_number);
    }
    
    printf("\nerror on line %d: Expected expression\n", current_line());
    instead_got();
    stop_parsing();
    return (Expression){0};
}

static u1 is_terminating_token(TokenKind token_kind){
    switch(token_kind){
    case TOKEN_NEXT:
    case TOKEN_CLOSE:
    case TOKEN_SEMICOLON:
    case TOKEN_CLOSE_BRACKET:
    case TOKEN_COLON:
        return true;
    default:
        return false;
    }
}

static u8 parse_get_precedence(u32 token_kind){
    switch(token_kind){
    case TOKEN_OPEN:
    case TOKEN_OPEN_BRACKET:
    case TOKEN_MEMBER:
        return 16;
    case TOKEN_INCREMENT:
    case TOKEN_DECREMENT:
        return 15;
    case TOKEN_NOT:
    case TOKEN_BIT_COMPLEMENT:
        return 14;
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
    case TOKEN_BIT_AND:
        return 7;
    case TOKEN_BIT_XOR:
        return 6;
    case TOKEN_BIT_OR:
        return 5;
    case TOKEN_AND:
        return 4;
    case TOKEN_OR:
        return 3;
    case TOKEN_TERNARY:
        return 2;
    case TOKEN_ASSIGN:
    case TOKEN_ADD_ASSIGN:
    case TOKEN_SUBTRACT_ASSIGN:
    case TOKEN_MULTIPLY_ASSIGN:
    case TOKEN_DIVIDE_ASSIGN:
    case TOKEN_MOD_ASSIGN:
    case TOKEN_LSHIFT_ASSIGN:
    case TOKEN_RSHIFT_ASSIGN:
    case TOKEN_BIT_AND_ASSIGN:
    case TOKEN_BIT_OR_ASSIGN:
    case TOKEN_BIT_XOR_ASSIGN:
        return 1;
    default:
        return 0;
    }
}

static u1 is_right_associative(TokenKind operator){
    switch(operator){
    case TOKEN_ASSIGN:
    case TOKEN_TERNARY:
    case TOKEN_COLON:
        return true;
    default:
        return false;
    }
}

static Expression parse_rhs(u32 operator_precedence){
    // Returns right hand side of expression

    // Skip over operator token
    if(++parse_i >= num_tokens){
        printf("\nerror on line %d: Expected right hand side of expression\n", current_line());
        instead_got();
        stop_parsing();
        return (Expression){0};
    }

    Expression rhs = parse_primary_expression();
    if(had_parse_error) return rhs;

    TokenKind next_operator = tokens[parse_i].kind;
    u8 next_precedence = parse_get_precedence(next_operator);

    if(!(next_precedence + is_right_associative(next_operator) < operator_precedence)){
        rhs = parse_operator_expression(operator_precedence + 1, rhs);
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

    ExpressionKind expression_kind = expression_kind_binary_from_token_kind(operator);
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

static Expression parse_unary_prefix(
    TokenKind operator,
    u24 line_number
){
    Expression expression = parse_primary_expression();
    if(had_parse_error) return (Expression){0};

    u32 value = add_expression(expression);
    if(value >= EXPRESSIONS_CAPACITY){
        stop_parsing();
        return (Expression){0};
    }

    ExpressionKind expression_kind = expression_kind_unary_prefix_from_token_kind(operator);
    if(expression_kind == EXPRESSION_NONE){
        printf("\nerror on line %d: Could not get unary prefix expression kind from token kind\n", u24_unpack(line_number));
        stop_parsing();
        return (Expression){0};
    }

    return (Expression){
        .kind = expression_kind,
        .line = line_number,
        .ops = value,
    };
}

static Expression parse_unary_postfix(Expression expression){
    TokenKind operator = tokens[parse_i++].kind;

    u32 value = add_expression(expression);
    if(value >= EXPRESSIONS_CAPACITY){
        stop_parsing();
        return (Expression){0};
    }

    ExpressionKind expression_kind = expression_kind_unary_postfix_from_token_kind(operator);
    if(expression_kind == EXPRESSION_NONE){
        printf("\nerror on line %d: Could not get unary postfix expression kind from token kind\n", u24_unpack(expression.line));
        stop_parsing();
        return (Expression){0};
    }

    return (Expression){
        .kind = expression_kind,
        .line = expression.line,
        .ops = value,
    };
}

static Expression parse_ternary(Expression condition_expression, u24 line){
    u32 condition = add_expression(condition_expression);
    if(condition >= EXPRESSIONS_CAPACITY){
        stop_parsing();
        return (Expression){0};
    }

    if(!eat_token(TOKEN_TERNARY)){
        printf("\nerror on line %d: Expected '?' after condition of ternary expression\n", current_line());
        instead_got();
        stop_parsing();
        return (Expression){0};
    }

    Expression when_true_expression = parse_expression();
    if(had_parse_error) return (Expression){0};

    if(!eat_token(TOKEN_COLON)){
        printf("\nerror on line %d: Expected ':' after true branch of ternary expression\n", current_line());
        instead_got();
        stop_parsing();
        return (Expression){0};
    }
    
    Expression when_false_expression = parse_expression();
    if(had_parse_error) return (Expression){0};

    u32 when_true = add_expression(when_true_expression);
    if(when_true >= EXPRESSIONS_CAPACITY){
        stop_parsing();
        return condition_expression;
    }

    u32 when_false = add_expression(when_false_expression);
    if(when_false >= EXPRESSIONS_CAPACITY){
        stop_parsing();
        return condition_expression;
    }

    u32 ops = add_operands3(condition, when_true, when_false);

    return (Expression){
        .kind = EXPRESSION_TERNARY,
        .line = line,
        .ops = ops,
    };
}

static Expression parse_operator_assign(Expression lhs, TokenKind operator, u24 line_number, u8 operator_precedence){
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

    u32 math_ops = add_operands2(a, b);
    if(math_ops >= OPERANDS_CAPACITY){
        stop_parsing();
        return lhs;
    }

    ExpressionKind expression_kind = expression_kind_binary_from_assignment_token_kind(operator);
    if(expression_kind == EXPRESSION_NONE){
        printf("\nerror on line %d: Could not get math expression kind from token kind\n", u24_unpack(line_number));
        stop_parsing();
        return lhs;
    }

    Expression math_expression = (Expression){
        .kind = expression_kind,
        .line = line_number,
        .ops = math_ops,
    };

    u32 math = add_expression(math_expression);
    if(math >= EXPRESSIONS_CAPACITY){
        stop_parsing();
        return math_expression;
    }

    u32 ops = add_operands2(a, math);
    if(ops >= OPERANDS_CAPACITY){
        stop_parsing();
        return lhs;
    }

    return (Expression){
        .kind = EXPRESSION_ASSIGN,
        .ops = ops,
        .line = line_number,
    };
}

static Expression parse_operator_expression(u8 precedence, Expression lhs){
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
        case TOKEN_AND:
        case TOKEN_OR:
        case TOKEN_BIT_AND:
        case TOKEN_BIT_OR:
        case TOKEN_BIT_XOR:
            lhs = parse_math(lhs, operator, line_number, next_precedence);
            break;
        case TOKEN_TERNARY:
            lhs = parse_ternary(lhs, line_number);
            break;
        case TOKEN_ADD_ASSIGN:
        case TOKEN_SUBTRACT_ASSIGN:
        case TOKEN_MULTIPLY_ASSIGN:
        case TOKEN_DIVIDE_ASSIGN:
        case TOKEN_MOD_ASSIGN:
        case TOKEN_LSHIFT_ASSIGN:
        case TOKEN_RSHIFT_ASSIGN:
        case TOKEN_BIT_AND_ASSIGN:
        case TOKEN_BIT_OR_ASSIGN:
        case TOKEN_BIT_XOR_ASSIGN:
            lhs = parse_operator_assign(lhs, operator, line_number, next_precedence);
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

    return parse_operator_expression(0, primary);
}

