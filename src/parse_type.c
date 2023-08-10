
#include <stdio.h>
#include "../include/type.h"
#include "../include/token.h"
#include "../include/parse_context.h"
#include "../include/parse_dimensions.h"

Type parse_type(){
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
    type.dimensions = parse_dimensions((u32[4]){0, 0, 0, 0});
    return type;
}

