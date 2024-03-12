
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
#include "../include/parse_context.h"
#include "../include/parse_dimensions.h"
#include "../include/parse_statement.h"
#include "../include/parse_type.h"
#include "../include/typedef.h"
#include "../include/parse_typedef.h"
#include "../include/parse_macro.h"
#include "../include/parse_expression.h"

static u32 parse_function_body(Function function){
    // { ... }
    //       ^ ending token index
    //   ^  starting token index

    while(parse_i < num_tokens && !is_token(TOKEN_END)){
        if(parse_statement() != 0) return 1;
    }

    return (u32) had_parse_error;
}

ErrorCode parse_function(u32 symbol_name, u32 symbol_type, u24 line_number){
    if(!eat_token(TOKEN_OPEN)){
        printf("error on line %d: Expected '(' after function name\n", current_line());
        instead_got();
        return 1;
    }

    u32 begin = num_statements;

    while(!is_token(TOKEN_CLOSE)){
        u24 line_number = current_line_packed();

        Type type = parse_type();
        if(had_parse_error) return 1;

        if(!is_token(TOKEN_WORD)){
            printf("error on line %d: Expected parameter name after parameter type\n", current_line());
            stop_parsing();
            return 1;
        }

        u32 parameter_name = eat_word();

        if(is_token(TOKEN_OPEN_BRACKET)){
            type.dimensions = parse_dimensions(dimensions[type.dimensions]);
            if(type.dimensions >= UNIQUE_DIMENSIONS_CAPACITY) return 1;
        }

        u32 parameter_type = add_type(type);
        if(parameter_type >= TYPES_CAPACITY) return 1;

        u32 ops = add_operands2(parameter_type, parameter_name);
        if(ops >= OPERANDS_CAPACITY) return 1;

        if(add_statement_from_new((Expression){
            .kind = EXPRESSION_DECLARE,
            .line = line_number,
            .ops = ops,
        }) >= STATEMENTS_CAPACITY){
            stop_parsing();
            return 1;
        }

        if(!eat_token(TOKEN_NEXT)){
            if(!is_token(TOKEN_CLOSE)){
                printf("error on line %d: Expected ',' or ')' after function parameter\n", current_line());
                stop_parsing();
                return 1;
            }
        }
    }

    parse_i++;

    if(eat_token(TOKEN_SEMICOLON)){
        return 0;
    }

    if(!eat_token(TOKEN_BEGIN)){
        printf("error on line %d: Expected '{' after parameter list for function\n", current_line());
        instead_got();
        return 1;
    }

    Function function = (Function){
        .name = symbol_name,
        .arity = (u8) (num_statements - begin),
        .begin = begin,
        .num_stmts = (u32) 0,
        .return_type = symbol_type,
        .is_recursive = true, // assume recursive until we prove otherwise
        .line = line_number,
    };

    if(parse_function_body(function) != 0){
        return 1;
    }

    function.num_stmts = num_statements - begin;

    if(add_function(function) >= FUNCTIONS_CAPACITY){
        return 1;
    }

    if(!eat_token(TOKEN_END)){
        printf("error on line %d: Expected '}' after function body\n", current_line());
        instead_got();
        return 1;
    }

    return 0;
}

u32 parse(){
    parse_i = 0;
    had_parse_error = false;
    parse_trailing_semicolon = true;
    
    if(add_builtin_types() != 0) return 1;
    if(add_builtin_functions() != 0) return 1;

    while(parse_i < num_tokens){
        u24 line_number = tokens[parse_i].line;

        if(eat_token(TOKEN_TYPEDEF)){
            if(parse_typedef() != 0) return 1;
            continue;
        }

        if(eat_token(TOKEN_HASH)){
            if(parse_macro() != 0) return 1;
            continue;
        }

        // Parse type
        Type type = parse_type();
        if(had_parse_error) return 1;

        if(!is_token(TOKEN_WORD)){
            printf("error on line %d: Expected function name after type\n", current_line());
            instead_got();
            return 1;
        }

        u32 symbol_name = eat_word();

        // Global variable
        if(is_token(TOKEN_SEMICOLON) || is_token(TOKEN_OPEN_BRACKET) || is_token(TOKEN_ASSIGN)){
            type.dimensions = parse_dimensions(dimensions[type.dimensions]);
            if(type.dimensions >= UNIQUE_DIMENSIONS_CAPACITY) return 1;

            u32 symbol_type = add_type(type);
            if(symbol_type >= TYPES_CAPACITY) return 1;

            u32 initializer = EXPRESSIONS_CAPACITY;

            if(eat_token(TOKEN_ASSIGN)){
                Expression initial = parse_expression();
                if(had_parse_error) return 1;

                initializer = add_expression(initial);
                if(initializer >= EXPRESSIONS_CAPACITY) return 1;
            }

            if(!eat_token(TOKEN_SEMICOLON)){
                printf("error on line %d: Expected ';' after global variable\n", current_line());
                instead_got();
                return 1;
            }

            u32 global = add_global((Global){
                .name = symbol_name,
                .type = symbol_type,
                .line = line_number,
                .initializer = initializer,
            });
            if(global >= GLOBALS_CAPACITY) return 1;
        } else {
            u32 symbol_type = add_type(type);
            if(symbol_type >= TYPES_CAPACITY) return 1;

            // Function
            if(parse_function(symbol_name, symbol_type, line_number) != 0){
                return 1;
            }
        }
    }

    return (ErrorCode) had_parse_error;
}

