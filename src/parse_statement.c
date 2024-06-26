
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
static ErrorCode parse_if();
static ErrorCode parse_while();
static ErrorCode parse_do_while();
static ErrorCode parse_for();
static ErrorCode parse_conditionless_block();
static ErrorCode parse_return();
static ErrorCode parse_break();
static ErrorCode parse_continue();
static ErrorCode parse_switch();
static ErrorCode parse_case();
static ErrorCode parse_default();

ErrorCode parse_statement(){
    if(is_type_followed_by(TOKEN_WORD)){
        if(parse_declaration(true) != 0) return 1;
    } else if(eat_token(TOKEN_IF)){
        if(parse_if() != 0) return 1;
    } else if(eat_token(TOKEN_WHILE)){
        if(parse_while() != 0) return 1;
    } else if(eat_token(TOKEN_DO)){
        if(parse_do_while() != 0) return 1;
    } else if(eat_token(TOKEN_FOR)){
        if(parse_for() != 0) return 1;
    } else if(eat_token(TOKEN_RETURN)){
        if(parse_return() != 0) return 1;
    } else if(eat_token(TOKEN_BREAK)){
        if(parse_break() != 0) return 1;
    } else if(eat_token(TOKEN_CONTINUE)){
        if(parse_continue() != 0) return 1;
    } else if(eat_token(TOKEN_SWITCH)){
        if(parse_switch() != 0) return 1;
    } else if(eat_token(TOKEN_CASE)){
        if(parse_case() != 0) return 1;
    } else if(eat_token(TOKEN_DEFAULT)){
        if(parse_default() != 0) return 1;
    } else if(is_token(TOKEN_BEGIN)){
        if(parse_conditionless_block() != 0) return 1;
    } else {
        Expression statement = parse_expression();
        if(had_parse_error) return 1;

        // Transform increments and decrements to expression with no result
        if(statement.kind == EXPRESSION_POST_INCREMENT || statement.kind == EXPRESSION_PRE_INCREMENT){
            statement.kind = EXPRESSION_NO_RESULT_INCREMENT;
        } else if(statement.kind == EXPRESSION_POST_DECREMENT || statement.kind == EXPRESSION_PRE_DECREMENT){
            statement.kind = EXPRESSION_NO_RESULT_DECREMENT;
        }

        if(had_parse_error || !eat_semicolon() || add_statement_else_print_error(statement) >= STATEMENTS_CAPACITY){
            return 1;
        }
    }

    return 0;
}

static u1 eat_semicolon(){
    if(parse_trailing_semicolon && !eat_token(TOKEN_SEMICOLON)){
        u32 last_line;
        u32 this_line;

        if(parse_i > 0){
            parse_i--;
            last_line = current_line();
            parse_i++;
            this_line = current_line();
        } else {
            last_line = current_line();
            this_line = last_line;
        }


        if(this_line == last_line){
            printf("error on line %d: Expected ';' after statement\n", this_line);
            instead_got();
        } else {
            printf("error on line %d: Expected ';' after statement\n", last_line);
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

    return (ErrorCode) (!eat_semicolon() || add_statement_else_print_error(statement) >= STATEMENTS_CAPACITY);
}

ErrorCode parse_declaration(u1 allow_assignment){
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

    if(allow_assignment && is_token(TOKEN_ASSIGN)){
        return (ErrorCode) (add_statement_else_print_error(statement) >= STATEMENTS_CAPACITY || parse_declaration_assignment(variable_name, line_number));
    } else {
        return (ErrorCode) (!eat_semicolon() || add_statement_else_print_error(statement) >= STATEMENTS_CAPACITY);
    }
}

static u32 parse_block(u1 allow_cases){
    // Returns number of statements in block, otherwise STATEMENTS_CAPACITY if error

    u32 start = num_statements;
    u1 previous_parse_trailing_semicolon = parse_trailing_semicolon;
    parse_trailing_semicolon = true;

    u32 last_case = STATEMENTS_CAPACITY;
    
    // Traditional block
    if(eat_token(TOKEN_BEGIN)){
        while(parse_i < num_tokens && !is_token(TOKEN_END)){
            if(parse_statement() != 0) return STATEMENTS_CAPACITY;

            Expression expression = expressions[statements[num_statements - 1]];

            if(expression.kind == EXPRESSION_CASE){
                if(allow_cases){
                    if(last_case < STATEMENTS_CAPACITY){
                        // Set number of statements for last 'case' statement
                        operands[expressions[statements[last_case]].ops + 1] = num_statements - last_case - 2;
                    }

                    last_case = num_statements - 1;
                } else {
                    expression_print_cannot_use_case_here_error(expression);
                    stop_parsing();
                    return STATEMENTS_CAPACITY;
                }
            }
        }

        if(last_case < STATEMENTS_CAPACITY){
            // Set number of statements for last 'case' statement
            // NOTE: We only have to subtract 1, since we don't create a final 'case' statement
            operands[expressions[statements[last_case]].ops + 1] = num_statements - last_case - 1;
        }

        if(!eat_token(TOKEN_END)){
            printf("\nerror on line %d: Expected '}' to end block\n", current_line());
            stop_parsing();
            return STATEMENTS_CAPACITY;
        }
    } else {
        if(allow_cases){
            printf("\nerror on line %d: Expected '{' after value of 'switch' statement\n", current_line());
            stop_parsing();
            return STATEMENTS_CAPACITY;
        }

        // Single statement block
        if(parse_statement() != 0) return STATEMENTS_CAPACITY;
    }

    parse_trailing_semicolon = previous_parse_trailing_semicolon;
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
        .ops = (u32) 0,
    };

    u32 begin = add_statement_else_print_error(statement);
    if(begin >= STATEMENTS_CAPACITY) return 1;

    u32 num_then = parse_block(false);
    if(num_then >= STATEMENTS_CAPACITY) return 1;

    u32 num_else = 0;

    // Handle else
    if(eat_token(TOKEN_ELSE)){
        num_else = parse_block(false);
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

    u32 num_inside = parse_block(false);
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
        .ops = (u32) 0,
    };

    u32 begin = add_statement_else_print_error(statement);
    if(begin >= STATEMENTS_CAPACITY) return 1;

    u32 num_inside = parse_block(false);
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

static ErrorCode parse_for(){
    // for
    //     ^

    u24 line = tokens[parse_i - 1].line;

    u32 ops = add_operands5(0, 0, 0, 0, 0);
    if(ops >= OPERANDS_CAPACITY){
        stop_parsing();
        return 1;
    }

    Expression statement = (Expression){
        .kind = EXPRESSION_FOR,
        .line = line,
        .ops = ops,
    };

    u32 begin = add_statement_else_print_error(statement);
    if(begin >= STATEMENTS_CAPACITY) return 1;

    if(!eat_token(TOKEN_OPEN)){
        printf("\nerror on line %d: Expected '(' after 'for' keyword\n", current_line());
        stop_parsing();
        return 1;
    }

    if(!eat_token(TOKEN_SEMICOLON)){
        if(parse_statement() != 0) return 1;

        // Set number of pre-statements to be 1
        operands[ops] = num_statements - begin - 1;
    }

    Expression condition_expression;

    if(eat_token(TOKEN_SEMICOLON)){
        condition_expression = (Expression){
            .kind = EXPRESSION_U1,
            .line = current_line_packed(),
            .ops = (u32) 1,
        };
    } else {
        condition_expression = parse_expression();
        if(had_parse_error) return 1;

        if(!eat_token(TOKEN_SEMICOLON)){
            printf("\nerror on line %d: Expected ';' after condition in 'for' statement\n", current_line());
            stop_parsing();
            return 1;
        }
    }

    u32 condition = add_expression(condition_expression);
    if(condition >= EXPRESSIONS_CAPACITY){
        stop_parsing();
        return 1;
    }

    operands[ops + 1] = condition;

    if(!eat_token(TOKEN_SEMICOLON)){
        // Disable trailing semicolon parsing
        u1 previous_parse_trailing_semicolon = parse_trailing_semicolon;
        parse_trailing_semicolon = false;

        if(parse_statement() != 0) return 1;

        // Restore previous trailing semicolon parsing setting
        parse_trailing_semicolon = previous_parse_trailing_semicolon;

        // Set number of post-statements to be 1
        operands[ops + 2] = num_statements - begin - 1 - operands[ops];
    }

    if(!eat_token(TOKEN_CLOSE)){
        printf("\nerror on line %d: Expected ')' after 'for' condition\n", current_line());
        stop_parsing();
        return 1;
    }

    u32 num_inside = parse_block(false);
    if(num_inside >= STATEMENTS_CAPACITY) return 1;

    // Set number of statements
    operands[ops + 3] = num_inside;
    return 0;
}

static ErrorCode parse_conditionless_block(){
    // {
    // ^

    Expression statement = (Expression){
        .kind = EXPRESSION_CONDITIONLESS_BLOCK,
        .line = current_line_packed(),
        .ops = (u32) 0,
    };

    u32 block = add_statement_else_print_error(statement);
    if(block >= STATEMENTS_CAPACITY){
        return 1;
    }

    u32 length = parse_block(false);

    if(length >= STATEMENTS_CAPACITY){
        return 1;
    }

    expressions[statements[block]].ops = length;
    return 0;
}

static ErrorCode parse_return(){
    // return 
    //         ^

    u24 line = tokens[parse_i - 1].line;
    u32 value = EXPRESSIONS_CAPACITY;

    // Parse return value if not followed by a ';'
    if(!is_token(TOKEN_SEMICOLON)){
        Expression value_expression = parse_expression();
        if(had_parse_error) return 1;

        value = add_expression(value_expression);
        if(value >= EXPRESSIONS_CAPACITY){
            stop_parsing();
            return 1;
        }
    }

    Expression statement = (Expression){
        .kind = EXPRESSION_RETURN,
        .line = line,
        .ops = value,
    };

    return (ErrorCode) (!eat_semicolon() || add_statement_else_print_error(statement) >= STATEMENTS_CAPACITY);
}

static ErrorCode parse_break(){
    // break
    //       ^

    Expression statement = (Expression){
        .kind = EXPRESSION_BREAK,
        .line = tokens[parse_i - 1].line,
        .ops = (u32) 0,
    };

    return (ErrorCode) (!eat_semicolon() || add_statement_else_print_error(statement) >= STATEMENTS_CAPACITY);
}

static ErrorCode parse_continue(){
    // continue
    //          ^

    Expression statement = (Expression){
        .kind = EXPRESSION_CONTINUE,
        .line = tokens[parse_i - 1].line,
        .ops = (u32) 0,
    };

    return (ErrorCode) (!eat_semicolon() || add_statement_else_print_error(statement) >= STATEMENTS_CAPACITY);
}

static ErrorCode parse_switch(){
    // switch
    //        ^

    u24 line = tokens[parse_i - 1].line;

    if(!eat_token(TOKEN_OPEN)){
        printf("\nerror on line %d: Expected '(' after 'switch' keyword\n", current_line());
        stop_parsing();
        return 1;
    }

    Expression condition_expression = parse_expression();
    if(had_parse_error) return 1;

    if(!eat_token(TOKEN_CLOSE)){
        printf("\nerror on line %d: Expected ')' after 'switch' condition\n", current_line());
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
        .kind = EXPRESSION_SWITCH,
        .line = line,
        .ops = ops,
    };

    u32 begin = add_statement_else_print_error(statement);
    if(begin >= STATEMENTS_CAPACITY) return 1;

    u32 num_inside = parse_block(true);
    if(num_inside >= STATEMENTS_CAPACITY) return 1;

    // Set number of statements
    operands[ops + 1] = num_inside;
    return 0;
}

static ErrorCode parse_case(){
    // case 
    //      ^

    u24 line = tokens[parse_i - 1].line;

    Expression value_expression = parse_expression();
    if(had_parse_error) return 1;

    u32 value = add_expression(value_expression);
    if(value >= EXPRESSIONS_CAPACITY){
        stop_parsing();
        return 1;
    }

    u32 ops = add_operands2(value, 0);
    if(ops >= OPERANDS_CAPACITY){
        stop_parsing();
        return 1;
    }

    Expression statement = (Expression){
        .kind = EXPRESSION_CASE,
        .line = line,
        .ops = ops,
    };

    if(!eat_token(TOKEN_COLON)){
        printf("error on line %d: Expected ':' after 'case' value\n", current_line());
        stop_parsing();
        return 1;
    }

    return (ErrorCode) (add_statement_else_print_error(statement) >= STATEMENTS_CAPACITY);
}

static ErrorCode parse_default(){
    // default 
    //         ^

    u24 line = tokens[parse_i - 1].line;

    u32 ops = add_operands2(EXPRESSIONS_CAPACITY, 0);
    if(ops >= OPERANDS_CAPACITY){
        stop_parsing();
        return 1;
    }

    // NOTE: 'default' is just a 'case' with an out of bounds expression id
    Expression statement = (Expression){
        .kind = EXPRESSION_CASE,
        .line = line,
        .ops = ops,
    };

    if(!eat_token(TOKEN_COLON)){
        printf("error on line %d: Expected ':' after 'default' value\n", current_line());
        stop_parsing();
        return 1;
    }

    return (ErrorCode) (add_statement_else_print_error(statement) >= STATEMENTS_CAPACITY);
}

