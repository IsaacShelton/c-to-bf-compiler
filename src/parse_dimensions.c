
#include <stdio.h>
#include <string.h>
#include "../include/parse_context.h"
#include "../include/storage.h"

u32 parse_add_dimensions(u32 type_dimensions[4]){
    u32 dims = add_dimensions(type_dimensions);

    if(dims >= UNIQUE_DIMENSIONS_CAPACITY){
        printf("\nerror on line %d: Maximum number of unique dimension pairs exceeded. Use fewer or reconfigure your compiler.\n", current_line());
        stop_parsing();
        return UNIQUE_DIMENSIONS_CAPACITY;
    }
    
    return dims;
}

u32 parse_dimensions(u32 start_type_dimensions[4]){
    // Type dimensions is {0, 0, 0, 0} to start,
    // Unless some dimensions for the type already exist.

    u32 type_dimensions[4];
    memcpy(type_dimensions, start_type_dimensions, sizeof(u32) * 4);

    u32 next_dim = 0;

    // Find next dimension for type dimensions
    while(next_dim < UNIQUE_DIMENSIONS_CAPACITY && type_dimensions[next_dim] != 0){
        next_dim++;
    }

    while(eat_token(TOKEN_OPEN_BRACKET)){
        if(next_dim == 4){
            printf("\nerror on line %d: Cannot nest array dimensions more than 4 deep\n", current_line());
            stop_parsing();
            return UNIQUE_DIMENSIONS_CAPACITY;
        }

        u32 dim;

        if(is_token(TOKEN_INT)){
            dim = eat_int();
        } else if(is_token(TOKEN_WORD)){
            u32 definition_name = eat_word();

            u32 resolved = try_resolve_define(definition_name);
            if(resolved >= EXPRESSIONS_CAPACITY){
                printf("\nerror on line %d: Undeclared compile-time definition '", current_line());
                print_aux_cstr(definition_name);
                printf("'\n");
                stop_parsing();
                return UNIQUE_DIMENSIONS_CAPACITY;
            }

            Expression expression = expressions[resolved];
            if(expression.kind != EXPRESSION_INT){
                printf("\nerror on line %d: Cannot use non-integer from compile-time definition '", current_line());
                print_aux_cstr(definition_name);
                printf("' as type dimension\n");
                stop_parsing();
                return UNIQUE_DIMENSIONS_CAPACITY;
            }

            dim = expression.ops;
        } else {
            printf("\nerror on line %d: Expected number for dimension of type\n", current_line());
            instead_got();
            stop_parsing();
            return UNIQUE_DIMENSIONS_CAPACITY;
        }

        if(dim == 0){
            printf("\nerror on line %d: Cannot have array dimension of 0\n", current_line());
            stop_parsing();
            return UNIQUE_DIMENSIONS_CAPACITY;
        }

        type_dimensions[next_dim++] = dim;

        if(!eat_token(TOKEN_CLOSE_BRACKET)){
            printf("\nerror on line %d: Expected ']' after dimension in type\n", current_line());
            instead_got();
            stop_parsing();
            return UNIQUE_DIMENSIONS_CAPACITY;
        }
    }

    if(next_dim == 0){
        return 0;
    }

    return parse_add_dimensions(type_dimensions);
}

