
#include <stdio.h>
#include "../include/storage.h"
#include "../include/parse_statement.h"
#include "../include/parse_context.h"
#include "../include/parse_expression.h"
#include "../include/parse_type.h"
#include "../include/parse_dimensions.h"
#include "../include/expression_print.h"

static u1 eat_semicolon();
static u32 add_statement_else_print_error(Expression expression);
static ErrorCode parse_declaration_assignment(u32 variable_name, u24 line_number);
static ErrorCode parse_declaration();

ErrorCode parse_statement(){
    if(is_type_followed_by(TOKEN_WORD)){
        if(parse_declaration()) return 1;
    } else {
        Expression statement = parse_expression();
        if(had_parse_error) return 1;

        if(had_parse_error || !eat_semicolon() || add_statement_else_print_error(statement) >= STATEMENTS_CAPACITY){
            return 1;
        }
    }

    return 0;
}

static u1 eat_semicolon(){
    if(!eat_token(TOKEN_SEMICOLON)){
        u32 last_line, this_line;

        if(parse_i > 0){
            parse_i--;
            last_line = current_line();
            parse_i++;
            this_line = current_line();
        } else {
            last_line = current_line();
            this_line = last_line;
        }

        printf("error on line %d: Expected ';' after statement\n", last_line);

        if(this_line == last_line){
            instead_got();
        }

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

static ErrorCode parse_declaration_assignment(u32 variable_name, u24 line_number){
    if(!eat_token(TOKEN_ASSIGN)){
        printf("error on line %d: Expected '=' during assignment\n", current_line());
        stop_parsing();
        return 1;
    }

    Expression expression = parse_expression();
    if(had_parse_error) return 1;

    u32 new_value = add_expression(expression);
    if(new_value >= EXPRESSIONS_CAPACITY){
        stop_parsing();
        return 1;
    }

    u32 destination = add_expression((Expression){
        .kind = EXPRESSION_VARIABLE,
        .line = line_number,
        .ops = variable_name,
    });
    if(destination >= EXPRESSIONS_CAPACITY){
        stop_parsing();
        return 1;
    }

    u32 ops = add_operands2(destination, new_value);
    if(ops >= OPERANDS_CAPACITY){
        stop_parsing();
        return 1;
    }

    Expression statement = (Expression){
        .kind = EXPRESSION_ASSIGN,
        .line = line_number,
        .ops = ops,
    };

    return !eat_semicolon() || add_statement_else_print_error(statement) >= STATEMENTS_CAPACITY;
}

static ErrorCode parse_declaration(){
    u24 line_number = current_line_packed();

    Type type = parse_type();
    if(had_parse_error) return 1;

    if(!is_token(TOKEN_WORD)){
        printf("error on line %d: Expected variable name after type\n", current_line());
        instead_got();
        return 1;
    }

    u32 variable_name = eat_word();

    if(is_token(TOKEN_OPEN_BRACKET)){
        type.dimensions = parse_dimensions(dimensions[type.dimensions]);
        if(type.dimensions >= UNIQUE_DIMENSIONS_CAPACITY) return 1;
    }
    
    u32 variable_type = add_type(type);
    if(variable_type >= TYPES_CAPACITY){
        stop_parsing();
        return 1;
    }

    u32 ops = add_operands2(variable_type, variable_name);
    if(ops >= OPERANDS_CAPACITY){
        stop_parsing();
        return 1;
    }

    Expression statement = (Expression){
        .kind = EXPRESSION_DECLARE,
        .line = line_number,
        .ops = ops,
    };

    if(is_token(TOKEN_ASSIGN)){
        return add_statement_else_print_error(statement) >= STATEMENTS_CAPACITY || parse_declaration_assignment(variable_name, line_number);
    } else {
        return !eat_semicolon() || add_statement_else_print_error(statement) >= STATEMENTS_CAPACITY;
    }
}

