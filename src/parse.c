
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include "../include/parse.h"
#include "../include/storage.h"
#include "../include/token_print.h"
#include "../include/type_print.h"
#include "../include/global_print.h"
#include "../include/expression.h"
#include "../include/builtin_functions.h"
#include "../include/expression_print.h"
#include "../include/builtin_types.h"

u32 parse_i = 0;
u1 had_parse_error = false;

static u0 instead_got(){
    if(parse_i < num_tokens){
        printf("  Instead got: `");
        token_print(tokens[parse_i]);
        printf("`\n");
    }
}

static u0 stop_parsing(){
    had_parse_error = true;
    parse_i = num_tokens;
}

static u1 is_token(TokenKind kind){
    return parse_i < num_tokens && tokens[parse_i].kind == kind;
}

static u32 current_line(){
    if(parse_i < num_tokens){
        return u24_unpack(tokens[parse_i].line);
    } else {
        return 0;
    }
}

static u32 eat_word(){
    // Expects `is_token(TOKEN_WORD)` to be true
    return tokens[parse_i++].data;
}

static u32 eat_int(){
    // Expects `is_token(TOKEN_INT)` to be true
    return tokens[parse_i++].data;
}

static u32 eat_string(){
    // Expects `is_token(TOKEN_STRING)` to be true
    return tokens[parse_i++].data;
}

static u1 eat_token(TokenKind kind){
    if(parse_i < num_tokens && tokens[parse_i].kind == kind){
        parse_i++;
        return true;
    } else {
        return false;
    }
}

static Type parse_type(){
    Type type = {
        .name = 0,
        .dimensions = 0,
    };

    // Parse type name
    if(!is_token(TOKEN_WORD)){
        printf("error on line %d: Expected type\n", current_line());
        instead_got();
        stop_parsing();
        return type;
    }

    type.name = eat_word();

    // Parse dimensions
    u32 type_dimensions[4] = {0, 0, 0, 0};
    u32 next_dim = 0;

    while(eat_token(TOKEN_OPEN_BRACKET)){
        if(next_dim == 4){
            printf("error on line %d: Cannot nest array dimensions more than 4 deep\n", current_line());
            stop_parsing();
            return type;
        }

        if(!is_token(TOKEN_INT)){
            printf("error on line %d: Expected number for dimension of type\n", current_line());
            instead_got();
            stop_parsing();
            return type;
        }

        u32 dim = eat_int();

        if(dim == 0){
            printf("error on line %d: Cannot have array dimension of 0\n", current_line());
            stop_parsing();
            return type;
        }

        type_dimensions[next_dim++] = dim;

        if(!eat_token(TOKEN_CLOSE_BRACKET)){
            printf("error on line %d: Expected ']' after dimension in type\n", current_line());
            instead_got();
            stop_parsing();
            return type;
        }
    }

    if(next_dim != 0){
        // Try to find existing slot with same value
        for(u32 i = 0; i < UNIQUE_DIMENSIONS_CAPACITY; i++){
            u1 match = memcmp(dimensions[i], type_dimensions, sizeof(u32[4]));

            if(match){
                type.dimensions = i;
                break;
            }
        }

        if(type.dimensions == 0){
            // Insert if not found

            if(num_dimensions + 1 >= UNIQUE_DIMENSIONS_CAPACITY){
                printf("error on line %d: Maximum number of unique dimension pairs exceeded. Use fewer or reconfigure your compiler.\n", current_line());
                stop_parsing();
                return type;
            }

            for(u32 i = 0; i < 4; i++){
                dimensions[num_dimensions][i] = type_dimensions[i];
            }

            type.dimensions = num_dimensions++;
        }
    }

    return type;
}

static u1 is_declaration(){
    // Returns whether next statement during parsing is a declaration.
    // Trys to match current token parsing context against `TypeName[1][2][3][4] name`-like sequence.

    u32 prev_parse_i = parse_i;
    u1 ok = eat_token(TOKEN_WORD);

    for(u32 i = 1; ok && is_token(TOKEN_OPEN_BRACKET); i++){
        if(
            !eat_token(TOKEN_OPEN_BRACKET)
         || !eat_token(TOKEN_INT)
         || !eat_token(TOKEN_CLOSE_BRACKET)
         || i > 4
        ){
            ok = false;
            break;
        }
    }

    ok = ok && is_token(TOKEN_WORD);
    parse_i = prev_parse_i;
    return ok;
}

static u1 is_assignment(){
    // Returns whether next statement during parsing is an assignment.
    // Trys to match current token parsing context against `variable_name[1][2][3][4] =`-like sequence.

    u32 prev_parse_i = parse_i;
    u1 ok = eat_token(TOKEN_WORD);

    for(u32 i = 1; ok && is_token(TOKEN_OPEN_BRACKET); i++){
        if(
            !eat_token(TOKEN_OPEN_BRACKET)
         || !eat_token(TOKEN_INT)
         || !eat_token(TOKEN_CLOSE_BRACKET)
         || i > 4
        ){
            ok = false;
            break;
        }
    }

    ok = ok && is_token(TOKEN_ASSIGN);
    parse_i = prev_parse_i;
    return ok;
}

static u1 eat_semicolon(){
    if(!eat_token(TOKEN_SEMICOLON)){
        if(parse_i > 0){
            parse_i--;
        }

        printf("error on line %d: Expected ';' after statement\n", current_line());
        stop_parsing();
        return false;
    } else {
        return true;
    }
}

static u32 add_statement_else_print_error(Expression expression){
    u32 statement = add_statement_from_new(expression);
    if(statement >= STATEMENTS_CAPACITY) stop_parsing();
    return statement;
}

static Expression parse_expression();

static Expression parse_expression_print(){
    Expression expression = (Expression){
        .kind = EXPRESSION_PRINT,
        .ops = 0,
    };

    if(!is_token(TOKEN_STRING)){
        printf("error on line %d: Expected string after '(' in print statement\n", current_line());
        instead_got();
        stop_parsing();
        return expression;
    }

    expression.ops = eat_string();

    if(!eat_token(TOKEN_CLOSE)){
        printf("error on line %d: Expected ')' after string in print statement\n", current_line());
        instead_got();
        stop_parsing();
        return expression;
    }

    return expression;
}

static Expression parse_expression_call(u32 name){
    Expression expression = (Expression){
        .kind = EXPRESSION_CALL,
        .ops = 0,
    };

    /* Operands layout */
    /* { name, arity, arg1, arg2, ..., argN } */
    u32 ops = add_operand(name);
    u32 arity_location = add_operand(0);

    if(/* (redundant) operands >= OPERANDS_CAPACITY || */ arity_location >= OPERANDS_CAPACITY){
        stop_parsing();
        return expression;
    }

    while(!is_token(TOKEN_CLOSE)){
        Expression argument_expression = parse_expression();

        u32 argument = add_expression(argument_expression);
        if(argument >= EXPRESSIONS_CAPACITY){
            stop_parsing();
            return expression;
        }

        if(add_operand(argument) >= OPERANDS_CAPACITY){
            stop_parsing();
            return expression;
        }

        operands[arity_location]++;

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

    expression.ops = ops;
    return expression;
}

static Expression parse_secondary_expression(u8 precedence, Expression lhs);

static Expression parse_primary_expression(){
    Expression expression = (Expression){
        .kind = 0,
        .ops = 0,
    };

    if(is_token(TOKEN_WORD)){
        u32 name = eat_word();

        if(eat_token(TOKEN_OPEN)){
            if(aux_cstr_equals_print(name)){
                // Print call?
                return parse_expression_print();
            }

            // Regular call
            return parse_expression_call(name);
        }

        // Else, normal variable
        return (Expression){
            .kind = EXPRESSION_VARIABLE,
            .ops = name,
        };
    }

    if(is_token(TOKEN_INT)){
        return (Expression){
            .kind = EXPRESSION_INT,
            .ops = eat_int(),
        };
    }
    
    printf("error on line %d: Expected expression\n", current_line());
    stop_parsing();
    return expression;
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
    case TOKEN_ADD:
        return 2;
    case TOKEN_SUBTRACT:
        return 2;
    default:
        return 255;
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

    if(next_precedence < operator_precedence){
        rhs = parse_secondary_expression(next_precedence, rhs);
        if(had_parse_error) return rhs;
    }

    return rhs;
}

static Expression parse_math(Expression lhs, ExpressionKind expression_kind, u8 operator_precedence){
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

    return (Expression){
        .kind = expression_kind,
        .ops = ops,
    };
}

static Expression parse_secondary_expression(u8 precedence, Expression lhs){
    while(true){
        if(parse_i >= num_tokens){
            return lhs;
        }

        TokenKind operator = tokens[parse_i].kind;
        u8 operator_precedence = parse_get_precedence(operator);

        if(is_terminating_token(operator) || precedence < operator_precedence){
            return lhs;
        }

        switch(operator){
        case TOKEN_ADD:
            lhs = parse_math(lhs, EXPRESSION_ADD, operator_precedence);
            break;
        case TOKEN_SUBTRACT:
            lhs = parse_math(lhs, EXPRESSION_SUBTRACT, operator_precedence);
            break;
        }

        if(had_parse_error) return lhs;
    }

    return lhs;
}

static Expression parse_expression(){
    Expression primary = parse_primary_expression();
    if(had_parse_error) return primary;

    return parse_secondary_expression(255, primary);
}

static u32 parse_function_body(Function function){
    // { ... }
    //       ^ ending token index
    //   ^  starting token index

    while(parse_i < num_tokens && !is_token(TOKEN_END)){
        if(is_declaration()){
            Type type = parse_type();
            if(had_parse_error) return 1;

            u32 variable_type = add_type(type);
            if(variable_type >= TYPES_CAPACITY) return 1;

            if(!is_token(TOKEN_WORD)){
                printf("error on line %d: Expected variable name after type\n", current_line());
                instead_got();
                return 1;
            }

            u32 variable_name = eat_word();
            u32 ops = add_operands2(variable_type, variable_name);
            if(ops >= OPERANDS_CAPACITY){
                stop_parsing();
                return 1;
            }

            if(!eat_semicolon()){
                return 1;
            }

            Expression declaration = (Expression){
                .kind = EXPRESSION_DECLARE,
                .ops = ops,
            };

            if(add_statement_else_print_error(declaration) >= STATEMENTS_CAPACITY){
                return 1;
            }
        } else if(is_assignment()){
            u32 variable_name = eat_word();

            if(!eat_token(TOKEN_ASSIGN)){
                printf("error on line %d: Expected '=' during assignment\n", current_line());
                return 1;
            }

            Expression expression = parse_expression();
            if(had_parse_error) return 1;

            u32 new_value = add_expression(expression);
            if(new_value >= EXPRESSIONS_CAPACITY) return 1;

            u32 ops = add_operands2(variable_name, new_value);
            if(ops >= OPERANDS_CAPACITY) return 1;

            Expression assignment = (Expression){
                .kind = EXPRESSION_ASSIGN,
                .ops = ops,
            };

            if(
               !eat_semicolon()
            || add_statement_else_print_error(assignment) >= STATEMENTS_CAPACITY){
                return 1;
            }
        } else {
            Expression expression = parse_expression();
            if(had_parse_error) return 1;

            if(
               !eat_semicolon()
            || add_statement_else_print_error(expression) >= STATEMENTS_CAPACITY
            ){
                return 1;
            }
        }
    }

    return (u32) had_parse_error;
}

u32 parse(){
    if(add_builtin_types()) return 1;
    if(add_builtin_functions()) return 1;

    while(parse_i < num_tokens){
        // Parse type
        Type type = parse_type();
        if(had_parse_error) break;

        u32 symbol_type = add_type(type);
        if(symbol_type >= TYPES_CAPACITY) return 1;

        if(!is_token(TOKEN_WORD)){
            printf("error on line %d: Expected function name after type\n", current_line());
            instead_got();
            return 1;
        }

        u32 symbol_name = eat_word();

        if(eat_token(TOKEN_SEMICOLON)){
            // Is a global variable

            u32 global = add_global((Global){
                .name = symbol_name,
                .type = symbol_type,
            });
            if(global >= GLOBALS_CAPACITY) return 1;
            continue;
        }

        if(!eat_token(TOKEN_OPEN)){
            printf("error on line %d: Expected '(' after function name\n", current_line());
            instead_got();
            return 1;
        }

        u32 begin = num_statements;

        while(!is_token(TOKEN_CLOSE)){
            Type type = parse_type();
            if(had_parse_error) return 1;

            u32 parameter_type = add_type(type);
            if(parameter_type >= TYPES_CAPACITY) return 1;

            if(!is_token(TOKEN_WORD)){
                printf("error on line %d: Expected parameter name after parameter type\n", current_line());
                stop_parsing();
                return 1;
            }

            u32 parameter_name = eat_word();
            u32 ops = add_operands2(parameter_type, parameter_name);
            if(ops >= OPERANDS_CAPACITY) return 1;

            add_statement_from_new((Expression){
                .kind = EXPRESSION_DECLARE,
                .ops = ops,
            });

            if(!eat_token(TOKEN_NEXT)){
                if(!is_token(TOKEN_CLOSE)){
                    printf("error on line %d: Expected ',' or ')' after function parameter\n", current_line());
                    stop_parsing();
                    return 1;
                }
            }
        }

        parse_i++;

        if(!eat_token(TOKEN_BEGIN)){
            printf("error on line %d: Expected '{' after parameter list for function\n", current_line());
            instead_got();
            return 1;
        }

        Function function = {
            .name = symbol_name,
            .arity = num_statements - begin,
            .begin = begin,
            .num_stmts = 0,
            .return_type = symbol_type,
            .is_recursive = true, // assume recursive until we prove otherwise
        };

        if(parse_function_body(function)) break;
        function.num_stmts = num_statements - begin;

        if(add_function(function) >= FUNCTIONS_CAPACITY){
            return 1;
        }

        if(!eat_token(TOKEN_END)){
            printf("error on line %d: Expected '}' after function body\n", current_line());
            instead_got();
            return 1;
        }
    }

    return (u32) had_parse_error;
}

