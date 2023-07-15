
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include "../include/parse.h"
#include "../include/storage.h"
#include "../include/token_print.h"
#include "../include/type_print.h"
#include "../include/global_print.h"

int parse_i = 0;
bool had_parse_error = false;

static void instead_got(){
    printf("  Instead got: `");
    token_print(tokens[parse_i]);
    printf("`\n");
}

static void stop_parsing(){
    had_parse_error = true;
    parse_i = num_tokens;
}

static bool is_token(TokenKind kind){
    return tokens[parse_i].kind == kind;
}

static int current_line(){
    return tokens[parse_i].line;
}

static int eat_word(){
    // Expects `is_token(TOKEN_WORD)` to be true
    return tokens[parse_i++].data;
}

static int eat_int(){
    // Expects `is_token(TOKEN_INT)` to be true
    return tokens[parse_i++].data;
}

static bool eat_token(TokenKind kind){
    if(tokens[parse_i].kind == kind){
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
    unsigned int type_dimensions[4] = {0, 0, 0, 0};
    int next_dim = 0;

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

        int dim = eat_int();

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
        for(int i = 0; i < UNIQUE_DIMENSIONS_CAPACITY; i++){
            bool match = memcmp(dimensions[i], type_dimensions, sizeof(unsigned int[4]));

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

            for(int i = 0; i < 4; i++){
                dimensions[num_dimensions][i] = type_dimensions[i];
            }

            type.dimensions = num_dimensions++;
        }
    }

    return type;
}

int parse(){
    while(parse_i < num_tokens){
        // Parse type
        Type type = parse_type();
        if(had_parse_error) break;

        if(!is_token(TOKEN_WORD)){
            printf("error on line %d: Expected function name after type\n", current_line());
            instead_got();
            break;
        }

        int symbol_name = eat_word();

        if(eat_token(TOKEN_SEMICOLON)){
            // Is a global variable

            int global_type = add_type(type);
            if(global_type >= GLOBALS_CAPACITY){
                printf("Out of memory: Exceeded maximum number of global variables\n");
                return 1;
            }

            add_global((Global){
                .name = symbol_name,
                .type = global_type,
            });
            continue;
        }

        if(!eat_token(TOKEN_OPEN)){
            printf("error on line %d: Expected '(' after function name\n", current_line());
            instead_got();
            break;
        }

        while(!is_token(TOKEN_CLOSE)){
            printf("error on line %d: args parsing not implemented yet\n", current_line());
            stop_parsing();
            break;
        }

        parse_i++;

        if(!eat_token(TOKEN_BEGIN)){
            printf("error on line %d: Expected '{' after parameter list for function\n", current_line());
            instead_got();
            break;
        }

        if(!eat_token(TOKEN_END)){
            printf("error on line %d: Expected '}' after function body\n", current_line());
            instead_got();
            break;
        }
    }

    for(int i = 0; i < num_globals; i++){
        global_print(globals[i]);
        printf("\n");
    }

    return (int) had_parse_error;
}

