
#include <stdio.h>
#include <string.h>
#include "../include/parse_context.h"
#include "../include/storage.h"

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
            printf("error on line %d: Cannot nest array dimensions more than 4 deep\n", current_line());
            stop_parsing();
            return UNIQUE_DIMENSIONS_CAPACITY;
        }

        if(!is_token(TOKEN_INT)){
            printf("error on line %d: Expected number for dimension of type\n", current_line());
            instead_got();
            stop_parsing();
            return UNIQUE_DIMENSIONS_CAPACITY;
        }

        u32 dim = eat_int();

        if(dim == 0){
            printf("error on line %d: Cannot have array dimension of 0\n", current_line());
            stop_parsing();
            return UNIQUE_DIMENSIONS_CAPACITY;
        }

        type_dimensions[next_dim++] = dim;

        if(!eat_token(TOKEN_CLOSE_BRACKET)){
            printf("error on line %d: Expected ']' after dimension in type\n", current_line());
            instead_got();
            stop_parsing();
            return UNIQUE_DIMENSIONS_CAPACITY;
        }
    }

    if(next_dim == 0){
        return 0;
    }

    // Try to find existing slot with same value
    for(u32 i = 0; i < UNIQUE_DIMENSIONS_CAPACITY; i++){
        u1 match = memcmp(dimensions[i], type_dimensions, sizeof(u32[4])) == 0;

        if(match){
            return i;
        }
    }

    // Insert if not found
    if(num_dimensions + 1 >= UNIQUE_DIMENSIONS_CAPACITY){
        printf("error on line %d: Maximum number of unique dimension pairs exceeded. Use fewer or reconfigure your compiler.\n", current_line());
        stop_parsing();
        return UNIQUE_DIMENSIONS_CAPACITY;
    }

    for(u32 i = 0; i < 4; i++){
        dimensions[num_dimensions][i] = type_dimensions[i];
    }

    return num_dimensions++;
}

