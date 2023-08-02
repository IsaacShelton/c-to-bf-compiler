
#include "../include/token.h"
#include "../include/storage.h"
#include "../include/lex_token_literal.h"
#include "../include/lex_context.h"

LexedToken lex_character_literal(){
    LexedToken result = {
        .token = (Token){
            .line = u24_pack(lex_line_number),
            .kind = TOKEN_INT,
            .data = 0,
        },
        .consumed = 0,
    };

    if(code_buffer_length >= 4 && code_buffer[1] == '\\' && code_buffer[3] == '\''){
        switch(code_buffer[2]){
        case 't':
            result.token.data = '\t';
            break;
        case 'n':
            result.token.data = '\n';
            break;
        case '0':
            result.token.data = '\0';
            break;
        case '\\':
            result.token.data = '\\';
            break;
        case '\'':
            result.token.data = '\'';
            break;
        default:
            result.token.data = code_buffer[2];
        }

        result.consumed = 4;
    } else if(code_buffer_length >= 3 && code_buffer[2] == '\''){
        result.token.data = code_buffer[1];
        result.consumed = 3;
    }

    return result;
}

