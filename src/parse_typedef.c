
#include <stdio.h>
#include "../include/storage.h"
#include "../include/parse_typedef.h"
#include "../include/parse_context.h"
#include "../include/parse_statement.h"

static ErrorCode parse_typedef_struct(u24 line_number){
    // typedef struct {
    //                ^

    if(!eat_token(TOKEN_BEGIN)){
        printf("error on line %d: Expected '{' after struct keyword in type definition\n", current_line());
        stop_parsing();
        return 1;
    }

    u32 begin = num_statements;

    while(parse_i < num_tokens && !is_token(TOKEN_END)){
        if(parse_statement()) return 1;
    }

    if(!eat_token(TOKEN_END)){
        printf("error on line %d: Expected '}' after function body\n", current_line());
        instead_got();
        stop_parsing();
        return 1;
    }

    if(!is_token(TOKEN_WORD)){
        printf("error on line %d: Expected typedef name after '}'\n", current_line());
        instead_got();
        stop_parsing();
        return 1;
    }

    u32 name = eat_word();

    if(!eat_token(TOKEN_SEMICOLON)){
        printf("error on line %d: Expected ';' after typedef name\n", current_line());
        instead_got();
        stop_parsing();
        return 1;
    }

    u32 def = add_typedef((TypeDef){
        .kind = TYPEDEF_STRUCT,
        .line = line_number,
        .name = name,
        .begin = begin,
        .num_fields = num_statements - begin,
        .computed_size = -1,
    });

    return def >= TYPEDEFS_CAPACITY;
}

static ErrorCode parse_typedef_enum(u24 line_number){
    // typedef enum {
    //              ^

    if(!eat_token(TOKEN_BEGIN)){
        printf("error on line %d: Expected '{' after struct keyword in type definition\n", current_line());
        stop_parsing();
        return 1;
    }

    u32 begin = num_statements;

    while(parse_i < num_tokens && !is_token(TOKEN_END)){
        if(!is_token(TOKEN_WORD)){
            printf("error on line %d: Expected variant inside of enum\n", current_line());
            stop_parsing();
            return 1;
        }

        u24 variant_line_number = current_line_packed();
        u32 word = eat_word();

        u32 variant = add_statement_from_new((Expression){
            .kind = EXPRESSION_ENUM_VARIANT,
            .ops = word,
            .line = variant_line_number,
        });

        if(variant >= STATEMENTS_CAPACITY){
            stop_parsing();
            return 1;
        }

        // Optionally eat ','
        eat_token(TOKEN_NEXT);
    }

    if(!eat_token(TOKEN_END)){
        printf("error on line %d: Expected '}' after function body\n", current_line());
        instead_got();
        stop_parsing();
        return 1;
    }

    if(!is_token(TOKEN_WORD)){
        printf("error on line %d: Expected typedef name after '}'\n", current_line());
        instead_got();
        stop_parsing();
        return 1;
    }

    u32 name = eat_word();

    if(!eat_token(TOKEN_SEMICOLON)){
        printf("error on line %d: Expected ';' after typedef name\n", current_line());
        instead_got();
        stop_parsing();
        return 1;
    }

    u32 count = num_statements - begin;
    u32 size = -1;

    if(count <= 0xFF){
        size = 1;
    } else if(count <= 0xFFFF){
        size = 2;
    } else if(count <= 0xFFFFFF){
        size = 3;
    } else {
        size = 4;
    }

    u32 def = add_typedef((TypeDef){
        .kind = TYPEDEF_ENUM,
        .line = line_number,
        .name = name,
        .begin = begin,
        .num_fields = count,
        .computed_size = size,
    });

    return def >= TYPEDEFS_CAPACITY;
}

ErrorCode parse_typedef(){
    // typedef 
    //         ^

    u24 line_number = tokens[parse_i - 1].line;

    if(eat_token(TOKEN_STRUCT)){
        return parse_typedef_struct(line_number);
    }

    if(eat_token(TOKEN_ENUM)){
        return parse_typedef_enum(line_number);
    }

    printf("error on line %d: Alias typdefs are not supported yet\n", current_line());
    stop_parsing();
    return 1;
}

