
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
static ErrorCode parse_do_while();
static ErrorCode parse_return();
static ErrorCode parse_break();
static ErrorCode parse_continue();

ErrorCode parse_statement(){
    if(is_type_followed_by(TOKEN_WORD)){
        if(parse_declaration()) return 1;
    } else if(eat_token(TOKEN_IF)){
        if(parse_if()) return 1;
    } else if(eat_token(TOKEN_WHILE)){
        if(parse_while()) return 1;
    } else if(eat_token(TOKEN_DO)){
        if(parse_do_while()) return 1;
    } else if(eat_token(TOKEN_RETURN)){
        if(parse_return()) return 1;
    } else if(eat_token(TOKEN_BREAK)){
        if(parse_break()) return 1;
    } else if(eat_token(TOKEN_CONTINUE)){
        if(parse_continue()) return 1;
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

static u32 parse_block(){
    // Returns number of statements in block, otherwise STATEMENTS_CAPACITY if error

    u32 start = num_statements;
    
    // Traditional block
    if(eat_token(TOKEN_BEGIN)){
        while(parse_i < num_tokens && !is_token(TOKEN_END)){
            if(parse_statement()) return 1;
        }

        if(!eat_token(TOKEN_END)){
            printf("\nerror on line %d: Expected '}' to end block\n", current_line());
            stop_parsing();
            return STATEMENTS_CAPACITY;
        }
    } else {
        // Single statement block
        if(parse_statement()) return STATEMENTS_CAPACITY;
    }

    return num_statements - start;
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

    Expression statement = (Expression){
        .kind = EXPRESSION_IF,
        .line = line,
        .ops = 0,
    };

    u32 begin = add_statement_else_print_error(statement);
    if(begin >= STATEMENTS_CAPACITY) return 1;

    u32 num_then = parse_block();
    if(num_then >= STATEMENTS_CAPACITY) return 1;

    u32 num_else = 0;

    // Handle else
    if(eat_token(TOKEN_ELSE)){
        num_else = parse_block();
        if(num_else >= STATEMENTS_CAPACITY) return 1;
    }

    u32 ops;

    if(num_else == 0){
        // If statement
        ops = add_operands2(condition, num_then);
    } else {
        // If statement with else
        expressions[statements[begin]].kind = EXPRESSION_IF_ELSE;
        ops = add_operands3(condition, num_then, num_else);
    }

    if(ops >= OPERANDS_CAPACITY){
        stop_parsing();
        return 1;
    }

    expressions[statements[begin]].ops = ops;
    return 0;
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

    u32 ops = add_operands3(condition, 0, 0);
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
    if(begin >= STATEMENTS_CAPACITY) return 1;

    u32 num_inside = parse_block();
    if(num_inside >= STATEMENTS_CAPACITY) return 1;

    // Set number of statements
    operands[ops + 1] = num_inside;
    return 0;
}

static ErrorCode parse_do_while(){
    // do
    //    ^

    u24 line = tokens[parse_i - 1].line;

    Expression statement = (Expression){
        .kind = EXPRESSION_DO_WHILE,
        .line = line,
        .ops = 0,
    };

    u32 begin = add_statement_else_print_error(statement);
    if(begin >= STATEMENTS_CAPACITY) return 1;

    u32 num_inside = parse_block();
    if(num_inside >= STATEMENTS_CAPACITY) return 1;

    if(!eat_token(TOKEN_WHILE)){
        printf("\nerror on line %d: Expected 'while' after 'do-while' body\n", current_line());
        stop_parsing();
        return 1;
    }

    if(!eat_token(TOKEN_OPEN)){
        printf("\nerror on line %d: Expected '(' after 'while' keyword\n", current_line());
        stop_parsing();
        return 1;
    }

    Expression condition_expression = parse_expression();
    if(had_parse_error) return 1;

    if(!eat_token(TOKEN_CLOSE)){
        printf("\nerror on line %d: Expected ')' after 'do-while' condition\n", current_line());
        stop_parsing();
        return 1;
    }

    if(!eat_semicolon()){
        return 1;
    }

    u32 condition = add_expression(condition_expression);
    if(condition >= EXPRESSIONS_CAPACITY){
        stop_parsing();
        return 1;
    }

    u32 ops = add_operands3(condition, num_inside, 0);
    if(ops >= OPERANDS_CAPACITY){
        stop_parsing();
        return 1;
    }

    expressions[statements[begin]].ops = ops;
    return 0;
}

static ErrorCode parse_return(){
    // return 
    //         ^

    u24 line = tokens[parse_i - 1].line;

    Expression value_expression = parse_expression();
    if(had_parse_error) return 1;

    u32 value = add_expression(value_expression);
    if(value >= EXPRESSIONS_CAPACITY){
        stop_parsing();
        return 1;
    }

    Expression statement = (Expression){
        .kind = EXPRESSION_RETURN,
        .line = line,
        .ops = value,
    };

    return !eat_semicolon() || add_statement_else_print_error(statement) >= STATEMENTS_CAPACITY;
}

static ErrorCode parse_break(){
    // break
    //       ^

    Expression statement = (Expression){
        .kind = EXPRESSION_BREAK,
        .line = tokens[parse_i - 1].line,
        .ops = 0,
    };

    return !eat_semicolon() || add_statement_else_print_error(statement) >= STATEMENTS_CAPACITY;
}

static ErrorCode parse_continue(){
    // continue
    //          ^

    Expression statement = (Expression){
        .kind = EXPRESSION_CONTINUE,
        .line = tokens[parse_i - 1].line,
        .ops = 0,
    };

    return !eat_semicolon() || add_statement_else_print_error(statement) >= STATEMENTS_CAPACITY;
}

