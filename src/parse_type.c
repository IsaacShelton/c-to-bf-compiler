
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
        printf("\nerror on line %d: Expected type\n", current_line());
        instead_got();
        stop_parsing();
        return type;
    }

    type.name = eat_word();
    type.dimensions = parse_dimensions((u32[4]){0, 0, 0, 0});
    return type;
}

u1 is_type_followed_by(TokenKind followed_by){
    u32 prev_parse_i = parse_i;
    u1 ok = eat_token(TOKEN_WORD);

    for(u32 i = 1; ok && is_token(TOKEN_OPEN_BRACKET); i++){
        if(
            !eat_token(TOKEN_OPEN_BRACKET)
         || !eat_token(TOKEN_INT)
         || !eat_token(TOKEN_CLOSE_BRACKET)
         || i > 4
        ){
            ok = false;
            break;
        }
    }

    ok = ok && is_token(followed_by);
    parse_i = prev_parse_i;
    return ok;
}

