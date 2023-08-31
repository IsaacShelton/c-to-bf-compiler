
#include <stdio.h>
#include "../include/parse_context.h"
#include "../include/storage.h"
#include "../include/token_print.h"

u32 parse_i;
u1 had_parse_error;
u1 parse_trailing_semicolon;

u0 instead_got(){
    if(parse_i < num_tokens){
        printf("  Instead got: `");
        token_print(tokens[parse_i], false);
        printf("`\n");
    }
}

u0 stop_parsing(){
    had_parse_error = true;
    parse_i = num_tokens;
}

u1 is_token(TokenKind kind){
    return parse_i < num_tokens && tokens[parse_i].kind == kind;
}

u32 current_line(){
    if(parse_i < num_tokens){
        return u24_unpack(tokens[parse_i].line);
    } else if(parse_i == num_tokens && num_tokens > 0){
        return u24_unpack(tokens[parse_i - 1].line);
    } else {
        return 0;
    }
}

u24 current_line_packed(){
    if(parse_i < num_tokens){
        return tokens[parse_i].line;
    } else if(parse_i == num_tokens && num_tokens > 0){
        return tokens[parse_i - 1].line;
    } else {
        return u24_pack(0);
    }
}

u32 eat_word(){
    // Expects `is_token(TOKEN_WORD)` to be true
    return tokens[parse_i++].data;
}

u32 eat_int(){
    // Expects `is_token(TOKEN_INT)` to be true
    return tokens[parse_i++].data;
}

u32 eat_string(){
    // Expects `is_token(TOKEN_STRING)` to be true
    return tokens[parse_i++].data;
}

u1 eat_token(TokenKind kind){
    if(parse_i < num_tokens && tokens[parse_i].kind == kind){
        parse_i++;
        return true;
    } else {
        return false;
    }
}

