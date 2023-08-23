
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
static ErrorCode parse_if();
static ErrorCode parse_while();

ErrorCode parse_statement(){
    if(is_type_followed_by(TOKEN_WORD)){
        if(parse_declaration()) return 1;
    } else if(eat_token(TOKEN_IF)){
        if(parse_if()) return 1;
    } else if(eat_token(TOKEN_WHILE)){
        if(parse_while()) return 1;
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

static ErrorCode parse_while(){
    // while
    //       ^

    u24 line = tokens[parse_i - 1].line;

    if(!eat_token(TOKEN_OPEN)){
        printf("\nerror on line %d: Expected '(' after 'while' keyword\n", current_line());
        stop_parsing();
        return 1;
    }

    Expression condition_expression = parse_expression();
    if(had_parse_error) return 1;

    if(!eat_token(TOKEN_CLOSE)){
        printf("\nerror on line %d: Expected ')' after 'while' condition\n", current_line());
        stop_parsing();
        return 1;
    }

    u32 condition = add_expression(condition_expression);
    if(condition >= EXPRESSIONS_CAPACITY){
        stop_parsing();
        return 1;
    }

    u32 ops = add_operands2(condition, 0);
    if(ops >= OPERANDS_CAPACITY){
        stop_parsing();
        return 1;
    }

    Expression statement = (Expression){
        .kind = EXPRESSION_WHILE,
        .line = line,
        .ops = ops,
    };

    u32 begin = add_statement_else_print_error(statement);

    if(begin >= STATEMENTS_CAPACITY){
        return 1;
    }

    if(!eat_token(TOKEN_BEGIN)){
        printf("\nerror on line %d: Expected '{' after 'while' condition\n", current_line());
        stop_parsing();
        return 1;
    }

    while(parse_i < num_tokens && !is_token(TOKEN_END)){
        if(parse_statement()) return 1;
    }

    // Set number of statements
    operands[ops + 1] = num_statements - 1 - begin;

    if(!eat_token(TOKEN_END)){
        printf("\nerror on line %d: Expected '}' after 'while' body\n", current_line());
        stop_parsing();
        return 1;
    }

    return 0;
}

static ErrorCode parse_if(){
    // if
    //    ^

    u24 line = tokens[parse_i - 1].line;

    if(!eat_token(TOKEN_OPEN)){
        printf("\nerror on line %d: Expected '(' after 'if' keyword\n", current_line());
        stop_parsing();
        return 1;
    }

    Expression condition_expression = parse_expression();
    if(had_parse_error) return 1;

    if(!eat_token(TOKEN_CLOSE)){
        printf("\nerror on line %d: Expected ')' after 'if' condition\n", current_line());
        stop_parsing();
        return 1;
    }

    u32 condition = add_expression(condition_expression);
    if(condition >= EXPRESSIONS_CAPACITY){
        stop_parsing();
        return 1;
    }

    u32 parts[3] = { condition, 0, 0 };

    Expression statement = (Expression){
        .kind = EXPRESSION_IF,
        .line = line,
        .ops = 0,
    };

    u32 begin = add_statement_else_print_error(statement);

    if(begin >= STATEMENTS_CAPACITY){
        return 1;
    }

    if(!eat_token(TOKEN_BEGIN)){
        printf("\nerror on line %d: Expected '{' after 'if' condition\n", current_line());
        stop_parsing();
        return 1;
    }

    while(parse_i < num_tokens && !is_token(TOKEN_END)){
        if(parse_statement()) return 1;
    }

    // Set number of statements for 'if'
    parts[1] = num_statements - 1 - begin;

    if(!eat_token(TOKEN_END)){
        printf("\nerror on line %d: Expected '}' after 'if' body\n", current_line());
        stop_parsing();
        return 1;
    }

    // Handle else
    if(eat_token(TOKEN_ELSE)){
        if(!eat_token(TOKEN_BEGIN)){
            printf("\nerror on line %d: Expected '{' after 'else'\n", current_line());
            stop_parsing();
            return 1;
        }

        while(parse_i < num_tokens && !is_token(TOKEN_END)){
            if(parse_statement()) return 1;
        }

        // Set number of statements for 'else'
        parts[2] = num_statements - 1 - (begin + parts[1]);

        if(!eat_token(TOKEN_END)){
            printf("\nerror on line %d: Expected '}' after 'else' body\n", current_line());
            stop_parsing();
            return 1;
        }
    }

    u32 ops;

    if(parts[2] == 0){
        // If statement
        ops = add_operands2(parts[0], parts[1]);
    } else {
        // If statement with else
        expressions[statements[begin]].kind = EXPRESSION_IF_ELSE;
        ops = add_operands3(parts[0], parts[1], parts[2]);
    }

    if(ops >= OPERANDS_CAPACITY){
        stop_parsing();
        return 1;
    }

    expressions[statements[begin]].ops = ops;
    return 0;
}

