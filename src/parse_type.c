
#include <stdio.h>
#include "../include/type.h"
#include "../include/token.h"
#include "../include/storage.h"
#include "../include/parse_context.h"
#include "../include/parse_dimensions.h"
#include "../include/builtin_types.h"

Type parse_type(){
    Type type = (Type){
        .name = (u32) 0,
        .dimensions = (u32) 0,
    };

    // Parse type name
    if(!is_token(TOKEN_WORD)){
        printf("\nerror on line %d: Expected type\n", current_line());
        instead_got();
        stop_parsing();
        return type;
    }

    type.name = eat_word();

    u32 unaliased = try_resolve_type_alias(type.name);

    if(unaliased < TYPES_CAPACITY){
        type = types[unaliased];
        type.dimensions = parse_dimensions(dimensions[type.dimensions]);
    } else {
        u32 starting[4] = { (u32) 0, (u32) 0, (u32) 0, (u32) 0};
        type.dimensions = parse_dimensions(starting);
    }

    // Translate C integer types
    if(aux_cstr_equals_void(type.name)){
        type.name = types[u0_type].name;
    } else if(aux_cstr_equals_bool(type.name)){
        type.name  = types[u1_type].name;
    } else if(aux_cstr_equals_char(type.name)){
        type.name = types[u8_type].name;
    } else if(aux_cstr_equals_short(type.name)){
        type.name = types[u16_type].name;
    } else if(aux_cstr_equals_int(type.name)){
        type.name = types[u16_type].name;
    } else if(aux_cstr_equals_long(type.name)){
        type.name = types[u32_type].name;
    }

    return type;
}

u1 is_type_followed_by(TokenKind followed_by){
    u32 prev_parse_i = parse_i;
    u1 ok = eat_token(TOKEN_WORD);

    for(u8 i = 1; ok && is_token(TOKEN_OPEN_BRACKET); i++){
        if(
            !eat_token(TOKEN_OPEN_BRACKET)
         || !(eat_token(TOKEN_INT) || eat_token(TOKEN_WORD))
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

