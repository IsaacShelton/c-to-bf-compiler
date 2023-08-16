
#include <stdbool.h>
#include <stdio.h>
#include "../include/token.h"
#include "../include/token_print.h"
#include "../include/lex.h"
#include "../include/io.h"
#include "../include/storage.h"
#include "../include/lex_context.h"
#include "../include/lex_result.h"
#include "../include/lex_string.h"
#include "../include/lex_character_literal.h"
#include "../include/lex_line_comment.h"
#include "../include/lex_multiline_comment.h"

typedef struct {
    u8 c;
    TokenKind kind;
} SimpleToken;

SimpleToken simple_tokens[10] = {
    (SimpleToken){ .c = '{', .kind = TOKEN_BEGIN },
    (SimpleToken){ .c = '}', .kind = TOKEN_END },
    (SimpleToken){ .c = '(', .kind = TOKEN_OPEN },
    (SimpleToken){ .c = ')', .kind = TOKEN_CLOSE },
    (SimpleToken){ .c = ';', .kind = TOKEN_SEMICOLON },
    (SimpleToken){ .c = '[', .kind = TOKEN_OPEN_BRACKET },
    (SimpleToken){ .c = ']', .kind = TOKEN_CLOSE_BRACKET },
    (SimpleToken){ .c = ',', .kind = TOKEN_NEXT },
    (SimpleToken){ .c = '*', .kind = TOKEN_MULTIPLY },
    (SimpleToken){ .c = '%', .kind = TOKEN_MOD },
};

static u1 is_ident(u8 c){
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_' || (c >= '0' && c <= '9');
}

LexedToken lex_main(){
    LexedToken result = (LexedToken){
        .token = (Token){
            .kind = TOKEN_NONE,
            .data = 0,
            .line = u24_pack(lex_line_number),
        },
        .consumed = 0,
    };

    // Handle empty
    if(code_buffer_length == 0){
        result.token.kind = TOKEN_DONE;
        return result;
    }

    // Handle whitespace
    u32 whitespace = 0;
    for(u32 i = 0; i < code_buffer_length; i++){
        u8 c = code_buffer[i];

        if(c == ' ' || c == '\n' || c == '\t'){
            whitespace++;
        } else {
            break;
        }
    }

    if(whitespace){
        result.consumed = whitespace;
        return result;
    }

    // Handle simple tokens
    for(u32 i = 0; i < sizeof simple_tokens / sizeof(SimpleToken); i++){
        SimpleToken simple = simple_tokens[i];

        if(code_buffer[0] == simple.c){
            result.token.kind = simple.kind;
            result.consumed = 1;
            return result;
        }
    }

    u8 lead = code_buffer[0];

    // Handle strings
    if(lead == '"'){
        result.token.kind = TOKEN_STRING;
        result.consumed = 1;
        return result;
    }

    // Handle integers
    if(lead >= '0' && lead <= '9'){
        u32 value = lead - '0';

        u32 i = 1;
        lead = code_buffer[i];
        while(lead >= '0' && lead <= '9'){
            value = 10 * value + lead - '0';
            lead = code_buffer[++i];
        }

        result.token.kind = TOKEN_INT;
        result.token.data = value;
        result.consumed = i;
        return result;
    }

    // Handle identifiers
    if(is_ident(lead)){
        result.token.kind = TOKEN_WORD;
        u32 i = 1;

        while(i < code_buffer_length){
            if(is_ident(code_buffer[i])){
                i++;
            } else {
                break;
            }
        }

        // NOTE: Anything words longer than code buffer size will be split in two
        result.consumed = i;
        return result;
    }

    // Handle equals
    if(lead == '='){
        if(code_buffer_length > 1 && code_buffer[1] == '='){
            result.token.kind = TOKEN_EQUALS;
            result.consumed = 2;
        } else {
            result.token.kind = TOKEN_ASSIGN;
            result.consumed = 1;
        }
        return result;
    }

    // Handle not
    if(lead == '!'){
        if(code_buffer_length > 1 && code_buffer[1] == '='){
            result.token.kind = TOKEN_NOT_EQUALS;
            result.consumed = 2;
        } else {
            result.token.kind = TOKEN_NOT;
            result.consumed = 1;
        }
        return result;
    }


    // Handle add
    if(lead == '+'){
        result.token.kind = TOKEN_ADD;
        result.consumed = 1;
        return result;
    }

    // Handle subtract
    if(lead == '-'){
        result.token.kind = TOKEN_SUBTRACT;
        result.consumed = 1;
        return result;
    }

    // Handle divide or line comment
    if(lead == '/'){
        if(code_buffer_length > 1 && code_buffer[1] == '/'){
            result.token.kind = TOKEN_LINE_COMMENT;
            result.consumed = 2;
            return result;
        } else if(code_buffer_length > 1 && code_buffer[1] == '*'){
            result.token.kind = TOKEN_MULTILINE_COMMENT;
            result.consumed = 2;
            return result;
        } else {
            result.token.kind = TOKEN_DIVIDE;
            result.consumed = 1;
            return result;
        }
    }
    
    // Handle less than
    if(lead == '<'){
        if(code_buffer_length > 1 && code_buffer[1] == '<'){
            result.token.kind = TOKEN_LSHIFT;
            result.consumed = 2;
        } else {
            result.token.kind = TOKEN_LESS_THAN;
            result.consumed = 1;
        }
        return result;
    }

    // Handle greater than
    if(lead == '>'){
        if(code_buffer_length > 1 && code_buffer[1] == '>'){
            result.token.kind = TOKEN_RSHIFT;
            result.consumed = 2;
        } else {
            result.token.kind = TOKEN_GREATER_THAN;
            result.consumed = 1;
        }
        return result;
    }

    // Handle character literal
    if(lead == '\''){
        return lex_character_literal();
    }

    printf("error on line %d: Unknown character `%c` (ASCII %d)\n", lex_line_number, code_buffer[0], code_buffer[0]);
    result.token.kind = TOKEN_ERROR;
    result.consumed = 1;
    return result;
}

u32 lex(){
    lex_line_number = 1;

    u8 c = get();

    // Lex
    while(true){
        // (Re)-fill buffer
        while(code_buffer_length != CODE_BUFFER_CAPACITY && c){
            code_buffer[code_buffer_length++] = c;
            c = get();
        }

        // Process one token
        LexedToken lexed = lex_main();
        Token token = lexed.token;

        if(token.kind != TOKEN_DONE){
            if(token.kind == TOKEN_ERROR) return 1;

            if(token.kind != TOKEN_NONE && token.kind != TOKEN_LINE_COMMENT && token.kind != TOKEN_MULTILINE_COMMENT){
                // Append created token
                if(num_tokens == TOKENS_CAPACITY){
                    printf("Out of memory: Too many tokens\n");
                    return 1;
                }
                tokens[num_tokens++] = token;
            } else {
                // Increase line number by skipped newlines
                for(u32 i = 0; i < lexed.consumed; i++){
                    if(code_buffer[i] == '\n') lex_line_number++;
                }
            }

            // If token is a word, remember content by injecting aux string
            if(token.kind == TOKEN_WORD){
                tokens[num_tokens - 1].data = num_aux;

                if(num_aux + lexed.consumed + 1 >= AUX_CAPACITY){
                    printf("Out of memory: Auxiliary memory used up\n");
                    return 1;
                }

                for(u32 i = 0; i < lexed.consumed; i++){
                    aux[num_aux++] = code_buffer[i];
                }
                aux[num_aux++] = '\0';
            }

            // Advance code buffer
            for(u32 i = lexed.consumed; i < code_buffer_length; i++){
                code_buffer[i - lexed.consumed] = code_buffer[i];
            }
            code_buffer_length -= lexed.consumed;
        }
        
        if(token.kind == TOKEN_LINE_COMMENT){
            // Special code for line comments
            LexUnboundedResult result = lex_line_comment(c);
            if(result.error) return 1;
            c = result.new_c;
        } else if(token.kind == TOKEN_MULTILINE_COMMENT){
            // Special code for multi-line comments
            LexUnboundedResult result = lex_multiline_comment(c);
            if(result.error) return 1;
            c = result.new_c;
        } else if(token.kind == TOKEN_STRING){
            // Special code for lexing strings, so they are not limited to code buffer capacity
            LexUnboundedResult result = lex_string(c);
            if(result.error) return 1;
            c = result.new_c;
        }

        if(c == 0 && token.kind == TOKEN_DONE){
            break;
        }
    }

    return 0;
} 

